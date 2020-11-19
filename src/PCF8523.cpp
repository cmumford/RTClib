/*!
  @section license License

  Original library by JeeLabs https://jeelabs.org/pub/docs/rtclib/, released to
  the public domain.

  This version: MIT (see LICENSE)
*/

#include <rtc_pcf8523.h>

#include <rtc_i2c.h>
#include "RTC_util.h"
namespace rtc {

namespace {

// clang-format off
constexpr uint8_t PCF8523_ADDRESS = 0x68;        ///< I2C address for PCF8523

constexpr uint8_t PCF8523_CLKOUTCONTROL = 0x0F;  ///< Timer and CLKOUT control register
constexpr uint8_t PCF8523_CONTROL_1 = 0x00;      ///< Control and status register 1
constexpr uint8_t PCF8523_CONTROL_2 = 0x01;      ///< Control and status register 2
constexpr uint8_t PCF8523_CONTROL_3 = 0x02;      ///< Control and status register 3
constexpr uint8_t PCF8523_TIMER_B_FRCTL = 0x12;  ///< Timer B source clock frequency control
constexpr uint8_t PCF8523_TIMER_B_VALUE = 0x13;  ///< Timer B value (number clock periods)
constexpr uint8_t PCF8523_OFFSET = 0x0E;         ///< Offset register
constexpr uint8_t PCF8523_STATUSREG = 0x03;      ///< Status register
// clang-format on

}  // anonymous namespace

/**************************************************************************/
/*!
    @brief  Start I2C for the PCF8523 and test succesful connection
    @return True if Wire can find PCF8523 or false otherwise.
*/
/**************************************************************************/
bool PCF8523::begin(void) {
  return i2c_->Ping(PCF8523_ADDRESS);
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
bool PCF8523::lostPower(void) {
  uint8_t value;
  if (!i2c_->ReadRegister(PCF8523_ADDRESS, PCF8523_STATUSREG, &value))
    return false;
  return value >> 7;
}

/**************************************************************************/
/*!
    @brief  Check control register 3 to see if we've run adjust() yet (setting
   the date/time and battery switchover mode)
    @return True if the PCF8523 has been set up, false if not
*/
/**************************************************************************/
bool PCF8523::initialized(void) {
  uint8_t value;
  if (!i2c_->ReadRegister(PCF8523_ADDRESS, PCF8523_CONTROL_3, &value))
    return false;
  return ((value & 0xE0) != 0xE0);  // 0xE0 = standby mode, set after power out
}

/**************************************************************************/
/*!
    @brief  Set the date and time, set battery switchover mode
    @param dt DateTime to set
*/
/**************************************************************************/
bool PCF8523::adjust(const DateTime& dt) {
  auto op = i2c_->CreateWriteOp(PCF8523_ADDRESS, 0x3, "adjust");
  if (!op)
    return false;

  const uint8_t values[7] = {
      bin2bcd(dt.second()),
      bin2bcd(dt.minute()),
      bin2bcd(dt.hour()),
      bin2bcd(dt.day()),
      0,  // day of week.
      bin2bcd(dt.month()),
      bin2bcd(dt.year() - 2000U),
  };

  op->Write(values, sizeof(values));

  // set to battery switchover mode
  op->Restart(PCF8523_ADDRESS, OperationType::WRITE);
  op->WriteByte(PCF8523_CONTROL_3);
  op->WriteByte(0x0);
  return op->Execute();
}

/**************************************************************************/
/*!
    @brief  Get the current date/time
    @return DateTime object containing the current date/time
*/
/**************************************************************************/
DateTime PCF8523::now() {
  auto op = i2c_->CreateReadOp(PCF8523_ADDRESS, 0x3, "now");
  if (!op)
    return false;
  uint8_t values[7];  // for registers 0x00 - 0x06.
  if (!op->Read(values, sizeof(values)))
    return DateTime();
  if (!op->Execute())
    return DateTime();

  const uint8_t ss = bcd2bin(values[0] & 0x7F);
  const uint8_t mm = bcd2bin(values[1]);
  const uint8_t hh = bcd2bin(values[2]);
  const uint8_t d = bcd2bin(values[3]);
  // Skip day of week.
  const uint8_t m = bcd2bin(values[5]);
  const uint16_t y = bcd2bin(values[6]) + 2000U;

  return DateTime(y, m, d, hh, mm, ss);
}

/**************************************************************************/
/*!
    @brief  Resets the STOP bit in register Control_1
*/
/**************************************************************************/
bool PCF8523::start(void) {
  uint8_t ctlreg;
  if (!i2c_->ReadRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1, &ctlreg))
    return false;
  if (ctlreg & (1 << 5)) {
    return i2c_->WriteRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1,
                               ctlreg & ~(1 << 5));
  }
  return true;
}

