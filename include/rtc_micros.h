/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */

#ifndef RTC_MICROS_H_
#define RTC_MICROS_H_

#include <cstdint>

#include "rtc_datetime.h"

namespace rtc {

/**************************************************************************/
/*!
    @brief  RTC using the internal micros() clock, has to be initialized before
            use. Unlike Millis, this can be tuned in order to compensate for
            the natural drift of the system clock. Note that now() has to be
            called more frequently than the micros() rollover period, which is
            approximately 71.6 minutes.
*/
/**************************************************************************/
class Micros {
 public:
  /**
   * @brief  Start the RTC
   *
   * @param dt DateTime object with the date/time to set
   */
  static void begin(const DateTime& dt) { adjust(dt); }

  /**
   * @brief  Set the current date/time of the RTC_Micros clock.
   * @param dt DateTime object with the desired date and time
   */
  /**************************************************************************/
  static void adjust(const DateTime& dt);

  /**
   * Adjust the RTC_Micros clock to compensate for system clock drift
   *
   * @param ppm Adjustment to make. A positive adjustment makes the clock
   * faster.
   */
  static void adjustDrift(int ppm);

  /**
   * @brief  Get the current date/time from the RTC_Micros clock.
   *
   * @return DateTime object containing the current date/time
   */
  static DateTime now();

 protected:
  static uint32_t microsPerSecond;  ///< Number of microseconds reported by
                                    ///< micros() per "true" (calibrated) second
  static uint32_t lastUnix;    ///< Unix time from the previous call to now() -
                               ///< prevents rollover issues
  static uint32_t lastMicros;  ///< micros() value corresponding to the last
                               ///< full second of Unix time
};

}  // namespace rtc

#endif  // RTC_MICROS_H_
