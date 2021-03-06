# caliper-teensy31 - USB Serial and HID driver for Harbor Freight 100mm Electronic Digital Caliper

Note: There are many different types of Electronic Digital Caliper.
This software is for the kind which sends a 48-bit, binary encoded packet.

This is designed and tested on a Teensy 3.2 using the Arduino development environment.
It's also tested on an Adafruit Trinket M0 without USB HID support, see below.
(Despite the repository name, it's not designed to work with Teensy 3.1, which lacks the needed DAC pin)

It is probably adaptable to other Arduino-supported boards, but the presence of a true DAC (to supply 1.5v to the calipers)
and multiple independent analog comparators (to read the low voltage signals as digital inputs)
are key to having a simple design without any external components.
If you use a different board you'll need to make different arrangements.
(for instance, I started with a resistive divider and one-transistor inverters,
like many others who have also done similar project)

# Install the software

(pin numbers in the main text are for Teensy 3.2; see below for pin numbering on Trinket M0)

1. Configure Arduino for USB Serial+Keyboard.
1. Upload the sketch
1. Verify that pin "A13" generates 1.5V with respect to GND.  If it doesn't, and you continue to follow these instructions, you'll fry the caliper.
1. Verify that pins "11" and "23" are floating.

# Hookup
1. Remove the battery from the caliper.
 If you don't, the 1.5v from the Teensy will try to charge the non-chargeable battery
 which could potentially cause a fire or the release of harmful chemicals.
1. Gain access to the pin connections. On my device, this involved disassembling the caliper and melting a hole in the case:
   1. Remove the sticker on the back to show 4 screws that attach the electronics to the caliper scale.
   1. Remove these screws, noting which of the 4 screws has a finer pitch.
   1. Remove 4 more screws to access the top side of the PCB, where the solder pads are.
   1. Melt a hole through the case with the tip of the soldering iron.
   1. Pass a 4-conductor ribbon cable through the hole
1. Solder or otherwise arrange an electrical connection to the 4 pads inside the caliper above the battery connector
1. Reassemble the caliper and verify that it still works, temporarily reinstalling the battery to do so.
1. From left to right when viewed from the front, the connections on my caliper are GND, DATA, CLOCK and 1.5V
1. Connect GND to Teensy GND and 1.5V to pin A13 **after re-verifying that pin A13 has 1.5V with respect to GND**.
  If you like, you can add a moderate-value capacitor to the 1.5V rail, but I didn't experience any problems without it.
  Now, when Teensy is powered, the caliper should function.
1. Connect DATA to Teensy pin 23 and CLOCK to Teensy pin 11.
  These pins are used in "analog comparator" mode, with a threshold of .67v,
  making level translators unnecessary.
  (By editing the source code, it is possible to make alternate pin assignments,
  but only certain pins can be used as analog pin comparators
  and no effort has been made to make this easy to do.
  Refer to the Kinesis K20P64M72SF1RM manual, section 10.3.1, for pin assignments.
  DATA and CLOCK pins could potentially be assigne to any CMPx\_INy pins,
  as long as "x" is different for each signal.)
1. Connect a NO button to Teensy pin 12 and GND.
  This pin can also be changed in the source code.
  I have considered installing the button in the empty battery compartment or otherwise permanently on the caliper.

# Using
Don't measure anything that could form an electric circuit.
On my caliper, the metal jaws are connected to 1.5V, so even just touching them to the Teensy ground could damage the DAC.

When the position received by Teensy is stable, the LED will light.
When the position is stable and the button is clicked, the readout position will be typed in by USB Keyboard.
(Another button click will be ignored for a half second or so)
No unit symbol is attached, but for mm there are always two decimal places (e.g., 100.99)
and for inches there are always 4 decimal places (e.g., 3.9995).

The position is also transmitted continuously on serial.  You can use this for whatever purpose you imagine.
There's an included DRO program written in Tcl/TK.
It will show the caliper readout in the upper right of the screen, timing out after the caliper position has been stable for 4s.
If you have tcl/tk installed, then you can launch the DRO with `wish dro.tcl `_`comport`_ (e.g., `wish dro.tcl /dev/ttyACM0` on my Linux machine).

# If it doesn't work
If no data is read, the paste key will type "NO DATA" once per power on.
This could mean that your caliper is incompatible, or that something's wrong with the connection circuit.
I don't have any specific troubleshooting guidance.

(If things that look like powers of two are output, like 655.36mm, then data and clk are switched)

# Adafruit Triket M0

This board also works, though at the moment it only has simulated serial for use with dro.tcl, not USB HID to type in the values measured.
Follow these alternate pin assignments:

- pin 1: 1.5v output
- pin 3: data input
- pin 4: clk input
