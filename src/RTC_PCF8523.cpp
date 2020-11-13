/*!
  @section license License

  Original library by JeeLabs https://jeelabs.org/pub/docs/rtclib/, released to
  the public domain.

  This version: MIT (see LICENSE)
*/

#include <RTClib.h>

#if 0

/**************************************************************************/
/*!
    @brief  Start I2C for the PCF8523 and test succesful connection
    @return True if Wire can find PCF8523 or false otherwise.
*/
/**************************************************************************/
boolean RTC_PCF8523::begin(void) {
  Wire.begin();
  Wire.beginTransmission(PCF8523_ADDRESS);
  if (Wire.endTransmission() == 0)
    return true;
  return false;
}

/**************************************************************************/
/*!
    @brief  Check the status register Oscillator Stop flag to see if the PCF8523
   stopped due to power loss
    @details When battery or external power is first applied, the PCF8523's
   crystal oscillator takes up to 2s to stabilize. During this time adjust()
   cannot clear the 'OS' flag. See datasheet OS flag section for details.
    @return True if the bit is set (oscillator is or has stopped) and false only
   after the bit is cleared, for instance with adjust()
*/
/**************************************************************************/
boolean RTC_PCF8523::lostPower(void) {
  return (read_i2c_register(PCF8523_ADDRESS, PCF8523_STATUSREG) >> 7);
}

/**************************************************************************/
/*!
    @brief  Check control register 3 to see if we've run adjust() yet (setting
   the date/time and battery switchover mode)
    @return True if the PCF8523 has been set up, false if not
*/
/**************************************************************************/
boolean RTC_PCF8523::initialized(void) {
  Wire.beginTransmission(PCF8523_ADDRESS);
  Wire._I2C_WRITE((byte)PCF8523_CONTROL_3);
  Wire.endTransmission();

  Wire.requestFrom(PCF8523_ADDRESS, 1);
  uint8_t ss = Wire._I2C_READ();
  return ((ss & 0xE0) != 0xE0);  // 0xE0 = standby mode, set after power out
}

/**************************************************************************/
/*!
    @brief  Set the date and time, set battery switchover mode
    @param dt DateTime to set
*/
/**************************************************************************/
void RTC_PCF8523::adjust(const DateTime& dt) {
  Wire.beginTransmission(PCF8523_ADDRESS);
  Wire._I2C_WRITE((byte)3);  // start at location 3
  Wire._I2C_WRITE(bin2bcd(dt.second()));
  Wire._I2C_WRITE(bin2bcd(dt.minute()));
  Wire._I2C_WRITE(bin2bcd(dt.hour()));
  Wire._I2C_WRITE(bin2bcd(dt.day()));
  Wire._I2C_WRITE(bin2bcd(0));  // skip weekdays
  Wire._I2C_WRITE(bin2bcd(dt.month()));
  Wire._I2C_WRITE(bin2bcd(dt.year() - 2000U));
  Wire.endTransmission();

  // set to battery switchover mode
  Wire.beginTransmission(PCF8523_ADDRESS);
  Wire._I2C_WRITE((byte)PCF8523_CONTROL_3);
  Wire._I2C_WRITE((byte)0x00);
  Wire.endTransmission();
}

/**************************************************************************/
/*!
    @brief  Get the current date/time
    @return DateTime object containing the current date/time
*/
/**************************************************************************/
DateTime RTC_PCF8523::now() {
  Wire.beginTransmission(PCF8523_ADDRESS);
  Wire._I2C_WRITE((byte)3);
  Wire.endTransmission();

  Wire.requestFrom(PCF8523_ADDRESS, 7);
  uint8_t ss = bcd2bin(Wire._I2C_READ() & 0x7F);
  uint8_t mm = bcd2bin(Wire._I2C_READ());
  uint8_t hh = bcd2bin(Wire._I2C_READ());
  uint8_t d = bcd2bin(Wire._I2C_READ());
  Wire._I2C_READ();  // skip 'weekdays'
  uint8_t m = bcd2bin(Wire._I2C_READ());
  uint16_t y = bcd2bin(Wire._I2C_READ()) + 2000U;

  return DateTime(y, m, d, hh, mm, ss);
}

/**************************************************************************/
/*!
    @brief  Resets the STOP bit in register Control_1
*/
/**************************************************************************/
void RTC_PCF8523::start(void) {
  uint8_t ctlreg = read_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_1);
  if (ctlreg & (1 << 5)) {
    write_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_1, ctlreg & ~(1 << 5));
  }
}

