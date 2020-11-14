/*!
  @section license License

  Original library by JeeLabs https://jeelabs.org/pub/docs/rtclib/, released to
  the public domain.

  This version: MIT (see LICENSE)
*/

#include <RTClib.h>

#include "RTC_util.h"

namespace {

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

RTC_DS3231::RTC_DS3231(RTC_I2C* i2c) : i2c_(i2c) {}

/**************************************************************************/
/*!
    @brief  Start I2C for the DS3231 and test succesful connection
    @return True if Wire can find DS3231 or false otherwise.
*/
/**************************************************************************/
bool RTC_DS3231::begin(void) {
  return i2c_->Ping(DS3231_ADDRESS);
}

/**************************************************************************/
/*!
    @brief  Check the status register Oscillator Stop Flag to see if the DS3231
   stopped due to power loss
    @return True if the bit is set (oscillator stopped) or false if it is
   running
*/
/**************************************************************************/
bool RTC_DS3231::lostPower(void) {
  uint8_t reg_val;
  if (!i2c_->ReadRegister(DS3231_ADDRESS, DS3231_STATUSREG, &reg_val))
    return true;  // Can't read, assume true.
  return reg_val >> 7;
}

/**************************************************************************/
/*!
    @brief  Set the date and flip the Oscillator Stop Flag
    @param dt DateTime object containing the date/time to set
*/
/**************************************************************************/
void RTC_DS3231::adjust(const DateTime& dt) {
  {
    auto write_op = i2c_->BeginWrite(DS3231_ADDRESS);
    if (!write_op)
      return;

    write_op->WriteByte((uint8_t)DS3231_TIME);  // start at location 0
    write_op->WriteByte(bin2bcd(dt.second()));
    write_op->WriteByte(bin2bcd(dt.minute()));
    write_op->WriteByte(bin2bcd(dt.hour()));
    // The RTC must know the day of the week for the weekly alarms to work.
    write_op->WriteByte(bin2bcd(dowToDS3231(dt.dayOfTheWeek())));
    write_op->WriteByte(bin2bcd(dt.day()));
    write_op->WriteByte(bin2bcd(dt.month()));
    write_op->WriteByte(bin2bcd(dt.year() - 2000U));
  }

  uint8_t status;
  if (!i2c_->ReadRegister(DS3231_ADDRESS, DS3231_STATUSREG, &status))
    return;
  status &= ~0x80;  // flip OSF bit
  i2c_->WriteRegister(DS3231_ADDRESS, DS3231_STATUSREG, status);
}

/**************************************************************************/
/*!
    @brief  Get the current date/time
    @return DateTime object with the current date/time
*/
/**************************************************************************/
DateTime RTC_DS3231::now() {
  {
    auto write_op = i2c_->BeginWrite(DS3231_ADDRESS);
    if (!write_op)
      return DateTime();

    write_op->WriteByte(0x0);
  }

  auto read_op = i2c_->BeginRead(DS3231_ADDRESS, 7);
  uint8_t value = 0;
  uint8_t ss = read_op->ReadByte(&value) ? bcd2bin(value & 0x7F) : 0;
  uint8_t mm = read_op->ReadByte(&value) ? bcd2bin(value) : 0;
  uint8_t hh = read_op->ReadByte(&value) ? bcd2bin(value) : 0;
  read_op->ReadByte(&value);  // Ignore this value.
  uint8_t d = read_op->ReadByte(&value) ? bcd2bin(value) : 0;
  uint8_t m = read_op->ReadByte(&value) ? bcd2bin(value) : 0;
  uint16_t y = read_op->ReadByte(&value) ? bcd2bin(value) + 2000 : 0;

  return DateTime(y, m, d, hh, mm, ss);
}

/**************************************************************************/
/*!
    @brief  Read the SQW pin mode
    @return Pin mode, see Ds3231SqwPinMode enum
*/
/**************************************************************************/
Ds3231SqwPinMode RTC_DS3231::readSqwPinMode() {
  {
    auto write_op = i2c_->BeginWrite(DS3231_ADDRESS);
    write_op->WriteByte(DS3231_CONTROL);
  }

  auto read_op = i2c_->BeginRead(DS3231_ADDRESS, 1);
  uint8_t mode = 0;
  read_op->ReadByte(&mode);

  mode &= 0x93;
  return static_cast<Ds3231SqwPinMode>(mode);
}

/**************************************************************************/
/*!
    @brief  Set the SQW pin mode
    @param mode Desired mode, see Ds3231SqwPinMode enum
*/
/**************************************************************************/
void RTC_DS3231::writeSqwPinMode(Ds3231SqwPinMode mode) {
  uint8_t ctrl = 0x0;
  i2c_->ReadRegister(DS3231_ADDRESS, DS3231_CONTROL, &ctrl);

  ctrl &= ~0x04;  // turn off INTCON
  ctrl &= ~0x18;  // set freq bits to 0

  if (mode == DS3231_OFF) {
    ctrl |= 0x04;  // turn on INTCN
  } else {
    ctrl |= mode;
  }
  i2c_->WriteRegister(DS3231_ADDRESS, DS3231_CONTROL, ctrl);

  // Serial.println( read_i2c_register(DS3231_ADDRESS, DS3231_CONTROL), HEX);
}

/**************************************************************************/
/*!
    @brief  Get the current temperature from the DS3231's temperature sensor
    @return Current temperature (float)
*/
/**************************************************************************/
float RTC_DS3231::getTemperature() {
  {
    auto write_op = i2c_->BeginWrite(DS3231_ADDRESS);
    write_op->WriteByte(DS3231_TEMPERATUREREG);
  }

  auto read_op = i2c_->BeginRead(DS3231_ADDRESS, 2);
  uint8_t msb = 0;
  read_op->ReadByte(&msb);
  int8_t lsb = 0;
  read_op->ReadByte(reinterpret_cast<uint8_t*>(&lsb));

  //  Serial.print("msb=");
  //  Serial.print(msb,HEX);
  //  Serial.print(", lsb=");
  //  Serial.println(lsb,HEX);

  return (float)msb + (lsb >> 6) * 0.25f;
}

/**************************************************************************/
/*!
    @brief  Set alarm 1 for DS3231
        @param 	dt DateTime object
        @param 	alarm_mode Desired mode, see Ds3231Alarm1Mode enum
    @return False if control register is not set, otherwise true
*/
/**************************************************************************/
bool RTC_DS3231::setAlarm1(const DateTime& dt, Ds3231Alarm1Mode alarm_mode) {
  uint8_t ctrl = 0;
  i2c_->ReadRegister(DS3231_ADDRESS, DS3231_CONTROL, &ctrl);
  if (!(ctrl & 0x04)) {
    return false;
  }

  uint8_t A1M1 = (alarm_mode & 0x01) << 7;  // Seconds bit 7.
  uint8_t A1M2 = (alarm_mode & 0x02) << 6;  // Minutes bit 7.
  uint8_t A1M3 = (alarm_mode & 0x04) << 5;  // Hour bit 7.
  uint8_t A1M4 = (alarm_mode & 0x08) << 4;  // Day/Date bit 7.
  uint8_t DY_DT = (alarm_mode & 0x10)
                  << 2;  // Day/Date bit 6. Date when 0, day of week when 1.

  {
    auto write_op = i2c_->BeginWrite(DS3231_ADDRESS);
    write_op->WriteByte(DS3231_ALARM1);
    write_op->WriteByte(bin2bcd(dt.second()) | A1M1);
    write_op->WriteByte(bin2bcd(dt.minute()) | A1M2);
    write_op->WriteByte(bin2bcd(dt.hour()) | A1M3);
    if (DY_DT) {
      write_op->WriteByte(bin2bcd(dowToDS3231(dt.dayOfTheWeek())) | A1M4 |
                          DY_DT);
    } else {
      write_op->WriteByte(bin2bcd(dt.day()) | A1M4 | DY_DT);
    }
  }

  ctrl |= 0x01;  // AI1E
  return i2c_->WriteRegister(DS3231_ADDRESS, DS3231_CONTROL, ctrl);
}

/**************************************************************************/
/*!
    @brief  Set alarm 2 for DS3231
        @param 	dt DateTime object
        @param 	alarm_mode Desired mode, see Ds3231Alarm2Mode enum
    @return False if control register is not set, otherwise true
*/
/**************************************************************************/
bool RTC_DS3231::setAlarm2(const DateTime& dt, Ds3231Alarm2Mode alarm_mode) {
  uint8_t ctrl = 0;
  i2c_->ReadRegister(DS3231_ADDRESS, DS3231_CONTROL, &ctrl);
  if (!(ctrl & 0x04)) {
    return false;
  }

  uint8_t A2M2 = (alarm_mode & 0x01) << 7;  // Minutes bit 7.
  uint8_t A2M3 = (alarm_mode & 0x02) << 6;  // Hour bit 7.
  uint8_t A2M4 = (alarm_mode & 0x04) << 5;  // Day/Date bit 7.
  uint8_t DY_DT = (alarm_mode & 0x8)
                  << 3;  // Day/Date bit 6. Date when 0, day of week when 1.

  {
    auto write_op = i2c_->BeginWrite(DS3231_ADDRESS);
    write_op->WriteByte(DS3231_ALARM2);
    write_op->WriteByte(bin2bcd(dt.minute()) | A2M2);
    write_op->WriteByte(bin2bcd(dt.hour()) | A2M3);
    if (DY_DT) {
      write_op->WriteByte(bin2bcd(dowToDS3231(dt.dayOfTheWeek())) | A2M4 |
                          DY_DT);
    } else {
      write_op->WriteByte(bin2bcd(dt.day()) | A2M4 | DY_DT);
    }
  }

  ctrl |= 0x02;  // AI2E
  return i2c_->WriteRegister(DS3231_ADDRESS, DS3231_CONTROL, ctrl);
}

/**************************************************************************/
/*!
    @brief  Disable alarm
        @param 	alarm_num Alarm number to disable
*/
/**************************************************************************/
void RTC_DS3231::disableAlarm(uint8_t alarm_num) {
  uint8_t ctrl = 0;
  i2c_->ReadRegister(DS3231_ADDRESS, DS3231_CONTROL, &ctrl);
  ctrl &= ~(1 << (alarm_num - 1));
  i2c_->WriteRegister(DS3231_ADDRESS, DS3231_CONTROL, ctrl);
}

/**************************************************************************/
/*!
    @brief  Clear status of alarm
        @param 	alarm_num Alarm number to clear
*/
/**************************************************************************/
void RTC_DS3231::clearAlarm(uint8_t alarm_num) {
  uint8_t status;
  if (!i2c_->ReadRegister(DS3231_ADDRESS, DS3231_STATUSREG, &status))
    return;
  status &= ~(0x1 << (alarm_num - 1));
  i2c_->WriteRegister(DS3231_ADDRESS, DS3231_STATUSREG, status);
}

/**************************************************************************/
/*!
    @brief  Get status of alarm
        @param 	alarm_num Alarm number to check status of
        @return True if alarm has been fired otherwise false
*/
/**************************************************************************/
bool RTC_DS3231::alarmFired(uint8_t alarm_num) {
  uint8_t status;
  if (!i2c_->ReadRegister(DS3231_ADDRESS, DS3231_STATUSREG, &status))
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
void RTC_DS3231::enable32K(void) {
  uint8_t status;
  if (!i2c_->ReadRegister(DS3231_ADDRESS, DS3231_STATUSREG, &status))
    return;
  status |= (0x1 << 0x03);
  i2c_->WriteRegister(DS3231_ADDRESS, DS3231_STATUSREG, status);
  // Serial.println(read_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG), BIN);
}

/**************************************************************************/
/*!
    @brief  Disable 32KHz Output
*/
/**************************************************************************/
void RTC_DS3231::disable32K(void) {
  uint8_t status;
  if (!i2c_->ReadRegister(DS3231_ADDRESS, DS3231_STATUSREG, &status))
    return;
  status &= ~(0x1 << 0x03);
  i2c_->WriteRegister(DS3231_ADDRESS, DS3231_STATUSREG, status);
  // Serial.println(read_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG), BIN);
}

/**************************************************************************/
/*!
    @brief  Get status of 32KHz Output
    @return True if enabled otherwise false
*/
/**************************************************************************/
bool RTC_DS3231::isEnabled32K(void) {
  uint8_t status;
  if (!i2c_->ReadRegister(DS3231_ADDRESS, DS3231_STATUSREG, &status))
    return false;
  return (status >> 0x03) & 0x1;
}