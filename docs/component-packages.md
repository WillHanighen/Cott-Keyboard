# Component packages

All 91 schematic components now have footprints. The PCB remains unplaced and
unrouted; these assignments do not define the key spacing or switch heights.

| References | Selection |
| --- | --- |
| SW1–SW28 | Omron D2F-01FL hinge-lever microswitch, straight through-hole PCB terminals |
| D1–D28 | 1N4148W switching diode, SOD-123 |
| C1–C15 | 100 nF, 0805 hand-solder pads; select X7R, 25 V or higher |
| R1 | 330 ohms, 0805 hand-solder pads; 0.125 W or higher |
| J2 | Five direct wire-solder terminals, 4.2 mm pitch, 0.65 mm plated holes, with strain-relief holes |
| C16 | Existing EEU-FR1A471L, 470 µF radial electrolytic, 8 mm diameter, 3.5 mm lead pitch |
| D29–D42 | Existing 5050 four-pad addressable LEDs |
| U2 | Existing SOT-23-5 74AHCT1G125 |
| J1 | Existing horizontal JST-SH STEMMA QT connector |
| U1 | Existing Espressif DevKit footprint; requires the Espressif KiCad PCM library |

The custom Cott footprint library is registered in the project's fp-lib-table.
The switch footprint follows the straight-terminal D2F drawing: three terminals
at 5.08 mm pitch, 1.2 mm holes, and a 12.8 × 5.8 mm body.

The custom pad numbers intentionally match the existing SPST symbol:
**pad 1 is NO, pad 2 is COM, and pad 3 is unused NC**. In top-view footprint
coordinates COM is at x=0, NO at x=5.08, and NC at x=10.16.
NC has a solderable hole but no schematic connection. Do not route it.
These numbers are project assignments, not a claim about numbers printed on the
switch housing.

The D2F-01FL is the gold-contact, low-force hinge-lever variant. Its mechanical
fit, actuation force, lever clearance, and A-before-B placement must be checked
with samples before making the key mechanism. The footprint's courtyard covers
the body; leave additional clearance for the moving lever and key.

Omron specifies a minimum applicable load of 1 mA at 5 V for the D2F-01 series.
The ESP32's internal pull-ups draw much less current at 3.3 V. Contact reliability
at this circuit's load is therefore unverified, even with the gold-contact
version. Test the switch at the actual scan current; external column pull-ups or
a different contact technology may be needed. Do not substitute a general
high-current silver-contact switch without checking its low-load behavior.

J2 is a wire termination, not a plug connector. Its five signals remain
3.3 V, ground, X, Y, select in pad order. The selected 0.25 mm² footprint assumes
wire around AWG24; use smaller wire or adjust the hole size to the actual
conductor before manufacture.

Reapply these assignments without changing schematic positions or wires:

```sh
python3 scripts/assign_footprints.py
python3 scripts/check_schematic.py
```

[Omron D2F datasheet](https://omronfs.omron.com/en_US/ecb/products/pdf/en-d2f.pdf),
pages 2–4, defines the contact forms, terminal dimensions, and lever variants.
