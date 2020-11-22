/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */

#ifndef RTC_TIMESPAN_H_
#define RTC_TIMESPAN_H_

#include <cstdint>

namespace rtc {

/**
 * Timespan which can represent changes in time with seconds accuracy.
 */
class TimeSpan {
 public:
  /**
   * Create a new TimeSpan object in seconds
   *
   * @param seconds Number of seconds
   */
  TimeSpan(int32_t seconds = 0);

  /**
   * Create a new TimeSpan object using a number of days/hours/minutes/seconds.
   *
   *         e.g. Make a TimeSpan of 3 hours and 45 minutes:
   *
   *             new TimeSpan(0, 3, 45, 0);
   *
   * @param days Number of days
   * @param hours Number of hours
   * @param minutes Number of minutes
   * @param seconds Number of seconds
   */
  TimeSpan(int16_t days, int8_t hours, int8_t minutes, int8_t seconds);

  /**
   * Copy constructor, make a new TimeSpan using an existing one.
   *
   * @param copy The TimeSpan to copy
   */
  TimeSpan(const TimeSpan& copy);

  /**
   * Number of days in the TimeSpan e.g. 4.
   *
   * @return int16_t days
   */
  int16_t days() const { return _seconds / 86400L; }

  /**
   * Number of hours in the TimeSpan.
   *
   * This is not the total hours, it includes the days
   * e.g. 4 days, 3 hours - NOT 99 hours
   *
   * @return int8_t hours
   */
  int8_t hours() const { return _seconds / 3600 % 24; }

  /**
   * Number of minutes in the TimeSpan.
   *
   * This is not the total minutes, it includes days/hours
   * e.g. 4 days, 3 hours, 27 minutes
   *
   * @return int8_t minutes
   */
  int8_t minutes() const { return _seconds / 60 % 60; }

  /**
   * Number of seconds in the TimeSpan.
   *
   * This is not the total seconds, it includes the days/hours/minutes
   * e.g. 4 days, 3 hours, 27 minutes, 7 seconds
   *
   * @return int8_t seconds
   */
  int8_t seconds() const { return _seconds % 60; }

  /**
   * Total number of seconds in the TimeSpan, e.g. 358027
   *
   * @return int32_t seconds
   */
  int32_t totalseconds() const { return _seconds; }

  /**
   * Add two TimeSpans.
   *
   * @param right TimeSpan to add
   * @return New TimeSpan object, sum of left and right
   */
  TimeSpan operator+(const TimeSpan& right);

  /**
   * Subtract a TimeSpan.
   *
   * @param right TimeSpan to subtract
   * @return New TimeSpan object, right subtracted from left
   */
  TimeSpan operator-(const TimeSpan& right);

 protected:
  const int32_t _seconds;  ///< Actual TimeSpan value is stored as seconds.
};

}  // namespace rtc

#endif  // RTC_TIMESPAN_H_
