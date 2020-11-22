/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */

#ifndef RTC_PCF8523_H_
#define RTC_PCF8523_H_

#include <cstdint>
#include <memory>

namespace rtc {

class DateTime;
class I2CMaster;

/** PCF8523 INT/SQW pin mode settings */
enum Pcf8523SqwPinMode {
  PCF8523_OFF = 7,             /**< Off */
  PCF8523_SquareWave1HZ = 6,   /**< 1Hz square wave */
  PCF8523_SquareWave32HZ = 5,  /**< 32Hz square wave */
  PCF8523_SquareWave1kHz = 4,  /**< 1kHz square wave */
  PCF8523_SquareWave4kHz = 3,  /**< 4kHz square wave */
  PCF8523_SquareWave8kHz = 2,  /**< 8kHz square wave */
  PCF8523_SquareWave16kHz = 1, /**< 16kHz square wave */
  PCF8523_SquareWave32kHz = 0  /**< 32kHz square wave */
};

/** PCF8523 Timer Source Clock Frequencies for Timers A and B */
enum PCF8523TimerClockFreq {
  PCF8523_Frequency4kHz = 0,   /**< 1/4096th second = 244 microseconds,
                                    max 62.256 milliseconds */
  PCF8523_Frequency64Hz = 1,   /**< 1/64th second = 15.625 milliseconds,
                                    max 3.984375 seconds */
  PCF8523_FrequencySecond = 2, /**< 1 second, max 255 seconds = 4.25 minutes */
  PCF8523_FrequencyMinute = 3, /**< 1 minute, max 255 minutes = 4.25 hours */
  PCF8523_FrequencyHour = 4,   /**< 1 hour, max 255 hours = 10.625 days */
};

/** PCF8523 Timer Interrupt Low Pulse Width options for Timer B only */
enum PCF8523TimerIntPulse {
  PCF8523_LowPulse3x64Hz = 0,  /**<  46.875 ms   3/64ths second */
  PCF8523_LowPulse4x64Hz = 1,  /**<  62.500 ms   4/64ths second */
  PCF8523_LowPulse5x64Hz = 2,  /**<  78.125 ms   5/64ths second */
  PCF8523_LowPulse6x64Hz = 3,  /**<  93.750 ms   6/64ths second */
  PCF8523_LowPulse8x64Hz = 4,  /**< 125.000 ms   8/64ths second */
  PCF8523_LowPulse10x64Hz = 5, /**< 156.250 ms  10/64ths second */
  PCF8523_LowPulse12x64Hz = 6, /**< 187.500 ms  12/64ths second */
  PCF8523_LowPulse14x64Hz = 7  /**< 218.750 ms  14/64ths second */
};

/** PCF8523 Offset modes for making temperature/aging/accuracy adjustments */
enum Pcf8523OffsetMode {
  PCF8523_TwoHours = 0x00, /**< Offset made every two hours */
  PCF8523_OneMinute = 0x80 /**< Offset made every minute */
};

/**
 * RTC based on the PCF8523 chip connected via I2C.
 */
class PCF8523 {
 public:
  PCF8523(std::unique_ptr<I2CMaster> i2c);

  /**
   * Start I2C for the PCF8523 and test succesful connection.
   *
   * @return True if Wire can find PCF8523 or false otherwise.
   */
  bool begin(void);

  /**
   * Set the date and time, set battery switchover mode.
   *
   * @param dt DateTime to set.
   * @return True if successful, false if not.
   */
  bool adjust(const DateTime& dt);

  /**
   * Check the status register Oscillator Stop flag to see if the
   * PCF8523 stopped due to power loss.
   *
   * When battery or external power is first applied, the PCF8523's
   * crystal oscillator takes up to 2s to stabilize. During this time adjust()
   * cannot clear the 'OS' flag. See datasheet OS flag section for details.
   *
   * @return True if the bit is set (oscillator is or has stopped) and false
   * only after the bit is cleared, for instance with adjust().
   */
  bool lostPower(void);

  /**
   * Check control register 3 to see if we've run adjust() yet (setting
   * the date/time and battery switchover mode).
   *
   * @return True if the PCF8523 has been set up, false if not.
   */
  bool initialized(void);

  /**
   * Get the current date/time.
   *
   * @param dt Location to write current date/time.
   * @return True if successful, false if not.
   */
  bool now(DateTime* dt);

  /**
   * Resets the STOP bit in register Control_1.
   *
   * @return True if successful, false if not.
   */
  bool start(void);

  /**
   * Sets the STOP bit in register Control_1.
   *
   * @return True if successful, false if not.
   */
  bool stop(void);

  /**
   * Is the PCF8523 running?
   *
   * Check the STOP bit in register Control_1.
   *
   * @return True if running, false if not.
   */
  bool isRunning();

  /**
   * Read the mode of the INT/SQW pin on the PCF8523.
   *
   * @return SQW pin mode as a #Pcf8523SqwPinMode enum
   */
  Pcf8523SqwPinMode readSqwPinMode();