/**************************************************************************/
/*!
    @brief  Sets the STOP bit in register Control_1
*/
/**************************************************************************/
void RTC_PCF8523::stop(void) {
  uint8_t ctlreg = read_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_1);
  if (!(ctlreg & (1 << 5))) {
    write_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_1, ctlreg | (1 << 5));
  }
}

/**************************************************************************/
/*!
    @brief  Is the PCF8523 running? Check the STOP bit in register Control_1
    @return 1 if the RTC is running, 0 if not
*/
/**************************************************************************/
uint8_t RTC_PCF8523::isrunning() {
  uint8_t ctlreg = read_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_1);
  return !((ctlreg >> 5) & 1);
}

/**************************************************************************/
/*!
    @brief  Read the mode of the INT/SQW pin on the PCF8523
    @return SQW pin mode as a #Pcf8523SqwPinMode enum
*/
/**************************************************************************/
Pcf8523SqwPinMode RTC_PCF8523::readSqwPinMode() {
  int mode;

  Wire.beginTransmission(PCF8523_ADDRESS);
  Wire._I2C_WRITE(PCF8523_CLKOUTCONTROL);
  Wire.endTransmission();

  Wire.requestFrom((uint8_t)PCF8523_ADDRESS, (uint8_t)1);
  mode = Wire._I2C_READ();

  mode >>= 3;
  mode &= 0x7;
  return static_cast<Pcf8523SqwPinMode>(mode);
}

/**************************************************************************/
/*!
    @brief  Set the INT/SQW pin mode on the PCF8523
    @param mode The mode to set, see the #Pcf8523SqwPinMode enum for options
*/
/**************************************************************************/
void RTC_PCF8523::writeSqwPinMode(Pcf8523SqwPinMode mode) {
  Wire.beginTransmission(PCF8523_ADDRESS);
  Wire._I2C_WRITE(PCF8523_CLKOUTCONTROL);
  Wire._I2C_WRITE(mode << 3);  // disables other timers
  Wire.endTransmission();
}

/**************************************************************************/
/*!
    @brief  Enable the Second Timer (1Hz) Interrupt on the PCF8523.
    @details The INT/SQW pin will pull low for a brief pulse once per second.
*/
/**************************************************************************/
void RTC_PCF8523::enableSecondTimer() {
  // Leave compatible settings intact
  uint8_t ctlreg = read_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_1);
  uint8_t clkreg = read_i2c_register(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL);

  // TAM pulse int. mode (shared with Timer A), CLKOUT (aka SQW) disabled
  write_i2c_register(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, clkreg | 0xB8);

  // SIE Second timer int. enable
  write_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_1, ctlreg | (1 << 2));
}

/**************************************************************************/
/*!
    @brief  Disable the Second Timer (1Hz) Interrupt on the PCF8523.
*/
/**************************************************************************/
void RTC_PCF8523::disableSecondTimer() {
  // Leave compatible settings intact
  uint8_t ctlreg = read_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_1);

  // SIE Second timer int. disable
  write_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_1, ctlreg & ~(1 << 2));
}

/**************************************************************************/
/*!
    @brief  Enable the Countdown Timer Interrupt on the PCF8523.
    @details The INT/SQW pin will be pulled low at the end of a specified
   countdown period ranging from 244 microseconds to 10.625 days.
    Uses PCF8523 Timer B. Any existing CLKOUT square wave, configured with
   writeSqwPinMode(), will halt. The interrupt low pulse width is adjustable
   from 3/64ths (default) to 14/64ths of a second.
    @param clkFreq One of the PCF8523's Timer Source Clock Frequencies.
   See the #PCF8523TimerClockFreq enum for options and associated time ranges.
    @param numPeriods The number of clkFreq periods (1-255) to count down.
    @param lowPulseWidth Optional: the length of time for the interrupt pin
   low pulse. See the #PCF8523TimerIntPulse enum for options.
*/
/**************************************************************************/
void RTC_PCF8523::enableCountdownTimer(PCF8523TimerClockFreq clkFreq,
                                       uint8_t numPeriods,
                                       uint8_t lowPulseWidth) {
  // Datasheet cautions against updating countdown value while it's running,
  // so disabling allows repeated calls with new values to set new countdowns
  disableCountdownTimer();

  // Leave compatible settings intact
  uint8_t ctlreg = read_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_2);
  uint8_t clkreg = read_i2c_register(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL);

  // CTBIE Countdown Timer B Interrupt Enabled
  write_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_2, ctlreg |= 0x01);

  // Timer B source clock frequency, optionally int. low pulse width
  write_i2c_register(PCF8523_ADDRESS, PCF8523_TIMER_B_FRCTL,
                     lowPulseWidth << 4 | clkFreq);

  // Timer B value (number of source clock periods)
  write_i2c_register(PCF8523_ADDRESS, PCF8523_TIMER_B_VALUE, numPeriods);

  // TBM Timer B pulse int. mode, CLKOUT (aka SQW) disabled, TBC start Timer B
  write_i2c_register(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, clkreg | 0x79);
}

