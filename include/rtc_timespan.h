/**************************************************************************/
/*!
  Original library by JeeLabs http://news.jeelabs.org/code/, released to the
  public domain

  License: MIT (see LICENSE)

  This is a fork of JeeLab's fantastic real time clock library for Arduino.

  For details on using this library with an RTC module like the DS1307, PCF8523,
  or DS3231, see the guide at:
  https://learn.adafruit.com/ds1307-real-time-clock-breakout-board-kit/overview

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
*/
/**************************************************************************/

#ifndef RTC_TIMESPAN_H_
#define RTC_TIMESPAN_H_

#include <cstdint>

namespace rtc {

/**************************************************************************/
/*!
    @brief  Timespan which can represent changes in time with seconds accuracy.
*/
/**************************************************************************/
class TimeSpan {
 public:
  /*!
    @brief  Create a new TimeSpan object in seconds
    @param seconds Number of seconds
  */
  TimeSpan(int32_t seconds = 0);

  /*!
    @brief  Create a new TimeSpan object using a number of
            days/hours/minutes/seconds
            e.g. Make a TimeSpan of 3 hours and 45 minutes:
            new TimeSpan(0, 3, 45, 0);
    @param days Number of days
    @param hours Number of hours
    @param minutes Number of minutes
    @param seconds Number of seconds
  */
  TimeSpan(int16_t days, int8_t hours, int8_t minutes, int8_t seconds);

  /*!
    @brief  Copy constructor, make a new TimeSpan using an existing one
    @param copy The TimeSpan to copy
  */
  TimeSpan(const TimeSpan& copy);

  /*!
    @brief  Number of days in the TimeSpan
            e.g. 4
    @return int16_t days
  */
  int16_t days() const { return _seconds / 86400L; }

  /*!
    @brief  Number of hours in the TimeSpan
            This is not the total hours, it includes the days
            e.g. 4 days, 3 hours - NOT 99 hours
    @return int8_t hours
  */
  int8_t hours() const { return _seconds / 3600 % 24; }

  /*!
    @brief  Number of minutes in the TimeSpan
            This is not the total minutes, it includes days/hours
            e.g. 4 days, 3 hours, 27 minutes
    @return int8_t minutes
  */
  int8_t minutes() const { return _seconds / 60 % 60; }

  /*!
    @brief  Number of seconds in the TimeSpan
            This is not the total seconds, it includes the days/hours/minutes
            e.g. 4 days, 3 hours, 27 minutes, 7 seconds
    @return int8_t seconds
  */
  int8_t seconds() const { return _seconds % 60; }

  /*!
    @brief  Total number of seconds in the TimeSpan, e.g. 358027
    @return int32_t seconds
  */
  int32_t totalseconds() const { return _seconds; }

  /*!
    @brief  Add two TimeSpans
    @param right TimeSpan to add
    @return New TimeSpan object, sum of left and right
  */
  TimeSpan operator+(const TimeSpan& right);

  /*!
    @brief  Subtract a TimeSpan
    @param right TimeSpan to subtract
    @return New TimeSpan object, right subtracted from left
  */
  TimeSpan operator-(const TimeSpan& right);

 protected:
  int32_t _seconds;  ///< Actual TimeSpan value is stored as seconds
};

}  // namespace rtc

#endif  // RTC_TIMESPAN_H_