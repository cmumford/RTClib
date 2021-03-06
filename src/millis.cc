/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */

#include <rtclib/millis.h>

#include <rtclib/system_clock.h>

namespace rtc {

/** Alignment between the milis() timescale and the Unix timescale. These
  two variables are updated on each call to now(), which prevents
  rollover issues. Note that lastMillis is **not** the millis() value
  of the last call to now(): it's the millis() value corresponding to
  the last **full second** of Unix time. */
uint32_t Millis::lastMillis;
uint32_t Millis::lastUnix;

void Millis::adjust(const DateTime& dt) {
  lastMillis = SystemClock::millisSinceStart();
  lastUnix = dt.unixtime();
}

DateTime Millis::now() {
  const uint32_t elapsedSeconds =
      (SystemClock::millisSinceStart() - lastMillis) / 1000;
  lastMillis += elapsedSeconds * 1000;
  lastUnix += elapsedSeconds;
  return lastUnix;
}

}  // namespace rtc
