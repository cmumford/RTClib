/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */

#ifndef RTC_DS3231_H_
#define RTC_DS3231_H_

#include <memory>

namespace i2c {
class Master;
}

namespace rtc {

class DateTime;

/**
 * RTC based on the DS3231 chip connected via I2C.
 */
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

  enum class Alarm {
    A1,  ///< Alarm 1.
    A2   ///< Alarm 2.
  };

  DS3231(std::unique_ptr<i2c::Master> i2c);

  /**
   * Set the date and flip the Oscillator Stop Flag.
   *
   * @param dt DateTime object containing the date/time to set.
   */
  bool adjust(const DateTime& dt);

  /**
   * Start I2C for the DS3231 and test succesful connection.
   *
   * @return True if Wire can find DS3231 or false otherwise.
   */
  bool begin(void);

  /**
   * Check the status register Oscillator Stop Flag to see if the DS3231
   * stopped due to power loss.
   *
   * @return True if the bit is set (oscillator stopped) or false if it is
   * running.
   */
  bool lostPower(void);

  /**
   * Retrieve the current time from the clock.
   *
   * @param dt location to write the current time.
   * @return true if successful, false if not.
   */
  bool now(DateTime* dt);

  /**
   * Read the SQW pin mode.
   *
   * @return Pin mode.
   */
  SqwPinMode readSqwPinMode();

  /**
   * Set the SQW pin mode.
   *
   * @param mode Desired mode.
   */
  bool writeSqwPinMode(SqwPinMode mode);

  /**
   * Set alarm 1.
   *
   * @param 	dt DateTime object
   * @param 	alarm_mode Desired mode.
   * @return True if successful, false if error.
   */
  bool setAlarm1(const DateTime& dt, Alarm1Mode alarm_mode);

  /**
   * Set alarm 2.
   *
   * @param 	dt DateTime object
   * @param 	alarm_mode Desired mode.
   * @return True if successful, false if error.
   */
  bool setAlarm2(const DateTime& dt, Alarm2Mode alarm_mode);

  /**
   * Disable the specified alarm.
   *
   * @param alarm The alarm to disable.
   */
  void disableAlarm(Alarm alarm);

  /**
   * Clear status the specified alarm.
   *
   * @param alarm The alarm to clear.
   */
  void clearAlarm(Alarm alarm);

  /**
   * Get alarm status.
   *
   * @param alarm The alarm to retrieve status of.
   * @return True if alarm has been fired otherwise false.
   */
  bool isAlarmFired(Alarm alarm);

  /**
   * Enable 32KHz Output.
   *
   * @details The 32kHz output is enabled by default. It requires an external
   * pull-up resistor to function correctly.
   */
  void enable32K(void);

  /**
   * Disable 32KHz Output.
   */
  void disable32K(void);

  /**
   * Get status of 32KHz Output.
   *
   * @return True if enabled otherwise false.
   */
  bool isEnabled32K(void);

  /**
   * Get the current temperature from the DS3231's temperature sensor.
   *
   * @return Current temperature (degrees C).
   */
  float getTemperature();  // in Celcius degree

  /**
   * @brief Get the Aging Offset.
   *
   * @param aging_offset Location to write aging offset.
   *
   * @return True if successfully retrieved, false upon error.
   */
  bool getAgingOffset(int8_t* aging_offset);

 private:
  std::unique_ptr<i2c::Master> const i2c_;
};

}  // namespace rtc

#endif  // RTC_DS3231_H_
