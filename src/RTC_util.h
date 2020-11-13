/*!
  @section license License

  Original library by JeeLabs https://jeelabs.org/pub/docs/rtclib/, released to
  the public domain.

  This version: MIT (see LICENSE)
*/

#ifndef RTC_UTIL_H_
#define RTC_UTIL_H_

#include <cstdint>

/**************************************************************************/
/*!
    @brief  Convert a binary coded decimal value to binary. RTC stores time/date
   values as BCD.
    @param val BCD value
    @return Binary value
*/
/**************************************************************************/
uint8_t bcd2bin(uint8_t val);

/**************************************************************************/
/*!
    @brief  Convert a binary value to BCD format for the RTC registers
    @param val Binary value
    @return BCD value
*/
/**************************************************************************/
uint8_t bin2bcd(uint8_t val);

#endif  // #define RTC_UTIL_H_