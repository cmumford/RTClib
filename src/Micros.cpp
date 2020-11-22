/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */

#include <rtc_micros.h>

#include <cstdint>

namespace rtc {

/**
 * Number of microseconds reported by micros() per "true" (calibrated) second.
 */
uint32_t Micros::microsPerSecond = 1000000;

/**
 * The timing logic is identical to RTC_Millis.
 */
uint32_t Micros::lastMicros;
uint32_t Micros::lastUnix;

namespace {

uint32_t microsSinceStart() {
  // TODO: Implement me.
  return 0;
}

}  // anonymous namespace

void Micros::adjust(const DateTime& dt) {
  lastMicros = microsSinceStart();
  lastUnix = dt.unixtime();
}

void Micros::adjustDrift(int ppm) {
  microsPerSecond = 1000000 - ppm;
}

DateTime Micros::now() {
  uint32_t elapsedSeconds = (microsSinceStart() - lastMicros) / microsPerSecond;
  lastMicros += elapsedSeconds * microsPerSecond;
  lastUnix += elapsedSeconds;
  return lastUnix;
}

}  // namespace rtc
