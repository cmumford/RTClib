/*!
  @section license License

  This version: MIT (see LICENSE)
*/

#include <RTClib.h>

RTC_I2C::RTC_I2C() = default;

RTC_I2C::~RTC_I2C() = default;

bool RTC_I2C::WriteRegister(uint8_t addr, uint8_t reg, uint8_t val) {
  return true;
}

bool RTC_I2C::ReadRegister(uint8_t addr, uint8_t reg, uint8_t* val) {
  return true;
}

bool RTC_I2C::Ping(uint8_t addr) {
  return true;
}

std::unique_ptr<RTC_I2C_WriteCmd> RTC_I2C::BeginWrite(uint8_t addr) {
  return std::unique_ptr<RTC_I2C_WriteCmd>(new RTC_I2C_WriteCmd());
}

std::unique_ptr<RTC_I2C_ReadCmd> RTC_I2C::BeginRead(uint8_t addr,
                                                    uint32_t num_bytes) {
  return std::unique_ptr<RTC_I2C_ReadCmd>(new RTC_I2C_ReadCmd());
}