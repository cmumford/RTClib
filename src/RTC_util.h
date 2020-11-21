/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */

#ifndef RTC_UTIL_H_
#define RTC_UTIL_H_

#include <cstdint>

#if !defined(SET_BITS)
#define SET_BITS(value, bits) (value |= (bits))
#endif

#if !defined(CLEAR_BITS)
#define CLEAR_BITS(value, bits) (value &= ~(bits))
#endif

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