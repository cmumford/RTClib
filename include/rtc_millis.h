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

#ifndef RTC_MILLIS_H_
#define RTC_MILLIS_H_

#include <cstdint>

#include "rtc_datetime.h"

namespace rtc {

/**************************************************************************/
/*!
    @brief  RTC using the internal millis() clock, has to be initialized before
   use. NOTE: this is immune to millis() rollover events.
*/
/**************************************************************************/
class Millis {
 public:
  /*!
      @brief  Start the RTC
      @param dt DateTime object with the date/time to set
  */
  static void begin(const DateTime& dt) { adjust(dt); }
  static void adjust(const DateTime& dt);
  static DateTime now();

 protected:
  static uint32_t lastUnix;    ///< Unix time from the previous call to now() -
                               ///< prevents rollover issues
  static uint32_t lastMillis;  ///< the millis() value corresponding to the last
                               ///< **full second** of Unix time
};

}  // namespace rtc

#endif  // RTC_MILLIS_H_