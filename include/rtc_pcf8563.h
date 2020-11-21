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

#ifndef RTC_PCF8563_H_
#define RTC_PCF8563_H_

#include <cstdint>
#include <memory>

#include "rtc_datetime.h"
namespace rtc {

class I2CMaster;

/**************************************************************************/
/*!
    @brief  RTC based on the PCF8563 chip connected via I2C.
*/
/**************************************************************************/

class PCF8563 {
 public:
  enum class SqwPinMode {
    Off,       ///< Off.
    Rate1Hz,   ///< 1Hz square wave.
    Rate32Hz,  ///< 32Hz square wave.
    Rate1kHz,  ///< 1kHz square wave.
    Rate32kHz  ///< 32kHz square wave.
  };

  PCF8563(std::unique_ptr<I2CMaster> i2c);

  /**
   * Start I2C for the PCF8563 and test succesful connection.
   *
   * @return True if Wire can find PCF8563 or false otherwise.
   */
  bool begin(void);

  /**
   * Check the status of the VL bit in the VL_SECONDS register.
   *
   * The PCF8563 has an on-chip voltage-low detector. When VDD drops
   * below Vlow, bit VL in the VL_seconds register is set to indicate that
   * the integrity of the clock information is no longer guaranteed.
   *
   * @return True if the bit is set (VDD droped below Vlow) indicating that
   *         the clock integrity is not guaranteed and false only after the bit
   *         is cleared using adjust().
   */
  bool lostPower(void);

  /**
   * Set the date and time.
   *
   * @param dt DateTime to set
   * @return True if set, false upon error.
   */
  bool adjust(const DateTime& dt);

  /**
   * Get the current date/time.
   *
   * @param dt Location to receive current time.
   * @return True of successfully retrieved, false upon error.
   */
  bool now(DateTime* dt);

  /**
   * Resets the STOP bit in register Control_1.
   *
   * @return True if successful, false if error.
   */
  bool start(void);

  /**
   * Sets the STOP bit in register Control_1.
   *
   * @return True if successful, false if error.
   */
  bool stop(void);

  /**
   * Is the PCF8563 running?
   *
   * Check the STOP bit in register Control_1.
   * @return True if successful, false if error.
   */
  bool isrunning();

  /**
   * Read the mode of the CLKOUT pin on the PCF8563.
   *
   * @return The current square ware pin mode.
   */
  SqwPinMode readSqwPinMode();

  /**
   * Set the CLKOUT pin mode on the PCF8563.
   *
   * @param mode The pin mode to set.
   * @return True if successful, false if error.
   */
  bool writeSqwPinMode(SqwPinMode mode);

 private:
  std::unique_ptr<I2CMaster> i2c_;
};

}  // namespace rtc

#endif  // RTC_PCF8563_H_