  /**
   * Set the INT/SQW pin mode on the PCF8523.
   *
   * @param mode The mode to set, see the #Pcf8523SqwPinMode enum for options.
   *
   * @return True if successful, false if not.
   */
  bool writeSqwPinMode(Pcf8523SqwPinMode mode);

  /**
   * Enable the Second Timer (1Hz) Interrupt on the PCF8523.
   *
   * @return True if successful, false if not.
   */
  bool enableSecondTimer(void);

  /**
   * Disable the Second Timer (1Hz) Interrupt on the PCF8523.
   *
   * @return True if successful, false if not.
   */
  bool disableSecondTimer(void);

  /**
   * Enable the Countdown Timer Interrupt on the PCF8523.
   *
   * The INT/SQW pin will be pulled low at the end of a specified
   * countdown period ranging from 244 microseconds to 10.625 days.
   * Uses PCF8523 Timer B. Any existing CLKOUT square wave, configured with
   * writeSqwPinMode(), will halt. The interrupt low pulse width is adjustable
   * from 3/64ths (default) to 14/64ths of a second.
   *
   * @param clkFreq One of the PCF8523's Timer Source Clock Frequencies.
   *                See the #PCF8523TimerClockFreq enum for options and
   *                associated time ranges.
   *  @param numPeriods The number of clkFreq periods (1-255) to count down.
   *  @param lowPulseWidth Optional: the length of time for the interrupt pin
   *                       low pulse. See the #PCF8523TimerIntPulse enum for
   *                        options.
   * @return True if successful, false if not.
   */
  bool enableCountdownTimer(PCF8523TimerClockFreq clkFreq,
                            uint8_t numPeriods,
                            uint8_t lowPulseWidth);

  /**
   * Enable Countdown Timer using default interrupt low pulse width.
   *
   * @param clkFreq One of the PCF8523's Timer Source Clock Frequencies.
   *                See the #PCF8523TimerClockFreq enum for options and
   *                associated time ranges.
   * @param numPeriods The number of clkFreq periods (1-255) to count down.
   * @return True if successful, false if not.
   */
  bool enableCountdownTimer(PCF8523TimerClockFreq clkFreq, uint8_t numPeriods);

  /**
   * Disable the Countdown Timer Interrupt on the PCF8523.
   *
   * For simplicity, this function strictly disables Timer B by
   * setting TBC to 0. The datasheet describes TBC as the Timer B on/off switch.
   * Timer B is the only countdown timer implemented at this time.
   * The following flags have no effect while TBC is off, they are *not*
   * cleared:
   *    - TBM: Timer B will still be set to pulsed mode.
   *    - CTBIE: Timer B interrupt would be triggered if TBC were on.
   *    - CTBF: Timer B flag indicates that interrupt was triggered. Though
   *      typically used for non-pulsed mode, user may wish to query this later.
   *
   * @return True if successful, false if not.
   */
  bool disableCountdownTimer();

  /**
   * Stop all timers, clear their flags and settings on the PCF8523.
   *
   * This includes the Countdown Timer, Second Timer, and any CLKOUT
   * square wave configured with writeSqwPinMode().
   * @return True if successful, false if not.
   */
  bool deconfigureAllTimers();

  /**
   *  @brief Compensate the drift of the RTC.
   *  @details This method sets the "offset" register of the PCF8523,
   *    which can be used to correct a previously measured drift rate.
   *    Two correction modes are available:
   *
   *    - **PCF8523\_TwoHours**: Clock adjustments are performed on
   *      `offset` consecutive minutes every two hours. This is the most
   *      energy-efficient mode.
   *
   *    - **PCF8523\_OneMinute**: Clock adjustments are performed on
   *      `offset` consecutive seconds every minute. Extra adjustments are
   *      performed on the last second of the minute is `abs(offset)>60`.
   *
   *    The `offset` parameter sets the correction amount in units of
   *    roughly 4&nbsp;ppm. The exact unit depends on the selected mode:
   *
   *    |  mode               | offset unit                            |
   *    |---------------------|----------------------------------------|
   *    | `PCF8523_TwoHours`  | 4.340 ppm = 0.375 s/day = 2.625 s/week |
   *    | `PCF8523_OneMinute` | 4.069 ppm = 0.352 s/day = 2.461 s/week |
   *
   *    See the accompanying sketch pcf8523.ino for an example on how to
   *    use this method.
   *
   *  @param mode Correction mode, either `PCF8523_TwoHours` or
   *    `PCF8523_OneMinute`.
   *  @param offset Correction amount, from -64 to +63. A positive offset
   *    makes the clock slower.
   * @return True if successful, false if not.
   */
  bool calibrate(Pcf8523OffsetMode mode, int8_t offset);

 private:
  std::unique_ptr<I2CMaster> i2c_;
};

}  // namespace rtc

#endif  // RTC_PCF8523_H_