/**************************************************************************/
/*!
    @brief  Sets the STOP bit in register Control_1
*/
/**************************************************************************/
bool PCF8523::stop(void) {
  uint8_t ctlreg;
  if (!i2c_->ReadRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1, &ctlreg))
    return false;
  if (!(ctlreg & (1 << 5))) {
    return i2c_->WriteRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1,
                               ctlreg | (1 << 5));
  }
  return true;
}

/**************************************************************************/
/*!
    @brief  Is the PCF8523 running? Check the STOP bit in register Control_1
    @return 1 if the RTC is running, 0 if not
*/
/**************************************************************************/
bool PCF8523::isrunning() {
  uint8_t ctlreg;
  if (!i2c_->ReadRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1, &ctlreg))
    return false;

  return !((ctlreg >> 5) & 1);
}

/**************************************************************************/
/*!
    @brief  Read the mode of the INT/SQW pin on the PCF8523
    @return SQW pin mode as a #Pcf8523SqwPinMode enum
*/
/**************************************************************************/
Pcf8523SqwPinMode PCF8523::readSqwPinMode() {
  uint8_t mode;
  if (!i2c_->ReadRegister(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, &mode))
    return PCF8523_OFF;

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
bool PCF8523::writeSqwPinMode(Pcf8523SqwPinMode mode) {
  return i2c_->WriteRegister(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, mode << 3);
}

/**************************************************************************/
/*!
    @brief  Enable the Second Timer (1Hz) Interrupt on the PCF8523.
    @details The INT/SQW pin will pull low for a brief pulse once per second.
*/
/**************************************************************************/
bool PCF8523::enableSecondTimer() {
  uint8_t ctlreg;
  uint8_t clkreg;

  {
    auto op = i2c_->CreateReadOp(PCF8523_ADDRESS, PCF8523_CONTROL_1,
                                 "enableSecondTimer:read");
    if (!op)
      return false;
    op->Read(&ctlreg, sizeof(ctlreg));
    op->Restart(PCF8523_ADDRESS, OperationType::WRITE);
    op->WriteByte(PCF8523_CLKOUTCONTROL);
    op->Restart(PCF8523_ADDRESS, OperationType::READ);
    op->Read(&clkreg, sizeof(clkreg));
    if (!op->Execute())
      return false;
  }

  auto op = i2c_->CreateWriteOp(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL,
                                "enableSecondTimer:write");
  if (!op)
    return false;
  // TAM pulse int. mode (shared with Timer A), CLKOUT (aka SQW) disabled
  op->WriteByte(PCF8523_CLKOUTCONTROL);
  op->WriteByte(clkreg | 0xB8);

  // SIE Second timer int. enable
  op->WriteByte(PCF8523_CONTROL_1);
  op->WriteByte(ctlreg | (1 << 2));
  return op->Execute();
}

/**************************************************************************/
/*!
    @brief  Disable the Second Timer (1Hz) Interrupt on the PCF8523.
*/
/**************************************************************************/
bool PCF8523::disableSecondTimer() {
  // Leave compatible settings intact
  uint8_t ctlreg;
  if (!i2c_->ReadRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1, &ctlreg))
    return false;

  // SIE Second timer int. disable
  return i2c_->WriteRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1,
                             ctlreg & ~(1 << 2));
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
bool PCF8523::enableCountdownTimer(PCF8523TimerClockFreq clkFreq,
                                   uint8_t numPeriods,
                                   uint8_t lowPulseWidth) {
  // Datasheet cautions against updating countdown value while it's running,
  // so disabling allows repeated calls with new values to set new countdowns
  if (!disableCountdownTimer())
    return false;

  // Leave compatible settings intact
  uint8_t ctlreg;
  uint8_t clkreg;

  {
    auto op = i2c_->CreateReadOp(PCF8523_ADDRESS, PCF8523_CONTROL_2,
                                 "enableCountdownTimer:read");
    if (!op)
      return false;
    op->Read(&ctlreg, sizeof(ctlreg));

    op->Restart(PCF8523_ADDRESS, OperationType::WRITE);
    op->WriteByte(PCF8523_CLKOUTCONTROL);
    op->Restart(PCF8523_ADDRESS, OperationType::READ);
    op->Read(&clkreg, sizeof(clkreg));

    if (!op->Execute())
      return false;
  }

  auto op = i2c_->CreateWriteOp(PCF8523_ADDRESS, PCF8523_CONTROL_2,
                                "enableCountdownTimer:write");
  if (!op)
    return false;

  // CTBIE Countdown Timer B Interrupt Enabled
  op->WriteByte(ctlreg |= 0x01);

  // Timer B source clock frequency, optionally int. low pulse width
  op->Restart(PCF8523_ADDRESS, OperationType::WRITE);
  op->WriteByte(PCF8523_TIMER_B_FRCTL);
  op->WriteByte(lowPulseWidth << 4 | clkFreq);

  // Timer B value (number of source clock periods)
  op->Restart(PCF8523_ADDRESS, OperationType::WRITE);
  op->WriteByte(PCF8523_TIMER_B_VALUE);
  op->WriteByte(numPeriods);

  // TBM Timer B pulse int. mode, CLKOUT (aka SQW) disabled, TBC start Timer B
  op->Restart(PCF8523_ADDRESS, OperationType::WRITE);
  op->WriteByte(PCF8523_CLKOUTCONTROL);
  op->WriteByte(clkreg | 0x79);

  return op->Execute();
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
bool PCF8523::enableCountdownTimer(PCF8523TimerClockFreq clkFreq,
                                   uint8_t numPeriods) {
  return enableCountdownTimer(clkFreq, numPeriods, 0);
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
bool PCF8523::disableCountdownTimer() {
  uint8_t clkreg;
  if (!i2c_->ReadRegister(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, &clkreg))
    return false;
  return i2c_->WriteRegister(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL,
                             ~1 & clkreg);
}

/**************************************************************************/
/*!
    @brief  Stop all timers, clear their flags and settings on the PCF8523.
    @details This includes the Countdown Timer, Second Timer, and any CLKOUT
   square wave configured with writeSqwPinMode().
*/
/**************************************************************************/
bool PCF8523::deconfigureAllTimers() {
  disableSecondTimer();  // Surgically clears CONTROL_1

  auto op = i2c_->CreateWriteOp(PCF8523_ADDRESS, PCF8523_CONTROL_2,
                                "deconfigureAllTimers");
  if (!op)
    return false;

  op->WriteByte(0);

  op->Restart(PCF8523_ADDRESS, OperationType::WRITE);
  op->WriteByte(PCF8523_CLKOUTCONTROL);
  op->WriteByte(0);

  op->Restart(PCF8523_ADDRESS, OperationType::WRITE);
  op->WriteByte(PCF8523_TIMER_B_FRCTL);
  op->WriteByte(0);

  op->Restart(PCF8523_ADDRESS, OperationType::WRITE);
  op->WriteByte(PCF8523_TIMER_B_VALUE);
  op->WriteByte(0);

  return op->Execute();
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
bool PCF8523::calibrate(Pcf8523OffsetMode mode, int8_t offset) {
  uint8_t reg = (uint8_t)offset & 0x7F;
  reg |= mode;
  return i2c_->WriteRegister(PCF8523_ADDRESS, PCF8523_OFFSET, reg);
}

}  // namespace rtc
