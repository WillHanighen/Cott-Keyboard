#include <Arduino.h>
#include <Wire.h>
#include <USB.h>
#include <USBMIDI.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SH110X.h>
#include <atomic>
#include "velocity_key.h"

namespace {
constexpr uint8_t kColumns[] = {1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 17};
constexpr uint8_t kRows[] = {18, 21};
constexpr uint8_t kSda = 39, kScl = 40, kLed = 47;
constexpr uint8_t kJoyX = 15, kJoyY = 16, kJoySwitch = 42;
// Provisional ascending chromatic order; change offsets to match the keybed.
constexpr uint8_t kNoteOffsets[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

struct Settings {
  int baseNote = 60;
  int channel = 1;
  int fastMs = 2;
  int slowMs = 80;
  int brightness = 24;
} settings;

struct ScanEvent {
  uint8_t key;
  cottage::KeyEvent type;
  uint32_t travelUs;
};
struct PlayingNote {
  bool active = false;
  uint8_t note = 0;
  uint8_t channel = 1;
  uint8_t velocity = 0;
} playing[14];

USBMIDI midi;
Adafruit_SH1107 display(128, 128, &Wire, -1, 400000, 100000);
Adafruit_NeoPixel pixels(14, kLed, NEO_GRB + NEO_KHZ800);
QueueHandle_t events = nullptr;
std::atomic<bool> scanFault{false};
bool displayReady = false, scanReady = false;
uint8_t menuItem = 0, lastVelocity = 0;
uint32_t lastTravelUs = 0;
int centerX = 2048, centerY = 2048;
constexpr const char *kMenuNames[] = {
    "Base note", "Channel", "Fast ms", "Slow ms", "LED level", "Panic"
};

// No I2C, MIDI, allocation, or logging in the scanner.
void scanTask(void *) {
  cottage::VelocityKey keys[14];
  for (;;) {
    bool contacts[2][14];
    uint32_t times[2];
    for (uint8_t row = 0; row < 2; ++row) {
      digitalWrite(kRows[row], LOW);
      pinMode(kRows[row], OUTPUT);
      delayMicroseconds(30);
      times[row] = micros();
      for (uint8_t col = 0; col < 14; ++col)
        contacts[row][col] = digitalRead(kColumns[col]) == LOW;
      pinMode(kRows[row], INPUT); // Inactive row floats, never drives HIGH.
    }
    for (uint8_t col = 0; col < 14; ++col) {
      const auto type = keys[col].sample(contacts[0][col], contacts[1][col],
                                         times[0], times[1]);
      if (type != cottage::KeyEvent::None) {
        const ScanEvent event{col, type, keys[col].travelUs};
        if (xQueueSend(events, &event, 0) != pdTRUE) scanFault.store(true);
      }
    }
    vTaskDelay(1); // About 1 kHz; A/B travel timing has roughly 1 ms resolution.
  }
}

bool sendMidi(uint8_t status, uint8_t data1, uint8_t data2) {
  midiEventPacket_t packet = {
      static_cast<uint8_t>(status >> 4), status, data1, data2};
  return midi.writePacket(&packet);
}

void resetPlaying() {
  for (auto &note : playing) note.active = false;
}

void serviceMidi(bool connected) {
  // Keep an event until TinyUSB accepts it; never drop a Note Off on backpressure.
  static ScanEvent event;
  static bool pending = false;
  if (!connected || scanFault.load()) {
    pending = false;
    ScanEvent discarded;
    while (xQueueReceive(events, &discarded, 0) == pdTRUE) {}
    return;
  }
  for (uint16_t count = 0; count < 128; ++count) {
    if (!pending && xQueueReceive(events, &event, 0) != pdTRUE) break;
    pending = true;
    auto &note = playing[event.key];
    if (event.type == cottage::KeyEvent::On) {
      const uint8_t velocity = cottage::velocity(
          event.travelUs, settings.fastMs * 1000u, settings.slowMs * 1000u);
      const uint8_t number = settings.baseNote + kNoteOffsets[event.key];
      if (!sendMidi(0x90 | (settings.channel - 1), number, velocity)) break;
      note = {true, number, static_cast<uint8_t>(settings.channel), velocity};
      lastVelocity = velocity;
      lastTravelUs = event.travelUs;
    } else if (note.active) {
      if (!sendMidi(0x80 | (note.channel - 1), note.note, 0)) break;
      note.active = false;
    }
    pending = false;
  }
}

// Retry panic packets as well. A queue overflow latches a fault until reset.
bool panicPending = false;
uint8_t panicChannel = 0;
void servicePanic() {
  while (panicPending && USB) {
    if (!sendMidi(0xb0 | panicChannel, 120, 0)) return; // All Sound Off
    if (!sendMidi(0xb0 | panicChannel, 123, 0)) return; // All Notes Off
    if (++panicChannel == 16) {
      panicPending = false;
      panicChannel = 0;
      resetPlaying();
    }
  }
}

int axisDirection(int reading, int center) {
  if (reading < center - 900) return -1;
  if (reading > center + 900) return 1;
  return 0;
}

void updateMenu(uint32_t now) {
  static uint32_t lastSample = 0, lastAction = 0;
  static int previousX = 0, previousY = 0;
  static cottage::Contact button;
  static bool previousButton = false;
  if (now - lastSample < 10) return;
  lastSample = now;
  const int x = axisDirection(analogRead(kJoyX), centerX);
  const int y = axisDirection(analogRead(kJoyY), centerY);
  if ((x || y) && ((!previousX && !previousY) || now - lastAction >= 250)) {
    lastAction = now;
    if (y) menuItem = (menuItem + (y > 0 ? 1 : 5)) % 6;
    else switch (menuItem) {
      case 0: settings.baseNote = constrain(settings.baseNote + x, 0, 114); break;
      case 1: settings.channel = constrain(settings.channel + x, 1, 16); break;
      case 2: settings.fastMs = constrain(settings.fastMs + x, 1, settings.slowMs - 1); break;
      case 3: settings.slowMs = constrain(settings.slowMs + x, settings.fastMs + 1, 250); break;
      case 4: settings.brightness = constrain(settings.brightness + x, 0, 63); break;
      default: break;
    }
  }
  previousX = x;
  previousY = y;
  button.sample(digitalRead(kJoySwitch) == LOW, micros());
  if (button.closed && !previousButton && menuItem == 5) panicPending = true;
  previousButton = button.closed;
}

void drawStatus() {
  pixels.setBrightness(settings.brightness);
  for (uint8_t i = 0; i < 14; ++i) {
    pixels.setPixelColor(i, playing[i].active
        ? pixels.Color(0, playing[i].velocity * 2, 0)
        : pixels.Color(0, 0, 12));
  }
  pixels.show();
  if (!displayReady) return;
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Cott MIDI");
  display.println(scanFault.load() || !scanReady ? "Scan fault: reset!"
                                               : (USB ? "USB connected" : "USB waiting"));
  const int values[] = {settings.baseNote, settings.channel, settings.fastMs,
                         settings.slowMs, settings.brightness};
  for (uint8_t i = 0; i < 6; ++i) {
    display.print(i == menuItem ? "> " : "  ");
    display.print(kMenuNames[i]);
    if (i < 5) {
      display.print(": ");
      display.print(values[i]);
    }
    display.println();
  }
  display.print("Velocity: ");
  display.println(lastVelocity);
  display.print("Travel us: ");
  display.println(lastTravelUs);
  display.println("Up/down: select");
  display.println("L/R: edit  Click:panic");
  display.display();
}
} // namespace

void setup() {
  Serial.begin(115200);
  for (uint8_t pin : kColumns) pinMode(pin, INPUT_PULLUP);
  for (uint8_t pin : kRows) pinMode(pin, INPUT);
  pinMode(kJoySwitch, INPUT_PULLUP);
  analogReadResolution(12);
  analogSetPinAttenuation(kJoyX, ADC_11db);
  analogSetPinAttenuation(kJoyY, ADC_11db);
  uint32_t sumX = 0, sumY = 0;
  for (uint8_t i = 0; i < 64; ++i) {
    sumX += analogRead(kJoyX);
    sumY += analogRead(kJoyY);
    delay(2);
  }
  centerX = sumX / 64;
  centerY = sumY / 64;
  if (centerX < 1000 || centerX > 3000) centerX = 2048;
  if (centerY < 1000 || centerY > 3000) centerY = 2048;

  pixels.begin();
  pixels.clear();
  pixels.show();
  Wire.begin(kSda, kScl);
  Wire.setTimeOut(20);
  delay(100);
  for (uint8_t address : {0x3d, 0x3c}) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0 && display.begin(address, true)) {
      displayReady = true;
      display.setTextSize(1);
      display.setTextColor(SH110X_WHITE);
      break;
    }
  }
  midi.begin();
  USB.productName("Cott MIDI Keyboard");
  USB.usbPower(500);
  USB.begin();
  events = xQueueCreate(128, sizeof(ScanEvent));
  // Scanner preempts display work on the Arduino core; it sleeps every scan.
  scanReady = events && xTaskCreatePinnedToCore(
      scanTask, "keyscan", 4096, nullptr, 2, nullptr, ARDUINO_RUNNING_CORE) == pdPASS;
}

void loop() {
  static bool wasConnected = false, faultHandled = false;
  static uint32_t lastDisplay = 0;
  const bool connected = USB;
  if (!connected && wasConnected) {
    resetPlaying();
    panicPending = false;
    panicChannel = 0;
  }
  if (connected && !wasConnected) panicPending = true;
  wasConnected = connected;
  if (scanFault.load() && !faultHandled) {
    panicPending = true;
    faultHandled = true;
  }
  if (scanReady) {
    if (panicPending) serviceMidi(false);
    servicePanic();
    if (!panicPending) serviceMidi(connected);
  }
  const uint32_t now = millis();
  updateMenu(now);
  if (now - lastDisplay >= 100) {
    lastDisplay = now;
    drawStatus();
  }
  delay(1);
}
