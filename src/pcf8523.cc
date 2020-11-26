/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */

#include <rtclib/pcf8523.h>

#include <i2clib/master.h>
#include <i2clib/operation.h>
#include <rtclib/datetime.h>
#include "rtc_util.h"

using i2c::Operation;

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

constexpr uint8_t CLKOUT_SQW_32kHz = 0b00000000;
constexpr uint8_t CLKOUT_SQW_16kHz = 0b00001000;
constexpr uint8_t CLKOUT_SQW_8kHz  = 0b00010000;
constexpr uint8_t CLKOUT_SQW_4kHz  = 0b00011000;
constexpr uint8_t CLKOUT_SQW_1kHz  = 0b00100000;
constexpr uint8_t CLKOUT_SQW_32Hz  = 0b00101000;
constexpr uint8_t CLKOUT_SQW_1Hz   = 0b00110000;
constexpr uint8_t CLKOUT_SQW_Off   = 0b00111000;
constexpr uint8_t CLKOUT_SQW_MASK  = 0b00111000;

// clang-format on

}  // anonymous namespace

PCF8523::PCF8523(i2c::Master i2c) : i2c_(std::move(i2c)) {}

bool PCF8523::begin(void) {
  return i2c_.Ping(PCF8523_ADDRESS);
}

bool PCF8523::lostPower(void) {
  uint8_t value;
  if (!i2c_.ReadRegister(PCF8523_ADDRESS, PCF8523_STATUSREG, &value))
    return false;
  return value >> 7;
}

bool PCF8523::initialized(void) {
  uint8_t value;
  if (!i2c_.ReadRegister(PCF8523_ADDRESS, PCF8523_CONTROL_3, &value))
    return false;
  return ((value & 0xE0) != 0xE0);  // 0xE0 = standby mode, set after power out
}

bool PCF8523::adjust(const DateTime& dt) {
  auto op = i2c_.CreateWriteOp(PCF8523_ADDRESS, 0x3, "adjust");
  if (!op.ready())
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

  op.Write(values, sizeof(values));

  // set to battery switchover mode
  op.RestartReg(PCF8523_CONTROL_3, Operation::Type::WRITE);
  op.WriteByte(0x0);
  return op.Execute();
}

bool PCF8523::now(DateTime* dt) {
  auto op = i2c_.CreateReadOp(PCF8523_ADDRESS, 0x3, "now");
  if (!op.ready())
    return false;
  uint8_t values[7];  // for registers 0x00 - 0x06.
  if (!op.Read(values, sizeof(values)))
    return false;
  if (!op.Execute())
    return false;

  const uint8_t ss = bcd2bin(values[0] & 0x7F);
  const uint8_t mm = bcd2bin(values[1]);
  const uint8_t hh = bcd2bin(values[2]);
  const uint8_t d = bcd2bin(values[3]);
  // Skip day of week.
  const uint8_t m = bcd2bin(values[5]);
  const uint16_t y = bcd2bin(values[6]) + 2000U;

  *dt = DateTime(y, m, d, hh, mm, ss);
  return true;
}

bool PCF8523::start(void) {
  uint8_t ctlreg;
  if (!i2c_.ReadRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1, &ctlreg))
    return false;
  if (ctlreg & (1 << 5)) {
    return i2c_.WriteRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1,
                              ctlreg & ~(1 << 5));
  }
  return true;
}

bool PCF8523::stop(void) {
  uint8_t ctlreg;
  if (!i2c_.ReadRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1, &ctlreg))
    return false;
  if (!(ctlreg & (1 << 5))) {
    return i2c_.WriteRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1,
                              ctlreg | (1 << 5));
  }
  return true;
}

bool PCF8523::isRunning() {
  uint8_t ctlreg;
  if (!i2c_.ReadRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1, &ctlreg))
    return false;

  return !((ctlreg >> 5) & 1);
}

PCF8523::SqwPinMode PCF8523::readSqwPinMode() {
  uint8_t mode;
  if (!i2c_.ReadRegister(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, &mode))
    return SqwPinMode::Off;

  switch (mode & CLKOUT_SQW_MASK) {  // COF[2:0]
    case CLKOUT_SQW_32kHz:
      return SqwPinMode::Rate32kHz;
    case CLKOUT_SQW_16kHz:
      return SqwPinMode::Rate16kHz;
    case CLKOUT_SQW_8kHz:
      return SqwPinMode::Rate8kHz;
    case CLKOUT_SQW_4kHz:
      return SqwPinMode::Rate4kHz;
    case CLKOUT_SQW_1kHz:
      return SqwPinMode::Rate1kHz;
    case CLKOUT_SQW_32Hz:
      return SqwPinMode::Rate32Hz;
    case CLKOUT_SQW_1Hz:
      return SqwPinMode::Rate1Hz;
    case CLKOUT_SQW_Off:
      // Fallthrough
    default:
      return SqwPinMode::Off;
  }
}

