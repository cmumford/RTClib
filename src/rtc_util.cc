/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */

#include "RTC_util.h"

uint8_t bcd2bin(uint8_t val) {
  return val - 6 * (val >> 4);
}

uint8_t bin2bcd(uint8_t val) {
  return val + 6 * (val / 10);
}