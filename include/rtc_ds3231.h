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

/** DS3231 SQW pin mode settings */
enum Ds3231SqwPinMode {
  DS3231_OFF = 0b00100,            /**< Off */
  DS3231_SquareWave1Hz = 0b00000,  /**<  1Hz square wave */
  DS3231_SquareWave1kHz = 0b01000, /**<  1kHz square wave */
  DS3231_SquareWave4kHz = 0b10000, /**<  4kHz square wave */
  DS3231_SquareWave8kHz = 0b11000  /**<  8kHz square wave */
};

/** DS3231 Alarm modes for alarm 1 */
enum Ds3231Alarm1Mode {
  DS3231_A1_PerSecond = 0x0F, /**< Alarm once per second */
  DS3231_A1_Second = 0x0E,    /**< Alarm when seconds match */
  DS3231_A1_Minute = 0x0C,    /**< Alarm when minutes and seconds match */
  DS3231_A1_Hour = 0x08,      /**< Alarm when hours, minutes
                                   and seconds match */
  DS3231_A1_Date = 0x00,      /**< Alarm when date (day of month), hours,
                                   minutes and seconds match */
  DS3231_A1_Day = 0x10        /**< Alarm when day (day of week), hours,
                                   minutes and seconds match */
};
/** DS3231 Alarm modes for alarm 2 */
enum Ds3231Alarm2Mode {
  DS3231_A2_PerMinute = 0x7, /**< Alarm once per minute
                                  (whenever seconds are 0) */
  DS3231_A2_Minute = 0x6,    /**< Alarm when minutes match */
  DS3231_A2_Hour = 0x4,      /**< Alarm when hours and minutes match */
  DS3231_A2_Date = 0x0,      /**< Alarm when date (day of month), hours
                                  and minutes match */
  DS3231_A2_Day = 0x8        /**< Alarm when day (day of week), hours
                                  and minutes match */
};

/**************************************************************************/
/*!
    @brief  RTC based on the DS3231 chip connected via I2C.
*/
/**************************************************************************/
class DS3231 {
 public:
  DS3231(std::unique_ptr<I2CMaster> i2c);

  bool adjust(const DateTime& dt);
  bool begin(void);
  bool lostPower(void);
  DateTime now();
  Ds3231SqwPinMode readSqwPinMode();
  bool writeSqwPinMode(Ds3231SqwPinMode mode);
  bool setAlarm1(const DateTime& dt, Ds3231Alarm1Mode alarm_mode);
  bool setAlarm2(const DateTime& dt, Ds3231Alarm2Mode alarm_mode);
  void disableAlarm(uint8_t alarm_num);
  void clearAlarm(uint8_t alarm_num);
  bool alarmFired(uint8_t alarm_num);
  void enable32K(void);
  void disable32K(void);
  bool isEnabled32K(void);
  float getTemperature();  // in Celcius degree

 private:
  std::unique_ptr<I2CMaster> const i2c_;
};

}  // namespace rtc

#endif  // RTC_DS3231_H_