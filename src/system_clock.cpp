/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */

#include <rtclib/system_clock.h>

#include <cstdint>

#include <esp_timer.h>

namespace rtc {

int64_t SystemClock::microsSinceStart() {
  return esp_timer_get_time();
}

int64_t SystemClock::millisSinceStart() {
  return microsSinceStart() / 1000;
}

}  // namespace rtc
