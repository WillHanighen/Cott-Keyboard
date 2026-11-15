# Cott MIDI firmware

This sketch sends USB MIDI notes from 14 dual-contact keys. Row A closes first,
then row B. The elapsed time determines velocity. The joystick controls the
128×128 SH1107 OLED settings menu.

## Build and upload

Use PlatformIO from this directory:

```sh
pio run
pio run --target upload
pio device monitor
```

The configuration uses Espressif32 8.0.0 with Arduino USBMIDI, USB OTG mode,
32 MB flash, and octal PSRAM. The 16 MB partition table uses only part of the
flash. Confirm the actual purchased module matches these memory settings.

Connect the host to the DevKit's **native USB port** for MIDI. Its USB-to-UART
bridge port cannot carry USB MIDI. If automatic upload fails after firmware
starts using USB OTG, hold BOOT while resetting to enter the ROM downloader.

## Play and adjust settings

1. Leave the joystick centered and all keys released during reset.
2. Select Cott MIDI Keyboard as a MIDI input in a DAW or MIDI monitor.
3. Press each key. The default mapping is 14 ascending semitones starting at
   MIDI note 60, on channel 1. Edit `kNoteOffsets` if physical key order differs
   from electrical column order.
4. Move the joystick up or down to select a setting, then left or right to
   change it. Settings are held in RAM and reset on reboot.
5. Select Panic and click the joystick to send All Sound Off and All Notes Off
   on every channel. Release and press a key again to resume that note.

The menu controls the base note, MIDI channel, fastest strike time, slowest
strike time, and LED brightness. The display shows the latest velocity and raw
A-to-B travel time in microseconds. LEDs follow the electrical column order,
D29 through D42.

Changing the base note or channel affects new strikes. Notes already sounding
retain their original note and channel until released. Releasing B sends Note
Off with release velocity zero. Both contacts must open before another strike.
A key held at startup must be released before it can sound.

## Calibrate velocity

The initial curve maps travel times of 2 ms or less to velocity 127 and 80 ms or
more to velocity 1, with a linear curve between those limits. These are starting
values, not measurements of this keybed.

Play hard and soft strikes while watching Travel us. Adjust Fast ms and Slow ms
to the observed range. Slow ms is always greater than Fast ms.

The scanner timestamps the first observed contact edge and confirms closure
after 500 µs. Opening requires 2 ms stability. A dedicated task scans about once
per millisecond, so timing resolution is roughly 1 ms and depends on scheduling.
OLED redraws run outside the scan task. Very fast strikes within one scan period
cannot be distinguished accurately; validate the resulting dynamics on hardware.

The scanner queues events for the USB task. USB backpressure retries packets.
If the event queue overflows, the firmware sends panic messages and latches
a scan fault until reset. USB disconnects discard outstanding events; held keys
are not replayed on reconnect.

## Checks

From this directory, run the contact-state tests on the host:

```sh
c++ -std=c++17 -Wall -Wextra -Werror -Iinclude test/velocity_key_test.cpp -o /tmp/cott-velocity-test
/tmp/cott-velocity-test
```

The tests cover strike timing, release bounce, full release before retrigger,
incomplete and reversed strikes, startup with held contacts, timer wraparound,
14 simultaneous keys, and velocity mapping.

Before relying on the keyboard, use a MIDI monitor to check hard and soft
strikes, repeated notes, chords, menu changes while notes are held, panic, and
USB reconnection. These behaviors have not been tested on physical hardware.

See [the schematic review](../../docs/schematic-smoke-check.md) for the electrical
checks and remaining assembly checks.
