/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */

#include <pcf8523.h>

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

bool PCF8523::begin(void) {
  return i2c_->Ping(PCF8523_ADDRESS);
}

bool PCF8523::lostPower(void) {
  uint8_t value;
  if (!i2c_->ReadRegister(PCF8523_ADDRESS, PCF8523_STATUSREG, &value))
    return false;
  return value >> 7;
}

bool PCF8523::initialized(void) {
  uint8_t value;
  if (!i2c_->ReadRegister(PCF8523_ADDRESS, PCF8523_CONTROL_3, &value))
    return false;
  return ((value & 0xE0) != 0xE0);  // 0xE0 = standby mode, set after power out
}

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
  op->Restart(PCF8523_ADDRESS, PCF8523_CONTROL_3, OperationType::WRITE);
  op->WriteByte(0x0);
  return op->Execute();
}

bool PCF8523::now(DateTime* dt) {
  auto op = i2c_->CreateReadOp(PCF8523_ADDRESS, 0x3, "now");
  if (!op)
    return false;
  uint8_t values[7];  // for registers 0x00 - 0x06.
  if (!op->Read(values, sizeof(values)))
    return false;
  if (!op->Execute())
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
  if (!i2c_->ReadRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1, &ctlreg))
    return false;
  if (ctlreg & (1 << 5)) {
    return i2c_->WriteRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1,
                               ctlreg & ~(1 << 5));
  }
  return true;
}

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

bool PCF8523::isRunning() {
  uint8_t ctlreg;
  if (!i2c_->ReadRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1, &ctlreg))
    return false;

  return !((ctlreg >> 5) & 1);
}

Pcf8523SqwPinMode PCF8523::readSqwPinMode() {
  uint8_t mode;
  if (!i2c_->ReadRegister(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, &mode))
    return PCF8523_OFF;

  mode >>= 3;
  mode &= 0x7;

  return static_cast<Pcf8523SqwPinMode>(mode);
}

bool PCF8523::writeSqwPinMode(Pcf8523SqwPinMode mode) {
  return i2c_->WriteRegister(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, mode << 3);
}

bool PCF8523::enableSecondTimer() {
  uint8_t ctlreg;
  uint8_t clkreg;

  {
    auto op = i2c_->CreateReadOp(PCF8523_ADDRESS, PCF8523_CONTROL_1,
                                 "enableSecondTimer:read");
    if (!op)
      return false;
    op->Read(&ctlreg, sizeof(ctlreg));
    op->Restart(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, OperationType::READ);
    op->Read(&clkreg, sizeof(clkreg));
    if (!op->Execute())
      return false;
  }

  auto op = i2c_->CreateWriteOp(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL,
                                "enableSecondTimer:write");
  if (!op)
    return false;
  // TAM pulse int. mode (shared with Timer A), CLKOUT (aka SQW) disabled
  op->WriteByte(clkreg | 0xB8);

  // SIE Second timer int. enable
  op->Restart(PCF8523_ADDRESS, PCF8523_CONTROL_1, OperationType::WRITE);
  op->WriteByte(ctlreg | (1 << 2));
  return op->Execute();
}

bool PCF8523::disableSecondTimer() {
  // Leave compatible settings intact
  uint8_t ctlreg;
  if (!i2c_->ReadRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1, &ctlreg))
    return false;

  // SIE Second timer int. disable
  return i2c_->WriteRegister(PCF8523_ADDRESS, PCF8523_CONTROL_1,
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
    auto op = i2c_->CreateReadOp(PCF8523_ADDRESS, PCF8523_CONTROL_2,
                                 "enableCountdownTimer:read");
    if (!op)
      return false;
    op->Read(&ctlreg, sizeof(ctlreg));

    op->Restart(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, OperationType::READ);
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
  op->Restart(PCF8523_ADDRESS, PCF8523_TIMER_B_FRCTL, OperationType::WRITE);
  op->WriteByte(lowPulseWidth << 4 | clkFreq);

  // Timer B value (number of source clock periods)
  op->Restart(PCF8523_ADDRESS, PCF8523_TIMER_B_VALUE, OperationType::WRITE);
  op->WriteByte(numPeriods);

  // TBM Timer B pulse int. mode, CLKOUT (aka SQW) disabled, TBC start Timer B
  op->Restart(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, OperationType::WRITE);
  op->WriteByte(clkreg | 0x79);

  return op->Execute();
}

bool PCF8523::enableCountdownTimer(PCF8523TimerClockFreq clkFreq,
                                   uint8_t numPeriods) {
  return enableCountdownTimer(clkFreq, numPeriods, 0);
}

bool PCF8523::disableCountdownTimer() {
  uint8_t clkreg;
  if (!i2c_->ReadRegister(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, &clkreg))
    return false;
  return i2c_->WriteRegister(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL,
                             ~1 & clkreg);
}

bool PCF8523::deconfigureAllTimers() {
  disableSecondTimer();  // Surgically clears CONTROL_1

  auto op = i2c_->CreateWriteOp(PCF8523_ADDRESS, PCF8523_CONTROL_2,
                                "deconfigureAllTimers");
  if (!op)
    return false;

  op->WriteByte(0);

  op->Restart(PCF8523_ADDRESS, PCF8523_CLKOUTCONTROL, OperationType::WRITE);
  op->WriteByte(0);

  op->Restart(PCF8523_ADDRESS, PCF8523_TIMER_B_FRCTL, OperationType::WRITE);
  op->WriteByte(0);

  op->Restart(PCF8523_ADDRESS, PCF8523_TIMER_B_VALUE, OperationType::WRITE);
  op->WriteByte(0);

  return op->Execute();
}

bool PCF8523::calibrate(Pcf8523OffsetMode mode, int8_t offset) {
  uint8_t reg = (uint8_t)offset & 0x7F;
  reg |= mode;
  return i2c_->WriteRegister(PCF8523_ADDRESS, PCF8523_OFFSET, reg);
}

}  // namespace rtc
