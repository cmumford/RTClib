/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */

#include <rtclib/ds1307.h>

#include <i2clib/master.h>
#include <i2clib/operation.h>
#include <rtclib/datetime.h>
#include "rtc_util.h"

namespace rtc {

namespace {

// clang-format off
constexpr uint8_t DS1307_ADDRESS = 0x68;  ///< I2C address for DS1307

constexpr uint8_t REGISTER_TIME_SECONDS = 0x00;
constexpr uint8_t REGISTER_TIME_MINUTES = 0x01;
constexpr uint8_t REGISTER_TIME_HOURS   = 0x02;
constexpr uint8_t REGISTER_TIME_DAY     = 0x03;
constexpr uint8_t REGISTER_TIME_DATE    = 0x04;
constexpr uint8_t REGISTER_TIME_MONTH   = 0x05;
constexpr uint8_t REGISTER_TIME_YEAR    = 0x06;
constexpr uint8_t REGISTER_CONTROL      = 0x07;
constexpr uint8_t REGISTER_NVRAM        = 0x08; // NVRAM: 56 bytes, 0x08..0x3f.

/**
 * @brief controls the output level of the SQW/OUT pin when the square-wave
 *        output is disabled.
 * 
 * If SQWE = 0, the logic level on the SQW/OUT pin is 1 if OUT = 1 
 * and is 0 if OUT = 0.
 */
constexpr uint8_t CONTROL_OUT      = 0b10000000;
constexpr uint8_t CONTROL_RESERVED = 0b01101100; // Unused register bits
constexpr uint8_t CONTROL_SQWE     = 0b00010000;
constexpr uint8_t CONTROL_RS1      = 0b00000010;
constexpr uint8_t CONTROL_RS0      = 0b00000001;

constexpr uint8_t CONTROL_SQW_OFF  = 0x0;
constexpr uint8_t CONTROL_SQW_ON   = CONTROL_OUT;
constexpr uint8_t CONTROL_SQW_1KH  = CONTROL_SQWE;
constexpr uint8_t CONTROL_SQW_4KH  = CONTROL_SQWE | CONTROL_RS0;
constexpr uint8_t CONTROL_SQW_8KH  = CONTROL_SQWE | CONTROL_RS1;
constexpr uint8_t CONTROL_SQW_32KH = CONTROL_SQWE | CONTROL_RS0 | CONTROL_RS1;

// clang-format on

}  // namespace

DS1307::DS1307(i2c::Master i2c) : i2c_(std::move(i2c)) {}

bool DS1307::begin(void) {
  return i2c_.Ping(DS1307_ADDRESS);
}

bool DS1307::isRunning(void) {
  uint8_t value;
  if (!i2c_.ReadRegister(DS1307_ADDRESS, REGISTER_TIME_SECONDS, &value))
    return false;
  return !(value >> 7);
}

bool DS1307::adjust(const DateTime& dt) {
  auto op = i2c_.CreateWriteOp(DS1307_ADDRESS, REGISTER_TIME_SECONDS, "adjust");
  if (!op.ready())
    return false;
  const uint8_t values[7] = {
      bin2bcd(dt.second()),
      bin2bcd(dt.minute()),
      bin2bcd(dt.hour()),
      0x0,  // Day of week
      bin2bcd(dt.day()),
      bin2bcd(dt.month()),
      bin2bcd(dt.year() - 2000U),
  };

  op.Write(values, sizeof(values));

  return op.Execute();
}

bool DS1307::now(DateTime* dt) {
  auto op = i2c_.CreateReadOp(DS1307_ADDRESS, REGISTER_TIME_SECONDS, "now");
  if (!op.ready())
    return false;
  uint8_t values[7];  // for registers 0x00 - 0x06.
  if (!op.Read(values, sizeof(values)))
    return false;
  if (!op.Execute())
    return false;

  const uint8_t ss = bcd2bin(values[REGISTER_TIME_SECONDS]);
  const uint8_t mm = bcd2bin(values[REGISTER_TIME_MINUTES]);
  const uint8_t hh = bcd2bin(values[REGISTER_TIME_HOURS]);
  // Skip day of week.
  const uint8_t d = bcd2bin(values[REGISTER_TIME_DATE]);
  const uint8_t m = bcd2bin(values[REGISTER_TIME_MONTH]);
  const uint16_t y = 2000 + bcd2bin(values[REGISTER_TIME_YEAR]);

  *dt = DateTime(y, m, d, hh, mm, ss);
  return true;
}

DS1307::SqwPinMode DS1307::readSqwPinMode() {
  uint8_t value;
  if (!i2c_.ReadRegister(DS1307_ADDRESS, REGISTER_CONTROL, &value))
    return DS1307::SqwPinMode::Off;

  if (value & CONTROL_SQWE) {
    switch (value & (CONTROL_RS0 | CONTROL_RS1)) {
      case 0x0:
        return SqwPinMode::Rate1Hz;
      case CONTROL_RS0:
        return SqwPinMode::Rate4kHz;
      case CONTROL_RS1:
        return SqwPinMode::Rate8kHz;
      case CONTROL_RS1 | CONTROL_RS0:
        return SqwPinMode::Rate32kHz;
      default:
        // unreachable
        return SqwPinMode::Off;
    }
  } else {
    return value & CONTROL_OUT ? SqwPinMode::On : SqwPinMode::Off;
  }
}

bool DS1307::writeSqwPinMode(SqwPinMode mode) {
  uint8_t reg_value = 0x0;
  switch (mode) {
    case SqwPinMode::Off:
      reg_value = CONTROL_SQW_OFF;
      break;
    case SqwPinMode::On:
      reg_value = CONTROL_SQW_ON;
      break;
    case SqwPinMode::Rate1Hz:
      reg_value = CONTROL_SQW_1KH;
      break;
    case SqwPinMode::Rate4kHz:
      reg_value = CONTROL_SQW_4KH;
      break;
    case SqwPinMode::Rate8kHz:
      reg_value = CONTROL_SQW_8KH;
      break;
    case SqwPinMode::Rate32kHz:
      reg_value = CONTROL_SQW_32KH;
      break;
  }
  return i2c_.WriteRegister(DS1307_ADDRESS, REGISTER_CONTROL, reg_value);
}

bool DS1307::readnvram(uint8_t address, void* buf, size_t num_bytes) {
  auto op =
      i2c_.CreateReadOp(DS1307_ADDRESS, REGISTER_NVRAM + address, "readnvram");
  if (!op.ready())
    return false;
  if (!op.Read(buf, num_bytes))
    return false;
  return op.Execute();
}

bool DS1307::writeNVRAM(uint8_t address, const void* buf, size_t num_bytes) {
  auto op = i2c_.CreateWriteOp(DS1307_ADDRESS, REGISTER_NVRAM + address,
                               "writeNVRAM");
  if (!op.ready())
    return false;
  if (!op.Write(buf, num_bytes))
    return false;
  return op.Execute();
}

}  // namespace rtc
