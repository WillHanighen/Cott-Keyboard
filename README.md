# Cott Keyboard
## A shitty 14-key MIDI keyboard

Idiot attempts PCB design n stuff for first time

## Hardware
 - [ESP32-S3-DevKitC-1-N32R16V](https://www.aliexpress.us/item/3256803793464226.html?gatewayAdapt=glo2usa) (note: URL dose not auto-select proper variant)
 - [Adafruit Monochrome 1.12" 128x128 OLED](https://www.adafruit.com/product/5297)
 - [STEMMA QT / Qwiic JST SH 4-Pin Cable](https://www.adafruit.com/product/4399)
 - [Analog 2-axis Thumb Joystick with Select Button + Breakout Board](https://www.adafruit.com/product/512)
 - [NeoPixel 5050 RGB LED with Integrated Driver Chip](https://www.adafruit.com/product/1655) (10 pack, buy 2; 14 total required)

### Power notice

This project was designed arround being powered by the DevKit's USB-C port, and the USB spec is max. 900mA for USB3, and 500mA for USB2 (what the type-c port communicates as)
In other words, keep the LED brightness down (~25% max, stinky I know, take it up with the USB implments fourm for not making it 2A, as [power that be] intended) and avoid whites.

## License

Licensed under CC BY-NC-SA 4.0: You can freely edit and redistribute this project as long as you credit me, don't do it commercially, and/or for monetary gain, and publish it under the same license.
