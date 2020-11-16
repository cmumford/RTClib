/**************************************************************************/
/*!
  @file     RTClib.h

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

#ifndef _RTCLIB_H_
#define _RTCLIB_H_

#include <cstdint>
#include <string>

#include "rtc_datetime.h"
#include "rtc_ds1307.h"
#include "rtc_ds3231.h"
#include "rtc_i2c.h"
#include "rtc_pcf8523.h"
#include "rtc_timespan.h"

namespace rtc {

/** PCF8563 CLKOUT pin mode settings */
enum Pcf8563SqwPinMode {
  PCF8563_SquareWaveOFF = 0x00,  /**< Off */
  PCF8563_SquareWave1Hz = 0x83,  /**< 1Hz square wave */
  PCF8563_SquareWave32Hz = 0x82, /**< 32Hz square wave */
  PCF8563_SquareWave1kHz = 0x81, /**< 1kHz square wave */
  PCF8563_SquareWave32kHz = 0x80 /**< 32kHz square wave */
};

/**************************************************************************/
/*!
    @brief  RTC based on the PCF8563 chip connected via I2C and the Wire library
*/
/**************************************************************************/

class PCF8563 {
 public:
  PCF8563(std::unique_ptr<I2CMaster> i2c);

  bool begin(void);
  bool lostPower(void);
  bool adjust(const DateTime& dt);
  DateTime now();
  bool start(void);
  bool stop(void);
  uint8_t isrunning();
  Pcf8563SqwPinMode readSqwPinMode();
  bool writeSqwPinMode(Pcf8563SqwPinMode mode);

 private:
  std::unique_ptr<I2CMaster> i2c_;
};

/**************************************************************************/
/*!
    @brief  RTC using the internal millis() clock, has to be initialized before
   use. NOTE: this is immune to millis() rollover events.
*/
/**************************************************************************/
class Millis {
 public:
  /*!
      @brief  Start the RTC
      @param dt DateTime object with the date/time to set
  */
  static void begin(const DateTime& dt) { adjust(dt); }
  static void adjust(const DateTime& dt);
  static DateTime now();

 protected:
  static uint32_t lastUnix;    ///< Unix time from the previous call to now() -
                               ///< prevents rollover issues
  static uint32_t lastMillis;  ///< the millis() value corresponding to the last
                               ///< **full second** of Unix time
};

/**************************************************************************/
/*!
    @brief  RTC using the internal micros() clock, has to be initialized before
            use. Unlike Millis, this can be tuned in order to compensate for
            the natural drift of the system clock. Note that now() has to be
            called more frequently than the micros() rollover period, which is
            approximately 71.6 minutes.
*/
/**************************************************************************/
class Micros {
 public:
  /*!
      @brief  Start the RTC
      @param dt DateTime object with the date/time to set
  */
  static void begin(const DateTime& dt) { adjust(dt); }
  static void adjust(const DateTime& dt);
  static void adjustDrift(int ppm);
  static DateTime now();

 protected:
  static uint32_t microsPerSecond;  ///< Number of microseconds reported by
                                    ///< micros() per "true" (calibrated) second
  static uint32_t lastUnix;    ///< Unix time from the previous call to now() -
                               ///< prevents rollover issues
  static uint32_t lastMicros;  ///< micros() value corresponding to the last
                               ///< full second of Unix time
};

}  // namespace rtc

#endif  // _RTCLIB_H_
