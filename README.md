# caliper-teensy31
USB HID driver for Harbor Freight 100mm Electronic Digital Caliper

Note: There are many different types of Electronic Digital Caliper.
This software is for the kind which sends a 48-bit, binary encoded packet.

# Install the software
This is designed and tested on a Teensy 3.1 using the Arduino development environment.
It is probably adaptable to other Arduino-supported boards, but one important feature of the Teensy is that it has a true DAC which is used to generate the 1.5V supply for the caliper.
If you use a different board you'll need to make different arrangements.

1. Configure Arduino for USB Serial+Keyboard.
1. Upload the sketch
1. Verify that pin "A13" generates 1.5V with respect to GND.  If it doesn't, and you continue to follow these instructions, you'll fry the caliper.

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
1. For Clock and Data, make a [1-transistor inverter](https://en.wikipedia.org/wiki/Inverter_(logic_gate)#/media/File:Transistor_pegelumsetzer.svg).
  The Teensy enables an internal pull-up, so only the base resistor and transistor are needed. 
  A 10k base resistor works fine, but if I were doing it again I'd try with 47k or 100k,
  because with 10k the base current of the two transistors is 10x the current consumption of the unmodified caliper.
  However, since you aren't powering this from battery, the only worry is whether this damages the caliper electronics themselves.
  So far (days of power-on time) it hasn't, which is unsurprising because while "10x" sounds like a lot it's still just microwatts.
1. Connect inverted clock to Teensy pin 14, and inverted ata to Teensy pin 11.
  Either of these pins can be changed in the source code
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
If you have tcl/tk installed, then you can launch the DRO with `wish dro.tcl`.

# If it doesn't work
If no data is read, the paste key will type "NO DATA" once per power on.
This could mean that your caliper is incompatible, or that something's wrong with the connection circuit.
I don't have any specific troubleshooting guidance.
