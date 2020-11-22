/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */

#include <cstdint>

namespace rtc {

class SystemClock {
 public:
  /**
   * The number of microseconds since the system started.
   */
  static int64_t microsSinceStart();

  /**
   * The number of milliseconds since the system started.
   */
  static int64_t millisSinceStart();
};

}  // namespace rtc
