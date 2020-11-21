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

#ifndef RTC_DS1307_H_
#define RTC_DS1307_H_

#include <memory>

#include "rtc_datetime.h"
namespace rtc {

class I2CMaster;

/**************************************************************************/
/*!
    @brief  RTC based on the DS1307 chip connected via I2C.
*/
/**************************************************************************/
class DS1307 {
 public:
  enum class SqwPinMode {
    Off,       // Set SQW/OUT pin set to zero.
    On,        // Set SQW/OUT pin set to 1.
    Rate1Hz,   // 1Hz square wave.
    Rate4kHz,  // 4kHz square wave.
    Rate8kHz,  // 8kHz square wave.
    Rate32kHz  // 32kHz square wave.
  };

  DS1307(std::unique_ptr<I2CMaster> i2c);

  bool begin(void);
  bool adjust(const DateTime& dt);
  bool isrunning(void);
  bool now(DateTime* now);
  SqwPinMode readSqwPinMode();
  bool writeSqwPinMode(SqwPinMode mode);
  bool readnvram(uint8_t address, void* buf, size_t num_bytes);
  bool writenvram(uint8_t address, const void* buf, size_t num_bytes);

 private:
  std::unique_ptr<I2CMaster> const i2c_;
};

}  // namespace rtc

#endif  // RTC_DS1307_H_