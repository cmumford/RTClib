/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */

#include <rtclib/ds3231.h>

#include <limits>

#include <i2clib/master.h>
#include <i2clib/operation.h>
#include <rtclib/datetime.h>
#include "RTC_util.h"

namespace rtc {

namespace {

// clang-format off

constexpr uint8_t DS3231_I2C_ADDRESS = 0x68;

constexpr uint8_t REGISTER_TIME_SECONDS   = 0x00;
constexpr uint8_t REGISTER_TIME_MINUTES   = 0x01;
constexpr uint8_t REGISTER_TIME_HOURS     = 0x02;
constexpr uint8_t REGISTER_TIME_DAY       = 0x03;
constexpr uint8_t REGISTER_TIME_DATE      = 0x04;
constexpr uint8_t REGISTER_TIME_MONTH     = 0x05;
constexpr uint8_t REGISTER_TIME_YEAR      = 0x06;
constexpr uint8_t REGISTER_ALARM1_SECONDS = 0x07;
constexpr uint8_t REGISTER_ALARM2_MINUTES = 0x0B;
constexpr uint8_t REGISTER_CONTROL        = 0x0E;
constexpr uint8_t REGISTER_STATUS         = 0x0F;
constexpr uint8_t REGISTER_AGING_OFFSET   = 0x10;
constexpr uint8_t REGISTER_TEMP_MSB       = 0x11;
constexpr uint8_t REGISTER_TEMP_LSB       = 0x12;

constexpr uint8_t CONTROL_EOSC  = 0b10000000; // Enable oscillator.
constexpr uint8_t CONTROL_BBSQW = 0b01000000; // Battery backed square wave.
constexpr uint8_t CONTROL_CONV  = 0b00100000; // Convert temperature.
constexpr uint8_t CONTROL_RS2   = 0b00010000; // Rate select bit 2.
constexpr uint8_t CONTROL_RS1   = 0b00001000; // Rate select bit 1.
constexpr uint8_t CONTROL_INTCN = 0b00000100; // Interrupt control.
constexpr uint8_t CONTROL_A2IE  = 0b00000010; // Alarm 2 interrupt enable.
constexpr uint8_t CONTROL_A1IE  = 0b00000001; // Alarm 1 interrupt enable.

constexpr uint8_t STATUS_OSF     = 0b10000000; // Oscillator stop flag.
constexpr uint8_t STATUS_UNUSED  = 0b01110000; // Unused register bits.
constexpr uint8_t STATUS_EN32kHz = 0b00001000; // Enable 32kHz output.
constexpr uint8_t STATUS_BSY     = 0b00000100; // Busy.
constexpr uint8_t STATUS_A2F     = 0b00000010; // Alarm 2 flag.
constexpr uint8_t STATUS_A1F     = 0b00000001; // Alarm 1 flag.

constexpr uint8_t A1M1_ENABLE  = 0b10000000;
constexpr uint8_t A1M1_SECONDS = 0b01111111;
constexpr uint8_t A1M2_ENABLE  = 0b10000000;
constexpr uint8_t A1M2_MINUTES = 0b01111111;
constexpr uint8_t A1M3_ENABLE  = 0b10000000;
constexpr uint8_t A1M3_12_24   = 0b01000000;
constexpr uint8_t A1M3_AM_PM   = 0b00100000;
constexpr uint8_t A1M3_HOURS   = 0b00011111;
constexpr uint8_t A1M4_ENABLE  = 0b10000000;
constexpr uint8_t A1M4_DY_DT   = 0b01000000;
constexpr uint8_t A1M4_DATE    = 0b00111111;

constexpr uint8_t A2M2_ENABLE  = 0b10000000;
constexpr uint8_t A2M2_MINUTES = 0b01111111;
constexpr uint8_t A2M3_ENABLE  = 0b10000000;
constexpr uint8_t A2M3_12_24   = 0b01000000;
constexpr uint8_t A2M3_AM_PM   = 0b00100000;
constexpr uint8_t A2M3_HOURS   = 0b00011111;
constexpr uint8_t A2M4_ENABLE  = 0b10000000;
constexpr uint8_t A2M4_DY_DT   = 0b01000000;
constexpr uint8_t A2M4_DATE    = 0b00111111;

// clang-format on

constexpr uint8_t kSquareWave1Hz = 0x0;
constexpr uint8_t kSquareWave1kHz = CONTROL_RS1;
constexpr uint8_t kSquareWave4kHz = CONTROL_RS2;
constexpr uint8_t kSquareWave8kHz = CONTROL_RS2 | CONTROL_RS1;

/**
 * Convert the day of the week to a representation suitable for
 * storing in the DS3231: from 1 (Monday) to 7 (Sunday).
 *
 * @param  d Day of the week as represented by the library:
 *           from 0 (Sunday) to 6 (Saturday).
 */
uint8_t dowToDS3231(uint8_t d) {
  return d == 0 ? 7 : d;
}

}  // anonymous namespace

DS3231::DS3231(i2c::Master i2c) : i2c_(std::move(i2c)) {}

bool DS3231::begin(void) {
  return i2c_.Ping(DS3231_I2C_ADDRESS);
}

bool DS3231::lostPower(void) {
  uint8_t reg_val;
  if (!i2c_.ReadRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, &reg_val))
    return true;  // Can't read, assume true.
  return reg_val & STATUS_OSF;
}

