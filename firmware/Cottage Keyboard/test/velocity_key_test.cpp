// Run from the firmware directory:
// c++ -std=c++17 -Wall -Wextra -Werror -Iinclude test/velocity_key_test.cpp -o /tmp/cott-velocity-test
// /tmp/cott-velocity-test
#include "velocity_key.h"
#include <cassert>
#include <iostream>
using cottage::KeyEvent;

struct Keybed {
  cottage::VelocityKey key;
  uint32_t now = 0;
  KeyEvent sample(bool a, bool b) {
    const auto event = key.sample(a, b, now, now + 40);
    now += 1000;
    return event;
  }
  void idle() { assert(sample(false, false) == KeyEvent::None); }
};

int main() {
  Keybed bed;
  bed.idle();
  assert(bed.sample(true, false) == KeyEvent::None);
  assert(bed.sample(true, false) == KeyEvent::None);
  assert(bed.sample(true, true) == KeyEvent::None);
  assert(bed.sample(true, true) == KeyEvent::On);
  assert(bed.key.travelUs == 2040);
  for (int i = 0; i < 20; ++i)
    assert(bed.sample(true, true) == KeyEvent::None);
  // Brief release bounce must not end the note.
  assert(bed.sample(true, false) == KeyEvent::None);
  assert(bed.sample(true, true) == KeyEvent::None);
  assert(bed.sample(true, false) == KeyEvent::None);
  assert(bed.sample(true, false) == KeyEvent::None);
  assert(bed.sample(true, false) == KeyEvent::Off);
  // B bouncing closed cannot retrigger while A remains closed.
  for (int i = 0; i < 4; ++i)
    assert(bed.sample(true, true) == KeyEvent::None);
  for (int i = 0; i < 5; ++i) bed.idle();
  assert(bed.sample(true, false) == KeyEvent::None);
  assert(bed.sample(true, true) == KeyEvent::None);
  assert(bed.sample(true, true) == KeyEvent::On);

  Keybed incomplete;
  incomplete.idle();
  incomplete.sample(true, false);
  incomplete.sample(true, false);
  for (int i = 0; i < 5; ++i) incomplete.idle();
  assert(incomplete.key.phase == cottage::VelocityKey::Phase::Ready);

  Keybed reversed;
  reversed.idle();
  for (int i = 0; i < 4; ++i)
    assert(reversed.sample(false, true) == KeyEvent::None);
  for (int i = 0; i < 4; ++i)
    assert(reversed.sample(true, true) == KeyEvent::None);

  Keybed startup;
  for (int i = 0; i < 5; ++i)
    assert(startup.sample(true, true) == KeyEvent::None);
  for (int i = 0; i < 5; ++i) startup.idle();
  assert(startup.key.phase == cottage::VelocityKey::Phase::Ready);

  Keybed wrap;
  wrap.now = 0xfffff000u;
  wrap.idle();
  wrap.sample(true, false);
  wrap.sample(true, false);
  wrap.sample(true, true);
  assert(wrap.sample(true, true) == KeyEvent::On);
  assert(wrap.key.travelUs == 2040);

  Keybed bounce;
  bounce.idle();
  bounce.sample(true, false);
  bounce.sample(false, false);
  bounce.sample(true, false);
  bounce.sample(true, false);
  bounce.sample(true, true);
  assert(bounce.sample(true, true) == KeyEvent::On);
  assert(bounce.key.travelUs == 2040);

  Keybed chord[14];
  for (auto &key : chord) {
    key.idle();
    key.sample(true, false);
    key.sample(true, false);
    key.sample(true, true);
    assert(key.sample(true, true) == KeyEvent::On);
  }
  assert(cottage::velocity(0, 2000, 80000) == 127);
  assert(cottage::velocity(2000, 2000, 80000) == 127);
  assert(cottage::velocity(41000, 2000, 80000) == 64);
  assert(cottage::velocity(80000, 2000, 80000) == 1);
  assert(cottage::velocity(1000000, 2000, 80000) == 1);
  for (uint32_t t = 1; t < 100000; ++t)
    assert(cottage::velocity(t, 2000, 80000) <= cottage::velocity(t - 1, 2000, 80000));
  std::cout << "Velocity key tests passed\n";
}
