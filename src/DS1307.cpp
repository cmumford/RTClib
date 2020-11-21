/*!
  @section license License

  Original library by JeeLabs https://jeelabs.org/pub/docs/rtclib/, released to
  the public domain.

  This version: MIT (see LICENSE)
*/

#include <rtc_ds1307.h>

#include <rtc_i2c.h>
#include "RTC_util.h"

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

constexpr uint8_t CONTROL_OUT      = 0b10000000;
constexpr uint8_t CONTROL_RESERVED = 0b01101100; // Unused register bits
constexpr uint8_t CONTROL_SQ_WAVE  = 0b00010000;
constexpr uint8_t CONTROL_RS1      = 0b00000010;
constexpr uint8_t CONTROL_RS0      = 0b00000001;

// clang-format on

}  // namespace

/**************************************************************************/
/*!
    @brief  Start I2C for the DS1307 and test succesful connection
    @return True if DS1307 is found, false otherwise.
*/
/**************************************************************************/
bool DS1307::begin(void) {
  return i2c_->Ping(DS1307_ADDRESS);
}

/**************************************************************************/
/*!
    @brief  Is the DS1307 running? Check the Clock Halt bit in register 0
    @return 1 if the RTC is running, 0 if not
*/
/**************************************************************************/
uint8_t DS1307::isrunning(void) {
  uint8_t value;
  if (!i2c_->ReadRegister(DS1307_ADDRESS, REGISTER_TIME_SECONDS, &value))
    return false;
  return !(value >> 7);
}

/**************************************************************************/
/*!
    @brief  Set the date and time in the DS1307
    @param dt DateTime object containing the desired date/time
*/
/**************************************************************************/
bool DS1307::adjust(const DateTime& dt) {
  auto op =
      i2c_->CreateWriteOp(DS1307_ADDRESS, REGISTER_TIME_SECONDS, "adjust");
  const uint8_t values[7] = {
      bin2bcd(dt.second()),
      bin2bcd(dt.minute()),
      bin2bcd(dt.hour()),
      0x0,  // Day of week
      bin2bcd(dt.day()),
      bin2bcd(dt.month()),
      bin2bcd(dt.year() - 2000U),
  };

  op->Write(values, sizeof(values));

  return op->Execute();
}

/**************************************************************************/
/*!
    @brief  Get the current date and time from the DS1307
    @return DateTime object containing the current date and time
*/
/**************************************************************************/
bool DS1307::now(DateTime* dt) {
  auto op = i2c_->CreateReadOp(DS1307_ADDRESS, REGISTER_TIME_SECONDS, "now");

  uint8_t values[7];  // for registers 0x00 - 0x06.
  if (!op->Read(values, sizeof(values)))
    return false;
  if (!op->Execute())
    return false;

  const uint8_t ss = bcd2bin(values[REGISTER_TIME_SECONDS]);
  const uint8_t mm = bcd2bin(values[REGISTER_TIME_MINUTES]);
  const uint8_t hh = bcd2bin(values[REGISTER_TIME_HOURS]);
  // Skip day of week.
  const uint8_t d = bcd2bin(values[REGISTER_TIME_DATE]);
  const uint8_t m = bcd2bin(values[REGISTER_TIME_MONTH]);
  const uint16_t y = bcd2bin(values[REGISTER_TIME_YEAR]);

  *dt = DateTime(y, m, d, hh, mm, ss);
  return true;
}

/**************************************************************************/
/*!
    @brief  Read the current mode of the SQW pin
    @return Mode as Ds1307SqwPinMode enum
*/
/**************************************************************************/
Ds1307SqwPinMode DS1307::readSqwPinMode() {
  uint8_t value;
  if (!i2c_->ReadRegister(DS1307_ADDRESS, REGISTER_CONTROL, &value))
    return DS1307_SquareWaveOff;

  if ((value & CONTROL_SQ_WAVE) == 0x0)
    CLEAR_BITS(value, CONTROL_RS1 | CONTROL_RS0);

  return static_cast<Ds1307SqwPinMode>(
      value & (CONTROL_OUT | CONTROL_SQ_WAVE | CONTROL_RS1 | CONTROL_RS0));
}

/**************************************************************************/
/*!
    @brief  Change the SQW pin mode
    @param mode The mode to use
*/
/**************************************************************************/
bool DS1307::writeSqwPinMode(Ds1307SqwPinMode mode) {
  return i2c_->WriteRegister(DS1307_ADDRESS, REGISTER_CONTROL, mode);
}

/**************************************************************************/
/*!
    @brief  Read data from the DS1307's NVRAM
    @param buf Pointer to a buffer to store the data - make sure it's large
   enough to hold size bytes
    @param num_bytes Number of bytes to read
    @param address Starting NVRAM address, from 0 to 55
*/
/**************************************************************************/
bool DS1307::readnvram(uint8_t address, void* buf, size_t num_bytes) {
  auto op =
      i2c_->CreateReadOp(DS1307_ADDRESS, REGISTER_NVRAM + address, "readnvram");
  if (!op)
    return false;
  if (!op->Read(buf, num_bytes))
    return false;
  return op->Execute();
}

/**************************************************************************/
/*!
    @brief  Write data to the DS1307 NVRAM
    @param address Starting NVRAM address, from 0 to 55
    @param buf Pointer to buffer containing the data to write
    @param num_bytes Number of bytes in buf to write to NVRAM
*/
/**************************************************************************/
bool DS1307::writenvram(uint8_t address, const void* buf, size_t num_bytes) {
  auto op = i2c_->CreateWriteOp(DS1307_ADDRESS, REGISTER_NVRAM + address,
                                "writenvram");
  if (!op)
    return false;
  if (!op->Write(buf, num_bytes))
    return false;
  return op->Execute();
}

}  // namespace rtc