bool DS3231::adjust(const DateTime& dt) {
  {
    auto op =
        i2c_.CreateWriteOp(DS3231_I2C_ADDRESS, REGISTER_TIME_SECONDS, "adjust");
    if (!op)
      return false;
    const uint8_t values[7] = {
        bin2bcd(dt.second()),       bin2bcd(dt.minute()),
        bin2bcd(dt.hour()),         bin2bcd(dowToDS3231(dt.dayOfTheWeek())),
        bin2bcd(dt.day()),          bin2bcd(dt.month()),
        bin2bcd(dt.year() - 2000U),
    };

    op->Write(values, sizeof(values));
    if (!op->Execute())
      return false;
  }

  uint8_t status;
  if (!i2c_.ReadRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, &status))
    return false;
  status &= ~STATUS_OSF;  // flip OSF bit
  return i2c_.WriteRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, status);
}

bool DS3231::now(DateTime* dt) {
  uint8_t values[7];  // for registers 0x00 - 0x06.
  auto op = i2c_.CreateReadOp(DS3231_I2C_ADDRESS, REGISTER_TIME_SECONDS, "now");
  if (!op)
    return false;
  if (!op->Read(values, sizeof(values)))
    return false;
  if (!op->Execute())
    return false;

  // BUG: Correctly handle the DY/DT flag. This assumes always date.
  *dt = DateTime(2000U + bcd2bin(values[REGISTER_TIME_YEAR]),
                 bcd2bin(values[REGISTER_TIME_MONTH]),
                 bcd2bin(values[REGISTER_TIME_DATE]),
                 bcd2bin(values[REGISTER_TIME_HOURS]),
                 bcd2bin(values[REGISTER_TIME_MINUTES]),
                 bcd2bin(values[REGISTER_TIME_SECONDS]));
  return true;
}

DS3231::SqwPinMode DS3231::readSqwPinMode() {
  uint8_t value;
  if (!i2c_.ReadRegister(DS3231_I2C_ADDRESS, REGISTER_CONTROL, &value))
    return SqwPinMode::Off;

  if (value & CONTROL_INTCN)
    return SqwPinMode::Off;

  switch (value & (CONTROL_RS2 | CONTROL_RS1)) {
    case kSquareWave1Hz:
      return SqwPinMode::Rate1Hz;
    case kSquareWave1kHz:
      return SqwPinMode::Rate1kHz;
    case kSquareWave4kHz:
      return SqwPinMode::Rate4kHz;
    case kSquareWave8kHz:
      return SqwPinMode::Rate8kHz;
  }

  return static_cast<SqwPinMode>(value &
                                 (CONTROL_RS2 | CONTROL_RS1 | CONTROL_INTCN));
}

