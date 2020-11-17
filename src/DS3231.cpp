/*!
  @section license License

  Original library by JeeLabs https://jeelabs.org/pub/docs/rtclib/, released to
  the public domain.

  This version: MIT (see LICENSE)
*/

#include <rtc_ds3231.h>

#include <limits>

#include <rtc_i2c.h>
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
constexpr uint8_t REGISTER_TEMP_MSB       = 0x11;
constexpr uint8_t REGISTER_TEMP_LSB       = 0x12;

constexpr uint8_t CONTROL_EOSC  = 0b10000000;
constexpr uint8_t CONTROL_BBSQW = 0b01000000;
constexpr uint8_t CONTROL_CONV  = 0b00100000;
constexpr uint8_t CONTROL_RS2   = 0b00010000;
constexpr uint8_t CONTROL_RS1   = 0b00001000;
constexpr uint8_t CONTROL_INTCN = 0b00000100;
constexpr uint8_t CONTROL_A2IE  = 0b00000010;
constexpr uint8_t CONTROL_A1IE  = 0b00000001;

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

/**************************************************************************/
/*!
    @brief  Convert the day of the week to a representation suitable for
            storing in the DS3231: from 1 (Monday) to 7 (Sunday).
    @param  d Day of the week as represented by the library:
            from 0 (Sunday) to 6 (Saturday).
*/
/**************************************************************************/
uint8_t dowToDS3231(uint8_t d) {
  return d == 0 ? 7 : d;
}

}  // anonymous namespace

DS3231::DS3231(std::unique_ptr<I2CMaster> i2c) : i2c_(std::move(i2c)) {}

/**************************************************************************/
/*!
    @brief  Start I2C for the DS3231 and test succesful connection
    @return True if Wire can find DS3231 or false otherwise.
*/
/**************************************************************************/
bool DS3231::begin(void) {
  return i2c_->Ping(DS3231_I2C_ADDRESS);
}

/**************************************************************************/
/*!
    @brief  Check the status register Oscillator Stop Flag to see if the DS3231
   stopped due to power loss
    @return True if the bit is set (oscillator stopped) or false if it is
   running
*/
/**************************************************************************/
bool DS3231::lostPower(void) {
  uint8_t reg_val;
  if (!i2c_->ReadRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, &reg_val))
    return true;  // Can't read, assume true.
  return reg_val >> 7;
}

/**************************************************************************/
/*!
    @brief  Set the date and flip the Oscillator Stop Flag
    @param dt DateTime object containing the date/time to set
*/
/**************************************************************************/
bool DS3231::adjust(const DateTime& dt) {
  {
    auto op = i2c_->CreateWriteOp(DS3231_I2C_ADDRESS, "adjust");
    if (!op)
      return false;
    op->WriteByte(REGISTER_TIME_SECONDS);  // First time register
    op->WriteByte(bin2bcd(dt.second()));
    op->WriteByte(bin2bcd(dt.minute()));
    op->WriteByte(bin2bcd(dt.hour()));
    // The RTC must know the day of the week for the weekly alarms to work.
    op->WriteByte(bin2bcd(dowToDS3231(dt.dayOfTheWeek())));
    op->WriteByte(bin2bcd(dt.day()));
    op->WriteByte(bin2bcd(dt.month()));
    op->WriteByte(bin2bcd(dt.year() - 2000U));
    if (!op->Execute())
      return false;
  }

  uint8_t status;
  if (!i2c_->ReadRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, &status))
    return false;
  status &= ~0x80;  // flip OSF bit
  return i2c_->WriteRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, status);
}