bool PCF8523::writeSqwPinMode(PCF8523::SqwPinMode mode) {
  uint8_t reg = 0x0;

  // TODO: Should this function preserve the other PCF8523_CLKOUTCONTROL
  // register bits? Most of those are alarm bits, and it's doubtful that
  // one would want to use alarms as well as the square wave feature.
  // Should probably **only** set the square wave bits (i.e. COF[2:0])
  // in this function, and provide API to set the others.

  switch (mode) {
    case SqwPinMode::Off:
      SET_BITS(reg, CLKOUT_SQW_Off);
      break;
    case SqwPinMode::Rate1Hz:
      SET_BITS(reg, CLKOUT_SQW_1Hz);
      break;
    case SqwPinMode::Rate32Hz:
      SET_BITS(reg, CLKOUT_SQW_32Hz);
      break;
    case SqwPinMode::Rate1kHz:
      SET_BITS(reg, CLKOUT_SQW_1kHz);
      break;
    case SqwPinMode::Rate4kHz:
      SET_BITS(reg, CLKOUT_SQW_4kHz);
      break;
    case SqwPinMode::Rate8kHz:
      SET_BITS(reg, CLKOUT_SQW_8kHz);
      break;
    case SqwPinMode::Rate16kHz:
      SET_BITS(reg, CLKOUT_SQW_16kHz);
      break;
    case SqwPinMode::Rate32kHz:
      SET_BITS(reg, CLKOUT_SQW_32kHz);
      break;
  }
  return i2c_.WriteRegister(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, reg);
}

bool PCF8523::enableSecondTimer() {
  uint8_t ctlreg;
  uint8_t clkreg;

  {
    auto op = i2c_.CreateReadOp(PCF8523_ADDRESS, PCF8523_CONTROL_1,
                                "enableSecondTimer:read");
    if (!op.ready())
      return false;
    op.Read(&ctlreg, sizeof(ctlreg));
    op.RestartReg(PCF8523_CLKOUTCONTROL, Operation::Type::READ);
    op.Read(&clkreg, sizeof(clkreg));
    if (!op.Execute())
      return false;
  }

  auto op = i2c_.CreateWriteOp(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL,
                               "enableSecondTimer:write");
  if (!op.ready())
    return false;
  // TAM pulse int. mode (shared with Timer A), CLKOUT (aka SQW) disabled
  op.WriteByte(clkreg | 0xB8);

  // SIE Second timer int. enable
  op.RestartReg(PCF8523_CONTROL_1, Operation::Type::WRITE);
  op.WriteByte(ctlreg | (1 << 2));
  return op.Execute();
}

bool PCF8523::disableSecondTimer() {
  // Leave compatible settings intact
  uint8_t ctlreg;
  if (!i2c_.ReadRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1, &ctlreg))
    return false;

  // SIE Second timer int. disable
  return i2c_.WriteRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1,
                            ctlreg & ~(1 << 2));
}

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
    auto op = i2c_.CreateReadOp(PCF8523_ADDRESS, PCF8523_CONTROL_2,
                                "enableCountdownTimer:read");
    if (!op.ready())
      return false;
    op.Read(&ctlreg, sizeof(ctlreg));

    op.RestartReg(PCF8523_CLKOUTCONTROL, Operation::Type::READ);
    op.Read(&clkreg, sizeof(clkreg));

    if (!op.Execute())
      return false;
  }

  auto op = i2c_.CreateWriteOp(PCF8523_ADDRESS, PCF8523_CONTROL_2,
                               "enableCountdownTimer:write");
  if (!op.ready())
    return false;

  // CTBIE Countdown Timer B Interrupt Enabled
  op.WriteByte(ctlreg |= 0x01);

  // Timer B source clock frequency, optionally int. low pulse width
  op.RestartReg(PCF8523_TIMER_B_FRCTL, Operation::Type::WRITE);
  op.WriteByte(lowPulseWidth << 4 | clkFreq);

  // Timer B value (number of source clock periods)
  op.RestartReg(PCF8523_TIMER_B_VALUE, Operation::Type::WRITE);
  op.WriteByte(numPeriods);

  // TBM Timer B pulse int. mode, CLKOUT (aka SQW) disabled, TBC start Timer B
  op.RestartReg(PCF8523_CLKOUTCONTROL, Operation::Type::WRITE);
  op.WriteByte(clkreg | 0x79);

  return op.Execute();
}

bool PCF8523::enableCountdownTimer(PCF8523TimerClockFreq clkFreq,
                                   uint8_t numPeriods) {
  return enableCountdownTimer(clkFreq, numPeriods, 0);
}

bool PCF8523::disableCountdownTimer() {
  uint8_t clkreg;
  if (!i2c_.ReadRegister(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, &clkreg))
    return false;
  return i2c_.WriteRegister(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL,
                            ~1 & clkreg);
}

bool PCF8523::deconfigureAllTimers() {
  disableSecondTimer();  // Surgically clears CONTROL_1

  auto op = i2c_.CreateWriteOp(PCF8523_ADDRESS, PCF8523_CONTROL_2,
                               "deconfigureAllTimers");
  if (!op.ready())
    return false;

  op.WriteByte(0);

  op.RestartReg(PCF8523_CLKOUTCONTROL, Operation::Type::WRITE);
  op.WriteByte(0);

  op.RestartReg(PCF8523_TIMER_B_FRCTL, Operation::Type::WRITE);
  op.WriteByte(0);

  op.RestartReg(PCF8523_TIMER_B_VALUE, Operation::Type::WRITE);
  op.WriteByte(0);

  return op.Execute();
}

bool PCF8523::calibrate(Pcf8523OffsetMode mode, int8_t offset) {
  uint8_t reg = (uint8_t)offset & 0x7F;
  reg |= mode;
  return i2c_.WriteRegister(PCF8523_ADDRESS, PCF8523_OFFSET, reg);
}

}  // namespace rtc
