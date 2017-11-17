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
constexpr auto PIN_nCLK = 14;
constexpr auto PIN_nDATA = 11;
constexpr auto PIN_nACT = 12;
constexpr auto PIN_DBG = 15;
constexpr auto DAC_SETTING_V1_5 = int(1.5 * 4096 / 3.3 + .5);
char buf[32];

void setup() {
  Serial.begin(9600); 
  analogWriteResolution(12);
  pinMode(PIN_nCLK, INPUT_PULLUP);
  pinMode(PIN_nDATA, INPUT_PULLUP);
  pinMode(PIN_nACT, INPUT_PULLUP);
  pinMode(PIN_DBG, OUTPUT);
  analogWrite(PIN_V1_5, DAC_SETTING_V1_5);
  // put your setup code here, to run once:
  snprintf(buf, sizeof(buf), "no data");
}

#define STATE_WAIT_IDLE (0)
#define STATE_WAIT_LOW (1)
#define STATE_WAIT_HIGH (2)

uint8_t state = STATE_WAIT_IDLE;
uint64_t reading;
uint8_t bitno;
uint8_t clk, data;
uint16_t time_idle;

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


uint64_t old_reading;
int j;
static_assert(sizeof(1ull) == 8, "unsigned long long is 64 bits");
uint8_t wait_low() {
  if (clk) return STATE_WAIT_LOW;
  if (!data) reading = reading | (1ull << bitno); // (data << bitno);
  
  #ifdef USB_SERIAL
//  if(bitno == j) {
//    Serial.print(!data);
//  }
  #endif
bitno ++;
  if (bitno == 48) {
//    j++;
//    if(j == 48) { j = 0; Serial.print('\n'); }
    auto stable = (reading == old_reading);
    old_reading = reading;
    if(!stable) return STATE_WAIT_IDLE;
    auto position = (reading & 0xfffffull);
    auto signbit = bool(reading & 0x100000ull);
    auto inch = bool(reading & (1ull << 47));
#define DO_DEBUG 0
#if DO_DEBUG
    snprintf(buf, sizeof(buf), "%12llx %d %d %d\n", 
        reading, (int)position, (int)signbit, (int)inch);
#else    
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
#endif
    return STATE_WAIT_IDLE;
  }
  return STATE_WAIT_HIGH;
}

bool old_act;

void loop() {
  bool act = !digitalRead(PIN_nACT);
  if (act && !old_act) {
    #ifdef USB_HID
    Keyboard.write(buf);
    #else
    Serial.write(buf);
    #endif
    *buf = 0;
    delay(300);
  }
  old_act = act;
  clk = !!digitalRead(PIN_nCLK);
  data = !!digitalRead(PIN_nDATA);

  digitalWrite(PIN_DBG, time_idle & 1);
  switch (state) {
    case STATE_WAIT_IDLE: state = wait_idle(); break;
    case STATE_WAIT_LOW:  state = wait_low(); break;
    case STATE_WAIT_HIGH: state = wait_high(); break;
  }
  //digitalWrite(PIN_DBG, 0);
  ///if(state != STATE_WAIT_IDLE) goto again;
  //if(bitno >= 24) { Serial.print((int)bitno); Serial.print("! "); Serial.println(state); }
  //    Serial.print(state); Serial.print(" "); Serial.print(bitno); Serial.print("\n");
}
