/*!
  @section license License

  Original library by JeeLabs https://jeelabs.org/pub/docs/rtclib/, released to
  the public domain.

  This version: MIT (see LICENSE)
*/

#include <rtc_pcf8563.h>

#include <rtc_i2c.h>
#include "RTC_util.h"

namespace rtc {

namespace {

// clang-format off
constexpr uint8_t PCF8563_ADDRESS = 0x51;        ///< I2C address for PCF8563

constexpr uint8_t PCF8563_CLKOUTCONTROL = 0x0D;  ///< CLKOUT control register
constexpr uint8_t PCF8563_CONTROL_1 = 0x00;      ///< Control and status register 1
constexpr uint8_t PCF8563_CONTROL_2 = 0x01;      ///< Control and status register 2
constexpr uint8_t PCF8563_VL_SECONDS = 0x02;     ///< register address for VL_SECONDS
constexpr uint8_t PCF8563_CLKOUT_MASK = 0x83;    ///< bitmask for SqwPinMode on CLKOUT pin
// clang-format on

}  // namespace
PCF8563::PCF8563(std::unique_ptr<I2CMaster> i2c) : i2c_(std::move(i2c)) {}

/**************************************************************************/
/*!
    @brief  Start I2C for the PCF8563 and test succesful connection
    @return True if Wire can find PCF8563 or false otherwise.
*/
/**************************************************************************/
bool PCF8563::begin() {
  return i2c_->Ping(PCF8563_ADDRESS);
}

/**************************************************************************/
/*!
    @brief  Check the status of the VL bit in the VL_SECONDS register.
    @details The PCF8563 has an on-chip voltage-low detector. When VDD drops
     below Vlow, bit VL in the VL_seconds register is set to indicate that
     the integrity of the clock information is no longer guaranteed.
    @return True if the bit is set (VDD droped below Vlow) indicating that
    the clock integrity is not guaranteed and false only after the bit is
    cleared using adjust()
*/
/**************************************************************************/

bool PCF8563::lostPower() {
  uint8_t value;
  if (!i2c_->ReadRegister(PCF8563_ADDRESS, PCF8563_VL_SECONDS, &value))
    return false;
  return value >> 7;
}

/**************************************************************************/
/*!
    @brief  Set the date and time
    @param dt DateTime to set
*/
/**************************************************************************/
bool PCF8563::adjust(const DateTime& dt) {
  auto op = i2c_->CreateWriteOp(PCF8563_ADDRESS, PCF8563_VL_SECONDS, "adjust");
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

/**************************************************************************/
/*!
    @brief  Get the current date/time
    @return DateTime object containing the current date/time
*/
/**************************************************************************/

bool PCF8563::now(DateTime* dt) {
  auto op = i2c_->CreateReadOp(PCF8563_ADDRESS, PCF8563_VL_SECONDS, "now");
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

/**************************************************************************/
/*!
    @brief  Resets the STOP bit in register Control_1
*/
/**************************************************************************/
bool PCF8563::start() {
  uint8_t ctlreg;
  if (!i2c_->ReadRegister(PCF8563_ADDRESS, PCF8563_CONTROL_1, &ctlreg))
    return false;

  return i2c_->WriteRegister(PCF8563_ADDRESS, PCF8563_CONTROL_1,
                             ctlreg & ~(1 << 5));
}

/**************************************************************************/
/*!
    @brief  Sets the STOP bit in register Control_1
*/
/**************************************************************************/
bool PCF8563::stop() {
  uint8_t ctlreg;
  if (!i2c_->ReadRegister(PCF8563_ADDRESS, PCF8563_CONTROL_1, &ctlreg))
    return false;

  return i2c_->WriteRegister(PCF8563_ADDRESS, PCF8563_CONTROL_1,
                             ctlreg | (1 << 5));
}

/**************************************************************************/
/*!
    @brief  Is the PCF8563 running? Check the STOP bit in register Control_1
    @return 1 if the RTC is running, 0 if not
*/
/**************************************************************************/
uint8_t PCF8563::isrunning() {
  uint8_t ctlreg;
  if (!i2c_->ReadRegister(PCF8563_ADDRESS, PCF8563_CONTROL_1, &ctlreg))
    return false;
  return !((ctlreg >> 5) & 1);
}

/**************************************************************************/
/*!
    @brief  Read the mode of the CLKOUT pin on the PCF8563
    @return CLKOUT pin mode as a #Pcf8563SqwPinMode enum
*/
/**************************************************************************/
Pcf8563SqwPinMode PCF8563::readSqwPinMode() {
  uint8_t mode;
  if (!i2c_->ReadRegister(PCF8563_ADDRESS, PCF8563_CLKOUTCONTROL, &mode))
    return PCF8563_SquareWaveOFF;
  return static_cast<Pcf8563SqwPinMode>(mode & PCF8563_CLKOUT_MASK);
}

/**************************************************************************/
/*!
    @brief  Set the CLKOUT pin mode on the PCF8563
    @param mode The mode to set, see the #Pcf8563SqwPinMode enum for options
*/
/**************************************************************************/
bool PCF8563::writeSqwPinMode(Pcf8563SqwPinMode mode) {
  return i2c_->WriteRegister(PCF8563_ADDRESS, PCF8563_CLKOUTCONTROL, mode);
}

}  // namespace rtc
