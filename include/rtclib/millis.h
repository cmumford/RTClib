/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */

#ifndef RTC_MILLIS_H_
#define RTC_MILLIS_H_

#include <cstdint>

#include "rtclib/datetime.h"

namespace rtc {

/**
 * RTC using the internal millis() clock, has to be initialized before  use.
 *
 * NOTE: this is immune to millis() rollover events.
 */
class Millis {
 public:
  /**
   * Start the RTC
   *
   * @param dt DateTime object with the date/time to set
   */
  static void begin(const DateTime& dt) { adjust(dt); }

  /**
   * Set the current date/time of the RTC_Millis clock.
   *
   * @param dt DateTime object with the desired date and time
   */
  static void adjust(const DateTime& dt);

  /**
   *  Return a DateTime object containing the current date/time.
   *
   * Note that computing (millis() - lastMillis) is rollover-safe as
   * long as this method is called at least once every 49.7 days.
   *
   * @return DateTime object containing current time
   */
  static DateTime now();

 protected:
  static uint32_t lastUnix;    ///< Unix time from the previous call to now() -
                               ///< prevents rollover issues
  static uint32_t lastMillis;  ///< the millis() value corresponding to the last
                               ///< **full second** of Unix time
};

}  // namespace rtc

#endif  // RTC_MILLIS_H_
