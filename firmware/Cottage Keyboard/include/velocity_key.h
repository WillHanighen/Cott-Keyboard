#pragma once
#include <cstdint>

namespace cottage {
struct Contact {
  bool candidate = false;
  bool closed = false;
  uint32_t edgeUs = 0;

  void sample(bool raw, uint32_t now) {
    if (raw != candidate) {
      candidate = raw;
      edgeUs = now;
    }
    // Timestamp the first edge, then confirm it. Do not time from debounce expiry.
    if (now - edgeUs >= (raw ? 500u : 2000u)) closed = candidate;
  }
};

enum class KeyEvent { None, On, Off };
struct VelocityKey {
  enum class Phase { Ready, Armed, Sounding, WaitRelease };
  Contact a, b;
  Phase phase = Phase::WaitRelease;
  uint32_t travelUs = 0;

  KeyEvent sample(bool rawA, bool rawB, uint32_t timeA, uint32_t timeB) {
    a.sample(rawA, timeA);
    b.sample(rawB, timeB);
    switch (phase) {
      case Phase::Ready:
        if (a.closed) phase = Phase::Armed;
        else if (b.closed) phase = Phase::WaitRelease;
        break;
      case Phase::Armed:
        break;
      case Phase::Sounding:
        if (!b.closed) {
          phase = Phase::WaitRelease;
          return KeyEvent::Off;
        }
        return KeyEvent::None;
      case Phase::WaitRelease:
        // Candidate checks also reject contacts held during startup.
        if (!a.closed && !b.closed && !a.candidate && !b.candidate)
          phase = Phase::Ready;
        return KeyEvent::None;
    }
    if (phase == Phase::Armed) {
      if (!a.closed) phase = Phase::WaitRelease;
      else if (b.closed) {
        travelUs = b.edgeUs - a.edgeUs;
        // B preceding A is not a valid strike, including across timer wrap.
        if (travelUs > 0x7fffffffu) phase = Phase::WaitRelease;
        else {
          phase = Phase::Sounding;
          return KeyEvent::On;
        }
      }
    }
    return KeyEvent::None;
  }
};

inline uint8_t velocity(uint32_t travelUs, uint32_t fastUs, uint32_t slowUs) {
  if (travelUs <= fastUs) return 127;
  if (travelUs >= slowUs) return 1;
  return static_cast<uint8_t>(127 - (travelUs - fastUs) * 126 / (slowUs - fastUs));
}
} // namespace cottage
