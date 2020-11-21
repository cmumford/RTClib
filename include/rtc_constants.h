/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */

#pragma once

namespace rtc {

#define SECONDS_PER_DAY 86400L  ///< 60 * 60 * 24
#define SECONDS_FROM_1970_TO_2000 \
  946684800  ///< Unixtime for 2000-01-01 00:00:00, useful for initialization

}  // namespace rtc
