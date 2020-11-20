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

#ifndef RTC_DS3231_H_
#define RTC_DS3231_H_

#include <memory>

#include "rtc_datetime.h"

namespace rtc {

class I2CMaster;

/**************************************************************************/
/*!
    @brief  RTC based on the DS3231 chip connected via I2C.
*/
/**************************************************************************/
class DS3231 {
 public:
  /**
   * @brief Square Wave pin mode.
   */
  enum class SqwPinMode {
    Off,       ///< Square wave disabled. Pin goes high during alarm.
    Rate1Hz,   ///<  1Hz square wave.
    Rate1kHz,  ///<  1kHz square wave.
    Rate4kHz,  ///<  4kHz square wave.
    Rate8kHz   ///<  8kHz square wave.
  };

  enum class Alarm1Mode {
    EverySecond,  ///< Alarm once / second.
    Second,       ///< Alarm when seconds match.
    Minute,       ///< Alarm when minutes and seconds match.
    Hour,         ///< Alarm when hours, minutes and seconds match .
    Date,  ///< Alarm when date (day of month), hours, minutes and seconds
           ///< match.
    Day    ///< Alarm when day (day of week), hours, minutes and seconds match.
  };

  enum class Alarm2Mode {
    EveryMinute,  ///< Alarm once per minute (whenever seconds are 0).
    Minute,       ///< Alarm when minutes match.
    Hour,         ///< Alarm when hours and minutes match.
    Date,  ///< Alarm when date (day of month), hours and minutes match */
    Day    ///< Alarm when day (day of week), hours and minutes match.
  };

  DS3231(std::unique_ptr<I2CMaster> i2c);

  bool adjust(const DateTime& dt);
  bool begin(void);
  bool lostPower(void);
  bool now(DateTime* dt);
  SqwPinMode readSqwPinMode();
  bool writeSqwPinMode(SqwPinMode mode);
  bool setAlarm1(const DateTime& dt, Alarm1Mode alarm_mode);
  bool setAlarm2(const DateTime& dt, Alarm2Mode alarm_mode);
  void disableAlarm(uint8_t alarm_num);
  void clearAlarm(uint8_t alarm_num);
  bool alarmFired(uint8_t alarm_num);
  void enable32K(void);
  void disable32K(void);
  bool isEnabled32K(void);
  float getTemperature();  // in Celcius degree
  bool getAgingOffset(int8_t* val);

 private:
  std::unique_ptr<I2CMaster> const i2c_;
};

}  // namespace rtc

#endif  // RTC_DS3231_H_