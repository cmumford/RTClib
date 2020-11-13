/*!
  @section license License

  Original library by JeeLabs https://jeelabs.org/pub/docs/rtclib/, released to
  the public domain.

  This version: MIT (see LICENSE)
*/

#include "RTC_util.h"

uint8_t bcd2bin(uint8_t val) {
  return val - 6 * (val >> 4);
}

uint8_t bin2bcd(uint8_t val) {
  return val + 6 * (val / 10);
}