/**************************************************************************/
/*!
    @brief  Get the current date/time
    @return DateTime object with the current date/time
*/
/**************************************************************************/
DateTime DS3231::now() {
  uint8_t values[7];  // for registers 0x00 - 0x06.
  auto op = i2c_->CreateWriteOp(DS3231_I2C_ADDRESS, "now");
  op->WriteByte(REGISTER_TIME_SECONDS);  // First time register address.
  op->Restart(DS3231_I2C_ADDRESS, OperationType::READ);
  if (!op->Read(values, sizeof(values)))
    return DateTime();
  if (!op->Execute())
    return DateTime();

  const uint8_t ss = bcd2bin(values[REGISTER_TIME_SECONDS]);
  const uint8_t mm = bcd2bin(values[REGISTER_TIME_MINUTES]);
  const uint8_t hh = bcd2bin(values[REGISTER_TIME_HOURS]);
  // Skip day of week.
  const uint8_t d = bcd2bin(values[REGISTER_TIME_DATE]);
  const uint8_t m = bcd2bin(values[REGISTER_TIME_MONTH]);
  const uint16_t y = bcd2bin(values[REGISTER_TIME_YEAR]);

  return DateTime(y, m, d, hh, mm, ss);
}

/**************************************************************************/
/*!
    @brief  Read the SQW pin mode
    @return Pin mode, see Ds3231SqwPinMode enum
*/
/**************************************************************************/
DS3231::SqwPinMode DS3231::readSqwPinMode() {
  uint8_t value;
  if (!i2c_->ReadRegister(DS3231_I2C_ADDRESS, REGISTER_CONTROL, &value))
    return SqwPinMode::Alarm;

  if (value & CONTROL_INTCN)
    return SqwPinMode::Alarm;

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

/**************************************************************************/
/*!
    @brief  Set the SQW pin mode
    @param mode Desired mode, see Ds3231SqwPinMode enum
*/
/**************************************************************************/
bool DS3231::writeSqwPinMode(SqwPinMode mode) {
  uint8_t ctrl;
  if (!i2c_->ReadRegister(DS3231_I2C_ADDRESS, REGISTER_CONTROL, &ctrl))
    return false;

  CLEAR_BITS(ctrl, CONTROL_RS2 | CONTROL_RS1 | CONTROL_INTCN);
  switch (mode) {
    case SqwPinMode::Alarm:
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

  return i2c_->WriteRegister(DS3231_I2C_ADDRESS, REGISTER_CONTROL, ctrl);
}

/**************************************************************************/
/*!
    @brief  Get the current temperature from the DS3231's temperature sensor
    @return Current temperature (float)
*/
/**************************************************************************/
float DS3231::getTemperature() {
  auto op = i2c_->CreateWriteOp(DS3231_I2C_ADDRESS, "getTemp");
  if (!op)
    return std::numeric_limits<int16_t>::max();
  op->WriteByte(REGISTER_TEMP_MSB);
  op->Restart(DS3231_I2C_ADDRESS, OperationType::READ);
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

/**************************************************************************/
/*!
    @brief  Set alarm 1 for DS3231
        @param 	dt DateTime object
        @param 	alarm_mode Desired mode, see Ds3231Alarm1Mode enum
    @return False if control register is not set, otherwise true
*/
/**************************************************************************/
bool DS3231::setAlarm1(const DateTime& dt, Alarm1Mode alarm_mode) {
  uint8_t ctrl;
  i2c_->ReadRegister(DS3231_I2C_ADDRESS, REGISTER_CONTROL, &ctrl);
  if (!(ctrl & CONTROL_INTCN))
    return false;

  uint8_t values[4] = {0, 0, 0, 0};

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

  SET_BITS(values[0], bin2bcd(dt.second()) & A1M1_SECONDS);
  SET_BITS(values[1], bin2bcd(dt.minute()) & A1M2_MINUTES);
  SET_BITS(values[2], bin2bcd(dt.hour()) & A1M3_HOURS);
  if (alarm_mode == Alarm1Mode::Day)
    SET_BITS(values[3], bin2bcd(dt.dayOfTheWeek()) & A1M4_DATE);
  else
    SET_BITS(values[3], bin2bcd(dt.day()) & A1M4_DATE);

  auto op = i2c_->CreateWriteOp(DS3231_I2C_ADDRESS, "setalm1");
  op->WriteByte(REGISTER_ALARM1_SECONDS);
  op->Write(values, sizeof(values));

  op->Restart(DS3231_I2C_ADDRESS, OperationType::WRITE);
  op->WriteByte(REGISTER_CONTROL);
  SET_BITS(ctrl, CONTROL_A1IE);
  op->WriteByte(ctrl);

  return op->Execute();
}

/**************************************************************************/
/*!
    @brief  Set alarm 2 for DS3231
        @param 	dt DateTime object
        @param 	alarm_mode Desired mode, see Ds3231Alarm2Mode enum
    @return False if control register is not set, otherwise true
*/
/**************************************************************************/
bool DS3231::setAlarm2(const DateTime& dt, Alarm2Mode alarm_mode) {
  uint8_t ctrl;
  i2c_->ReadRegister(DS3231_I2C_ADDRESS, REGISTER_CONTROL, &ctrl);
  if (!(ctrl & CONTROL_INTCN))
    return false;

  uint8_t values[3] = {0, 0, 0};

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

  SET_BITS(values[0], bin2bcd(dt.second()) & A2M2_MINUTES);
  SET_BITS(values[1], bin2bcd(dt.minute()) & A2M3_HOURS);
  SET_BITS(values[2], bin2bcd(dt.hour()) & A2M4_DATE);
  if (alarm_mode == Alarm2Mode::Day)
    SET_BITS(values[3], bin2bcd(dt.dayOfTheWeek()) & A1M4_DATE);
  else
    SET_BITS(values[3], bin2bcd(dt.day()) & A1M4_DATE);

  auto op = i2c_->CreateWriteOp(DS3231_I2C_ADDRESS, "setalm2");
  op->WriteByte(REGISTER_ALARM2_MINUTES);
  op->Write(values, sizeof(values));

  op->Restart(DS3231_I2C_ADDRESS, OperationType::WRITE);
  op->WriteByte(REGISTER_CONTROL);
  SET_BITS(ctrl, CONTROL_A2IE);
  op->WriteByte(ctrl);

  return op->Execute();
}

/**************************************************************************/
/*!
    @brief  Disable alarm
        @param 	alarm_num Alarm number to disable
*/
/**************************************************************************/
void DS3231::disableAlarm(uint8_t alarm_num) {
  uint8_t ctrl = 0;
  i2c_->ReadRegister(DS3231_I2C_ADDRESS, REGISTER_CONTROL, &ctrl);
  ctrl &= ~(1 << (alarm_num - 1));
  i2c_->WriteRegister(DS3231_I2C_ADDRESS, REGISTER_CONTROL, ctrl);
}

/**************************************************************************/
/*!
    @brief  Clear status of alarm
        @param 	alarm_num Alarm number to clear
*/
/**************************************************************************/
void DS3231::clearAlarm(uint8_t alarm_num) {
  uint8_t status;
  if (!i2c_->ReadRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, &status))
    return;
  status &= ~(0x1 << (alarm_num - 1));
  i2c_->WriteRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, status);
}

/**************************************************************************/
/*!
    @brief  Get status of alarm
        @param 	alarm_num Alarm number to check status of
        @return True if alarm has been fired otherwise false
*/
/**************************************************************************/
bool DS3231::alarmFired(uint8_t alarm_num) {
  uint8_t status;
  if (!i2c_->ReadRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, &status))
    return false;
  return (status >> (alarm_num - 1)) & 0x1;
}

/**************************************************************************/
/*!
    @brief  Enable 32KHz Output
    @details The 32kHz output is enabled by default. It requires an external
    pull-up resistor to function correctly
*/
/**************************************************************************/
void DS3231::enable32K(void) {
  uint8_t status;
  if (!i2c_->ReadRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, &status))
    return;
  status |= (0x1 << 0x03);
  i2c_->WriteRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, status);
}

/**************************************************************************/
/*!
    @brief  Disable 32KHz Output
*/
/**************************************************************************/
void DS3231::disable32K(void) {
  uint8_t status;
  if (!i2c_->ReadRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, &status))
    return;
  status &= ~(0x1 << 0x03);
  i2c_->WriteRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, status);
}

/**************************************************************************/
/*!
    @brief  Get status of 32KHz Output
    @return True if enabled otherwise false
*/
/**************************************************************************/
bool DS3231::isEnabled32K(void) {
  uint8_t status;
  if (!i2c_->ReadRegister(DS3231_I2C_ADDRESS, REGISTER_STATUS, &status))
    return false;
  return (status >> 0x03) & 0x1;
}

}  // namespace rtc