bool DS3231::writeSqwPinMode(SqwPinMode mode) {
  uint8_t ctrl;
  if (!i2c_.ReadRegister(DS3231_I2C_ADDRESS, REGISTER_CONTROL, &ctrl))
    return false;

  CLEAR_BITS(ctrl, CONTROL_RS2 | CONTROL_RS1 | CONTROL_INTCN);
  switch (mode) {
    case SqwPinMode::Off:
      SET_BITS(ctrl, CONTROL_INTCN);
      break;
    case SqwPinMode::Rate1Hz:
      SET_BITS(ctrl, kSquareWave1Hz);
      break;
    case SqwPinMode::Rate1kHz:
      SET_BITS(ctrl, kSquareWave1kHz);
      break;
    case SqwPinMode::Rate4kHz:
      SET_BITS(ctrl, kSquareWave4kHz);
      break;
    case SqwPinMode::Rate8kHz:
      SET_BITS(ctrl, kSquareWave8kHz);
      break;
  }

  return i2c_.WriteRegister(DS3231_I2C_ADDRESS, REGISTER_CONTROL, ctrl);
}

float DS3231::getTemperature() {
  auto op = i2c_.CreateReadOp(DS3231_I2C_ADDRESS, REGISTER_TEMP_MSB, "getTemp");
  if (!op)
    return std::numeric_limits<int16_t>::max();
  uint8_t values[2];  // MSB and LSB respectively.
  op->Read(&values, sizeof(values));
  if (!op->Execute())
    return std::numeric_limits<int16_t>::max();
  // Combine the 10-bit signed msb+lsb into a single floating point number
  // with 0.25°C accuracy. See DSD3231 spec pg. 15.
  // Multiply/divide by four as left-shifting a signed integer is undefined
  // according to the C++ spec.
  const int16_t msb = static_cast<int16_t>(values[0]);
  const uint8_t lsb = (values[1] >> 6);
  return static_cast<float>(msb * 4 + lsb) * 0.25f;
}

bool DS3231::getAgingOffset(int8_t* val) {
  return i2c_.ReadRegister(DS3231_I2C_ADDRESS, REGISTER_AGING_OFFSET,
                           reinterpret_cast<uint8_t*>(val));
}

bool DS3231::setAlarm1(const DateTime& dt, Alarm1Mode alarm_mode) {
  uint8_t ctrl;
  i2c_.ReadRegister(DS3231_I2C_ADDRESS, REGISTER_CONTROL, &ctrl);
  if (!(ctrl & CONTROL_INTCN))
    return false;

  uint8_t values[4] = {
      bin2bcd(dt.second()), bin2bcd(dt.minute()), bin2bcd(dt.hour()),
      bin2bcd(alarm_mode == Alarm1Mode::Day ? dt.dayOfTheWeek() : dt.day())};

  // See table 2 in datasheet.
  switch (alarm_mode) {
    case Alarm1Mode::EverySecond:
      SET_BITS(values[0], A1M1_ENABLE);
      // fallthrough.
    case Alarm1Mode::Second:
      SET_BITS(values[1], A1M2_ENABLE);
      // fallthrough.
    case Alarm1Mode::Minute:
      SET_BITS(values[2], A1M3_ENABLE);
      // fallthrough.
    case Alarm1Mode::Hour:
      SET_BITS(values[3], A1M4_ENABLE);
      break;
    case Alarm1Mode::Date:
      // Do nothing. All bits should be clear.
      break;
    case Alarm1Mode::Day:
      SET_BITS(values[3], A1M4_DY_DT);
      break;
  }

  auto op = i2c_.CreateWriteOp(DS3231_I2C_ADDRESS, REGISTER_ALARM1_SECONDS,
                               "setalm1");
  if (!op)
    return false;
  op->Write(values, sizeof(values));

  op->Restart(DS3231_I2C_ADDRESS, REGISTER_CONTROL, i2c::OperationType::WRITE);
  SET_BITS(ctrl, CONTROL_A1IE);
  op->WriteByte(ctrl);

  return op->Execute();
}

