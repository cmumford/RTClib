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

/** DS1307 SQW pin mode settings */
enum Ds1307SqwPinMode {
  DS1307_SquareWaveOff = 0x0,          // Set SQW/OUT pin set to zero.
  DS1307_SquareWaveOn = 0b10000000,    // Set SQW/OUT pin set to 1.
  DS1307_SquareWave1Hz = 0b00010000,   // 1Hz square wave.
  DS1307_SquareWave4kHz = 0b00010001,  // 4kHz square wave.
  DS1307_SquareWave8kHz = 0b00010010,  // 8kHz square wave.
  DS1307_SquareWave32kHz = 0b00010011  // 32kHz square wave.
};

/**************************************************************************/
/*!
    @brief  RTC based on the DS1307 chip connected via I2C and the Wire library
*/
/**************************************************************************/
class DS1307 {
 public:
  DS1307(std::unique_ptr<I2CMaster> i2c);

  bool begin(void);
  bool adjust(const DateTime& dt);
  uint8_t isrunning(void);
  DateTime now();
  Ds1307SqwPinMode readSqwPinMode();
  bool writeSqwPinMode(Ds1307SqwPinMode mode);
  bool readnvram(uint8_t address, void* buf, size_t num_bytes);
  bool writenvram(uint8_t address, const void* buf, size_t num_bytes);

 private:
  std::unique_ptr<I2CMaster> const i2c_;
};

}  // namespace rtc

#endif  // RTC_DS1307_H_