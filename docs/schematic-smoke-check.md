# Schematic smoke check

Reviewed 2026-11-14 local time with KiCad CLI 10.0.8 against the current schematic.
This is a schematic review, not a fabrication sign-off or a powered hardware test.

## Result

No obvious supply short, reversed LED-chain connection, or incompatible scan pin
assignment was found. KiCad ERC reports **0 errors and 0 warnings** with the
project's rules. Four check categories are disabled: single-use global labels,
four-way junctions, SPICE models, and footprint-filter mismatches.

The PCB file contains only an empty board declaration. Routing, ground returns,
decoupler placement, connector orientation, trace current capacity, antenna
clearance, and physical fit therefore remain unverified.

## Before layout and assembly

- All 91 components now have footprints. See [package selections](component-packages.md).
- **Verify the purchased DevKit against U1 and its footprint.** The schematic
  uses a generic ESP32-S3-DevKitC symbol and a PCM footprint. The README names an
  AliExpress N32R16V variant. Confirm header order, spacing, 5 V, 3.3 V, ground,
  USB connector access, and which USB connector exposes native USB. Successful
  compilation does not confirm the module's flash mode or supply path.
- **Verify J2 wiring by signal labels.** Its schematic order is 3.3 V, ground,
  X, Y, select. Do not assume the joystick breakout has the same physical order.
  Keep the potentiometers supplied from 3.3 V so the ADC inputs cannot reach 5 V.
- **Budget the entire USB load.** A brightness percentage is not a current
  limiter. The LEDs share the board's USB-derived 5 V rail. Include the DevKit,
  OLED, LED idle current, and startup charging of C16 in measurements. The
  sketch caps LED brightness at 63/255 and starts at 24/255, with Wi-Fi and
  Bluetooth unused. It advertises a 500 mA USB configuration; this does not
  establish what a particular host or clone board can safely supply.
- **Place capacitors near their loads.** C1–C14 are the 100 nF LED bypass
  capacitors, C15 bypasses U2, and C16 is the polarized 470 µF bulk capacitor.
  Place R1 near the first pixel's input. Confirm LED pin-1 orientation and
  capacitor polarity when assigning footprints. Adafruit recommends 500–1000 µF
  bulk capacitance for larger NeoPixel supplies; 470 µF is close, but startup
  and rail transients still require measurement.

## Connections checked

- Fourteen keys each have A and B contacts, with separate diodes. The anodes
  connect to pulled-up column inputs; the cathodes connect through switches to
  scan rows. Drive the selected row LOW and float the other row.
- ROW_A is GPIO18; ROW_B is GPIO21. KEY_0 through KEY_13 use GPIO
  1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 17.
- ROW_A switch order is SW1–SW12, SW27, SW28. ROW_B is SW13–SW26.
  Each column is one physical key, not two independent notes.
- GPIO47 feeds U2's input. U2 is a 5 V 74AHCT1G125 with active-low enable
  grounded. Its output goes through the 330 ohm R1 to D29 DIN. D29–D42 form
  one chain, with the last output intentionally unused.
- J1 follows the STEMMA QT signal order: ground, 3.3 V, SDA, SCL. SDA is GPIO39
  and SCL is GPIO40. The Adafruit OLED is an SH1107, not an SSD1306.
- Joystick X and Y use GPIO15 and GPIO16; select uses GPIO42 with a firmware
  pull-up. ADC2 is suitable here because the sketch does not start Wi-Fi.
- The schematic leaves GPIO19 and GPIO20 free for the DevKit's native USB.
  GPIO35–GPIO37, reserved for memory on octal-memory modules, are also unused.

Run the connectivity checks again from the repository root:

```sh
python3 scripts/check_schematic.py
```

The script exports a fresh netlist, runs ERC, compares the firmware GPIOs,
checks the contact pairs and connectors, and checks the LED chain and buffer
connections. It does not replace a review of the physical board.

## References

- [Espressif DevKitC-1 guide](https://docs.espressif.com/projects/esp-idf/en/v5.0.3/esp32s3/hw-reference/esp32s3/user-guide-devkitc-1.html)
- [Adafruit OLED pinouts](https://learn.adafruit.com/adafruit-monochrome-1-12-in-128x128-oled/pinouts)
- [Adafruit SH1107 Arduino example](https://github.com/adafruit/Adafruit_SH110x/blob/master/examples/SH1107_128x128/SH1107_128x128.ino)
- [Adafruit joystick specifications](https://www.adafruit.com/product/512)
- [Adafruit NeoPixel best practices](https://learn.adafruit.com/adafruit-neopixel-uberguide/best-practices)
- [Adafruit LED specifications and revision history](https://www.adafruit.com/product/1655)