bool DS3231::setAlarm2(const DateTime& dt, Alarm2Mode alarm_mode) {
  uint8_t ctrl;
  i2c_.ReadRegister(DS3231_I2C_ADDRESS, REGISTER_CONTROL, &ctrl);
  if (!(ctrl & CONTROL_INTCN))
    return false;

  uint8_t values[3] = {
      bin2bcd(dt.minute()), bin2bcd(dt.hour()),
      bin2bcd(alarm_mode == Alarm2Mode::Day ? dt.dayOfTheWeek() : dt.day())};

  switch (alarm_mode) {
    case Alarm2Mode::EveryMinute:
      SET_BITS(values[0], A2M2_ENABLE);
      // fallthrough.
    case Alarm2Mode::Minute:
      SET_BITS(values[1], A2M3_ENABLE);
      // fallthrough.
    case Alarm2Mode::Hour:
      SET_BITS(values[2], A2M4_ENABLE);
      break;
    case Alarm2Mode::Date:
      // Do nothing. All bits should be clear.
      break;
    case Alarm2Mode::Day:
      SET_BITS(values[2], A2M4_DY_DT);
      break;
  }

  auto op = i2c_.CreateWriteOp(DS3231_I2C_ADDRESS, REGISTER_ALARM2_MINUTES,
                               "setalm2");
  if (!op)
    return false;
  op->Write(values, sizeof(values));

  op->Restart(DS3231_I2C_ADDRESS, REGISTER_CONTROL, i2c::OperationType::WRITE);
  SET_BITS(ctrl, CONTROL_A2IE);
  op->WriteByte(ctrl);

  return op->Execute();
}

void DS3231::disableAlarm(Alarm alarm) {
  uint8_t ctrl = 0;
  i2c_.ReadRegister(DS3231_I2C_ADDRESS, REGISTER_CONTROL, &ctrl);
  if (alarm == Alarm::A1)
    CLEAR_BITS(ctrl, CONTROL_A1IE);
  else
    CLEAR_BITS(ctrl, CONTROL_A2IE);
  i2c_.WriteRegister(DS3231_I2C_ADDRESS, REGISTER_CONTROL, ctrl);
}

void DS3231::clearAlarm(Alarm alarm) {
  uint8_t status;
  if (!i2c_.ReadRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, &status))
    return;
  if (alarm == Alarm::A1)
    CLEAR_BITS(status, STATUS_A1F);
  else
    CLEAR_BITS(status, STATUS_A2F);
  i2c_.WriteRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, status);
}

bool DS3231::isAlarmFired(Alarm alarm) {
  uint8_t status;
  if (!i2c_.ReadRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, &status))
    return false;
  return alarm == Alarm::A1 ? status & STATUS_A1F : status & STATUS_A2F;
}

void DS3231::enable32K(void) {
  uint8_t status;
  if (!i2c_.ReadRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, &status))
    return;
  SET_BITS(status, STATUS_EN32kHz);
  i2c_.WriteRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, status);
}

void DS3231::disable32K(void) {
  uint8_t status;
  if (!i2c_.ReadRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, &status))
    return;
  CLEAR_BITS(status, STATUS_EN32kHz);
  i2c_.WriteRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, status);
}

bool DS3231::isEnabled32K(void) {
  uint8_t status;
  if (!i2c_.ReadRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, &status))
    return false;
  return status & STATUS_EN32kHz;
}

}  // namespace rtc