/**************************************************************************/
/*!
    @overload
    @brief  Enable Countdown Timer using default interrupt low pulse width.
    @param clkFreq One of the PCF8523's Timer Source Clock Frequencies.
   See the #PCF8523TimerClockFreq enum for options and associated time ranges.
    @param numPeriods The number of clkFreq periods (1-255) to count down.
*/
/**************************************************************************/
void RTC_PCF8523::enableCountdownTimer(PCF8523TimerClockFreq clkFreq,
                                       uint8_t numPeriods) {
  enableCountdownTimer(clkFreq, numPeriods, 0);
}

/**************************************************************************/
/*!
    @brief  Disable the Countdown Timer Interrupt on the PCF8523.
    @details For simplicity, this function strictly disables Timer B by setting
   TBC to 0. The datasheet describes TBC as the Timer B on/off switch.
   Timer B is the only countdown timer implemented at this time.
   The following flags have no effect while TBC is off, they are *not* cleared:
      - TBM: Timer B will still be set to pulsed mode.
      - CTBIE: Timer B interrupt would be triggered if TBC were on.
      - CTBF: Timer B flag indicates that interrupt was triggered. Though
        typically used for non-pulsed mode, user may wish to query this later.
*/
/**************************************************************************/
void RTC_PCF8523::disableCountdownTimer() {
  // Leave compatible settings intact
  uint8_t clkreg = read_i2c_register(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL);

  // TBC disable to stop Timer B clock
  write_i2c_register(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, ~1 & clkreg);
}

/**************************************************************************/
/*!
    @brief  Stop all timers, clear their flags and settings on the PCF8523.
    @details This includes the Countdown Timer, Second Timer, and any CLKOUT
   square wave configured with writeSqwPinMode().
*/
/**************************************************************************/
void RTC_PCF8523::deconfigureAllTimers() {
  disableSecondTimer();  // Surgically clears CONTROL_1
  write_i2c_register(PCF8523_ADDRESS, PCF8523_CONTROL_2, 0);
  write_i2c_register(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, 0);
  write_i2c_register(PCF8523_ADDRESS, PCF8523_TIMER_B_FRCTL, 0);
  write_i2c_register(PCF8523_ADDRESS, PCF8523_TIMER_B_VALUE, 0);
}

/**************************************************************************/
/*!
    @brief Compensate the drift of the RTC.
    @details This method sets the "offset" register of the PCF8523,
      which can be used to correct a previously measured drift rate.
      Two correction modes are available:

      - **PCF8523\_TwoHours**: Clock adjustments are performed on
        `offset` consecutive minutes every two hours. This is the most
        energy-efficient mode.

      - **PCF8523\_OneMinute**: Clock adjustments are performed on
        `offset` consecutive seconds every minute. Extra adjustments are
        performed on the last second of the minute is `abs(offset)>60`.

      The `offset` parameter sets the correction amount in units of
      roughly 4&nbsp;ppm. The exact unit depends on the selected mode:

      |  mode               | offset unit                            |
      |---------------------|----------------------------------------|
      | `PCF8523_TwoHours`  | 4.340 ppm = 0.375 s/day = 2.625 s/week |
      | `PCF8523_OneMinute` | 4.069 ppm = 0.352 s/day = 2.461 s/week |

      See the accompanying sketch pcf8523.ino for an example on how to
      use this method.

    @param mode Correction mode, either `PCF8523_TwoHours` or
      `PCF8523_OneMinute`.
    @param offset Correction amount, from -64 to +63. A positive offset
      makes the clock slower.
*/
/**************************************************************************/
void RTC_PCF8523::calibrate(Pcf8523OffsetMode mode, int8_t offset) {
  uint8_t reg = (uint8_t)offset & 0x7F;
  reg |= mode;

  Wire.beginTransmission(PCF8523_ADDRESS);
  Wire._I2C_WRITE(PCF8523_OFFSET);
  Wire._I2C_WRITE(reg);
  Wire.endTransmission();
}

#endif