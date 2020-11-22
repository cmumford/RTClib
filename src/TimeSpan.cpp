/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */

#include <rtc_timespan.h>

#include <rtc_constants.h>

namespace rtc {

TimeSpan::TimeSpan(int32_t seconds) : _seconds(seconds) {}

TimeSpan::TimeSpan(int16_t days, int8_t hours, int8_t minutes, int8_t seconds)
    : _seconds(static_cast<int32_t>(days) * SECONDS_PER_DAY +
               static_cast<int32_t>(hours) * SECONDS_PER_HOUR +
               static_cast<int32_t>(minutes) * 60 + seconds) {}

TimeSpan::TimeSpan(const TimeSpan& copy) : _seconds(copy._seconds) {}

TimeSpan TimeSpan::operator+(const TimeSpan& right) {
  return TimeSpan(_seconds + right._seconds);
}

TimeSpan TimeSpan::operator-(const TimeSpan& right) {
  return TimeSpan(_seconds - right._seconds);
}

}  // namespace rtc
