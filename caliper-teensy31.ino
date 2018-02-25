/*
    USB HID driver for Harbor Freight 100mm Electronic Digital Caliper 
    Copyright © 2017 Jeff Epler

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
constexpr auto PIN_V1_5 = A14;
constexpr auto PIN_nACT = 12;
constexpr auto PIN_LED = 13;
constexpr auto DAC_SETTING_V1_5 = int(1.5 * 4096 / 3.3 + .5);
char buf[81];

static_assert(sizeof(1ull) == 8, "unsigned long long is 64 bits");

void setup() {
  Serial.begin(9600); 
  analogWriteResolution(12);
  pinMode(PIN_nACT, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  analogWrite(PIN_V1_5, DAC_SETTING_V1_5);
  snprintf(buf, sizeof(buf), "no data");

// Set up the analog comparators to read the low-voltage signals.
//
// Unfortunately, analog comparators aren't standardized by the Arduino library,
// or by any third-party library I ran across, so we're going to go directly
// down to the register level for this.
  SIM_SCGC4 |= SIM_SCGC4_CMP; // enable clock to comparator module

// CMP1 (data pin)
  CMP1_CR0 = CMP_CR0_FILTER_CNT(1) | CMP_CR0_HYSTCTR(1);
  CMP1_CR1 = CMP_CR1_INV | CMP_CR1_EN;
  CMP1_MUXCR = CMP_MUXCR_PSEL(0) | CMP_MUXCR_MSEL(7);
// set DAC1 ref to code 14, since 3.3v * 14/64 = .67v
  CMP1_DACCR = CMP_DACCR_DACEN | CMP_DACCR_VRSEL | CMP_DACCR_VOSEL(14);

// CMP0 (clock pin)
  CMP0_CR0 = CMP_CR0_FILTER_CNT(1) | CMP_CR0_HYSTCTR(1);
  CMP0_CR1 = CMP_CR1_INV | CMP_CR1_EN;
  CMP0_MUXCR = CMP_MUXCR_PSEL(0) | CMP_MUXCR_MSEL(7);
// set DAC0 ref to code 14, since 3.3v * 14/64 = .67v
  CMP0_DACCR = CMP_DACCR_DACEN | CMP_DACCR_VRSEL | CMP_DACCR_VOSEL(14);

// For other pin assignment possibilities, see the reference manual
// https://cache.freescale.com/files/32bit/doc/ref_manual/K20P64M72SF1RM.pdf
// 3.7.2.1 (page 105) for internal connections to the analog comparator,
// and 10.3.1 (page 207ff) for external pin assignments.  Remember that
// each signal needs to go to a different CMPx comparator (e.g., CMP0 and CMP1,
// CMP1 and CMP2, or CMP0 and CMP2)
}

// (I don't use an enum because of a deficiency in arduino's handling of enums: https://playground.arduino.cc/Code/Enum)
#define STATE_WAIT_IDLE (0)
#define STATE_WAIT_LOW (1)
#define STATE_WAIT_HIGH (2)

uint8_t state, bitno;
bool clk, data, stable;
uint16_t time_idle;
uint64_t reading, old_reading;

uint8_t wait_idle() {
  if (!clk)
    time_idle ++;
  else
    time_idle = 0;
  if (time_idle > 300) {
    reading = 0;
    bitno = 0;
    time_idle = 0;
    return STATE_WAIT_HIGH;
  }
  return STATE_WAIT_IDLE;
}

uint8_t wait_high() {
  if (!clk) return STATE_WAIT_HIGH;
  return STATE_WAIT_LOW;
}


uint8_t wait_low() {
  if (clk) return STATE_WAIT_LOW;
  if (!data) reading = reading | (1ull << bitno); // (data << bitno);
  
  bitno ++;
  if (bitno == 48) {
    stable = (reading == old_reading);
    old_reading = reading;
    auto position = (reading & 0xfffffull);
    auto signbit = bool(reading & 0x100000ull);
    auto inch = bool(reading & (1ull << 47));

    digitalWrite(PIN_LED, stable);
    
    if (inch) {
      snprintf(buf, sizeof(buf), " %s%d.%03d%c",
             signbit ? "-" : "",
             (int)(position / 2000),
             (int)(position / 2) % 1000,
             position % 2 ? '5' : '0');
    } else {
      snprintf(buf, sizeof(buf), " %s%d.%02d",
             signbit ? "-" : "",
             (int)(position / 100),
             (int)(position % 100));
    }
    Serial.write(buf);
    Serial.write("\n");

    return STATE_WAIT_IDLE;
  }
  return STATE_WAIT_HIGH;
}

bool old_act;

void loop() {
  bool act = !digitalRead(PIN_nACT);
  if (act && !old_act && stable) {
    Keyboard.write(buf);
    *buf = 0;
    old_reading = ~0;
    stable = false; // this ends up acting like a debounce
    digitalWrite(PIN_LED, false);
  }
  old_act = act;
  clk = !!(CMP0_SCR & CMP_SCR_COUT);
  data = !!(CMP1_SCR & CMP_SCR_COUT);

  switch (state) {
    case STATE_WAIT_IDLE: state = wait_idle(); break;
    case STATE_WAIT_LOW:  state = wait_low(); break;
    case STATE_WAIT_HIGH: state = wait_high(); break;
  }
}
