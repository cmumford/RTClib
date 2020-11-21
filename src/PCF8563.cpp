/*!
  @section license License

  This is a fork of Adafruit's RTClib library.
*/

#include <rtc_pcf8563.h>

#include <rtc_i2c.h>
/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */
#include "RTC_util.h"

namespace rtc {

namespace {

// clang-format off
constexpr uint8_t PCF8563_I2C_ADDRESS = 0x51;

constexpr uint8_t REGISTER_CLKOUTCONTROL = 0x0D;
constexpr uint8_t REGISTER_CONTROL_1     = 0x00;
constexpr uint8_t REGISTER_CONTROL_2     = 0x01;
constexpr uint8_t REGISTER_VL_SECONDS    = 0x02;

// Datasheet section 8.7:
constexpr uint8_t kSquareWaveOff   = 0x0;
constexpr uint8_t kSquareWave1Hz   = 0b10000011;
constexpr uint8_t kSquareWave32Hz  = 0b10000010;
constexpr uint8_t kSquareWave1kHz  = 0b10000001;
constexpr uint8_t kSquareWave32kHz = 0b10000000;
constexpr uint8_t kSquareWaveMask  = 0b10000011;
// clang-format on

}  // namespace

PCF8563::PCF8563(std::unique_ptr<I2CMaster> i2c) : i2c_(std::move(i2c)) {}

bool PCF8563::begin() {
  return i2c_->Ping(PCF8563_I2C_ADDRESS);
}

bool PCF8563::lostPower() {
  uint8_t value;
  if (!i2c_->ReadRegister(PCF8563_I2C_ADDRESS, REGISTER_VL_SECONDS, &value))
    return false;
  return value >> 7;
}

bool PCF8563::adjust(const DateTime& dt) {
  auto op =
      i2c_->CreateWriteOp(PCF8563_I2C_ADDRESS, REGISTER_VL_SECONDS, "adjust");
  if (!op)
    return false;
  const uint8_t values[7] = {
      bin2bcd(dt.second()),
      bin2bcd(dt.minute()),
      bin2bcd(dt.hour()),
      bin2bcd(dt.day()),
      0x0,
      bin2bcd(dt.month()),
      bin2bcd(dt.year() - 2000),
  };
  op->Write(values, sizeof(values));
  return op->Execute();
}

bool PCF8563::now(DateTime* dt) {
  auto op = i2c_->CreateReadOp(PCF8563_I2C_ADDRESS, REGISTER_VL_SECONDS, "now");
  if (!op)
    return false;
  uint8_t values[7];
  op->Read(values, sizeof(values));
  if (!op->Execute())
    return false;

  const uint8_t ss = bcd2bin(values[0] & 0x7F);
  const uint8_t mm = bcd2bin(values[1] & 0x7F);
  const uint8_t hh = bcd2bin(values[2] & 0x3F);
  const uint8_t d = bcd2bin(values[3] & 0x3F);
  // skip 'weekdays'
  const uint8_t m = bcd2bin(values[5] & 0x1F);
  const uint16_t y = bcd2bin(values[6]) + 2000;

  *dt = DateTime(y, m, d, hh, mm, ss);
  return true;
}

bool PCF8563::start() {
  uint8_t ctlreg;
  if (!i2c_->ReadRegister(PCF8563_I2C_ADDRESS, REGISTER_CONTROL_1, &ctlreg))
    return false;

  return i2c_->WriteRegister(PCF8563_I2C_ADDRESS, REGISTER_CONTROL_1,
                             ctlreg & ~(1 << 5));
}

bool PCF8563::stop() {
  uint8_t ctlreg;
  if (!i2c_->ReadRegister(PCF8563_I2C_ADDRESS, REGISTER_CONTROL_1, &ctlreg))
    return false;

  return i2c_->WriteRegister(PCF8563_I2C_ADDRESS, REGISTER_CONTROL_1,
                             ctlreg | (1 << 5));
}

bool PCF8563::isrunning() {
  uint8_t ctlreg;
  if (!i2c_->ReadRegister(PCF8563_I2C_ADDRESS, REGISTER_CONTROL_1, &ctlreg))
    return false;
  return !((ctlreg >> 5) & 1);
}

PCF8563::SqwPinMode PCF8563::readSqwPinMode() {
  uint8_t mode;
  if (!i2c_->ReadRegister(PCF8563_I2C_ADDRESS, REGISTER_CLKOUTCONTROL, &mode))
    return PCF8563::SqwPinMode::Off;
  switch (mode & kSquareWaveMask) {
    case kSquareWaveOff:
      return SqwPinMode::Off;
    case kSquareWave1Hz:
      return SqwPinMode::Rate1Hz;
    case kSquareWave32Hz:
      return SqwPinMode::Rate32Hz;
    case kSquareWave1kHz:
      return SqwPinMode::Rate1kHz;
    case kSquareWave32kHz:
      return SqwPinMode::Rate32kHz;
    default:
      return SqwPinMode::Off;
  }
}

bool PCF8563::writeSqwPinMode(SqwPinMode mode) {
  uint8_t reg_value = kSquareWaveOff;
  switch (mode) {
    case SqwPinMode::Off:
      reg_value = kSquareWaveOff;
      break;
    case SqwPinMode::Rate1Hz:
      reg_value = kSquareWave1Hz;
      break;
    case SqwPinMode::Rate32Hz:
      reg_value = kSquareWave32Hz;
      break;
    case SqwPinMode::Rate1kHz:
      reg_value = kSquareWave1kHz;
      break;
    case SqwPinMode::Rate32kHz:
      reg_value = kSquareWave32kHz;
      break;
  }
  // Bits 6..2 are unused, setting to all zeros.
  return i2c_->WriteRegister(PCF8563_I2C_ADDRESS, REGISTER_CLKOUTCONTROL,
                             reg_value);
}

}  // namespace rtc
