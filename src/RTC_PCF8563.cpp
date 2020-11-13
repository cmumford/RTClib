/*!
  @section license License

  Original library by JeeLabs https://jeelabs.org/pub/docs/rtclib/, released to
  the public domain.

  This version: MIT (see LICENSE)
*/

#include <RTClib.h>

// START RTC_PCF8563 implementation

/**************************************************************************/
/*!
    @brief  Start I2C for the PCF8563 and test succesful connection
    @return True if Wire can find PCF8563 or false otherwise.
*/
/**************************************************************************/
boolean RTC_PCF8563::begin(void) {
  Wire.begin();
  Wire.beginTransmission(PCF8563_ADDRESS);
  if (Wire.endTransmission() == 0)
    return true;
  return false;
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

boolean RTC_PCF8563::lostPower(void) {
  return (read_i2c_register(PCF8563_ADDRESS, PCF8563_VL_SECONDS) >> 7);
}

/**************************************************************************/
/*!
    @brief  Set the date and time
    @param dt DateTime to set
*/
/**************************************************************************/
void RTC_PCF8563::adjust(const DateTime& dt) {
  Wire.beginTransmission(PCF8563_ADDRESS);
  Wire._I2C_WRITE(PCF8563_VL_SECONDS);  // start at location 2, VL_SECONDS
  Wire._I2C_WRITE(bin2bcd(dt.second()));
  Wire._I2C_WRITE(bin2bcd(dt.minute()));
  Wire._I2C_WRITE(bin2bcd(dt.hour()));
  Wire._I2C_WRITE(bin2bcd(dt.day()));
  Wire._I2C_WRITE(bin2bcd(0));  // skip weekdays
  Wire._I2C_WRITE(bin2bcd(dt.month()));
  Wire._I2C_WRITE(bin2bcd(dt.year() - 2000));
  Wire.endTransmission();
}

/**************************************************************************/
/*!
    @brief  Get the current date/time
    @return DateTime object containing the current date/time
*/
/**************************************************************************/

DateTime RTC_PCF8563::now() {
  Wire.beginTransmission(PCF8563_ADDRESS);
  Wire._I2C_WRITE((byte)2);
  Wire.endTransmission();

  Wire.requestFrom(PCF8563_ADDRESS, 7);
  uint8_t ss = bcd2bin(Wire._I2C_READ() & 0x7F);
  uint8_t mm = bcd2bin(Wire._I2C_READ() & 0x7F);
  uint8_t hh = bcd2bin(Wire._I2C_READ() & 0x3F);
  uint8_t d = bcd2bin(Wire._I2C_READ() & 0x3F);
  Wire._I2C_READ();  // skip 'weekdays'
  uint8_t m = bcd2bin(Wire._I2C_READ() & 0x1F);
  uint16_t y = bcd2bin(Wire._I2C_READ()) + 2000;

  return DateTime(y, m, d, hh, mm, ss);
}

/**************************************************************************/
/*!
    @brief  Resets the STOP bit in register Control_1
*/
/**************************************************************************/
void RTC_PCF8563::start(void) {
  uint8_t ctlreg = read_i2c_register(PCF8563_ADDRESS, PCF8563_CONTROL_1);
  if (ctlreg & (1 << 5)) {
    write_i2c_register(PCF8563_ADDRESS, PCF8563_CONTROL_1, ctlreg & ~(1 << 5));
  }
}

/**************************************************************************/
/*!
    @brief  Sets the STOP bit in register Control_1
*/
/**************************************************************************/
void RTC_PCF8563::stop(void) {
  uint8_t ctlreg = read_i2c_register(PCF8563_ADDRESS, PCF8563_CONTROL_1);
  if (!(ctlreg & (1 << 5))) {
    write_i2c_register(PCF8523_ADDRESS, PCF8563_CONTROL_1, ctlreg | (1 << 5));
  }
}

/**************************************************************************/
/*!
    @brief  Is the PCF8563 running? Check the STOP bit in register Control_1
    @return 1 if the RTC is running, 0 if not
*/
/**************************************************************************/
uint8_t RTC_PCF8563::isrunning() {
  uint8_t ctlreg = read_i2c_register(PCF8563_ADDRESS, PCF8563_CONTROL_1);
  return !((ctlreg >> 5) & 1);
}

/**************************************************************************/
/*!
    @brief  Read the mode of the CLKOUT pin on the PCF8563
    @return CLKOUT pin mode as a #Pcf8563SqwPinMode enum
*/
/**************************************************************************/
Pcf8563SqwPinMode RTC_PCF8563::readSqwPinMode() {
  int mode;

  Wire.beginTransmission(PCF8563_ADDRESS);
  Wire._I2C_WRITE(PCF8563_CLKOUTCONTROL);
  Wire.endTransmission();

  Wire.requestFrom((uint8_t)PCF8563_ADDRESS, (uint8_t)1);
  mode = Wire._I2C_READ();

  return static_cast<Pcf8563SqwPinMode>(mode & PCF8563_CLKOUT_MASK);
}

/**************************************************************************/
/*!
    @brief  Set the CLKOUT pin mode on the PCF8563
    @param mode The mode to set, see the #Pcf8563SqwPinMode enum for options
*/
/**************************************************************************/
void RTC_PCF8563::writeSqwPinMode(Pcf8563SqwPinMode mode) {
  Wire.beginTransmission(PCF8563_ADDRESS);
  Wire._I2C_WRITE(PCF8563_CLKOUTCONTROL);
  Wire._I2C_WRITE(mode);
  Wire.endTransmission();
}
// END RTC_PCF8563 implementation
