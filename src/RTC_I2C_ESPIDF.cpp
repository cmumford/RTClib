/*!
  @section license License

  This version: MIT (see LICENSE)
*/

#include <RTClib.h>

namespace {
constexpr bool ACK_CHECK_EN = true;
constexpr TickType_t kI2CCmdWaitTicks = 1000 / portTICK_RATE_MS;
}  // namespace

RTC_I2C::RTC_I2C(i2c_port_t i2c_num, SemaphoreHandle_t i2c_mutex)
    : i2c_num_(i2c_num), i2c_mutex_(i2c_mutex) {}

RTC_I2C::~RTC_I2C() = default;

bool RTC_I2C::WriteRegister(uint8_t addr, uint8_t reg, uint8_t val) {
  if (i2c_mutex_)
    xSemaphoreTake(i2c_mutex_, portMAX_DELAY);
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  esp_err_t err = i2c_master_start(cmd);
  if (err != ESP_OK)
    goto SET_END;
  err =
      i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
  if (err != ESP_OK)
    goto SET_END;
  err = i2c_master_write(cmd, reinterpret_cast<uint8_t*>(&reg), sizeof(reg),
                         ACK_CHECK_EN);
  if (err != ESP_OK)
    goto SET_END;
  err = i2c_master_stop(cmd);
  if (err != ESP_OK)
    goto SET_END;
  err = i2c_master_cmd_begin(i2c_num_, cmd, kI2CCmdWaitTicks);

SET_END:
  i2c_cmd_link_delete(cmd);
  if (i2c_mutex_)
    xSemaphoreGive(i2c_mutex_);
  return err;
}

bool RTC_I2C::ReadRegister(uint8_t addr, uint8_t reg, uint8_t* val) {
  return true;
}

bool RTC_I2C::Ping(uint8_t addr) {
  return true;
}

std::unique_ptr<RTC_I2C_WriteCmd> RTC_I2C::BeginWrite(uint8_t addr) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  if (!cmd)
    return nullptr;
  if (i2c_mutex_)
    xSemaphoreTake(i2c_mutex_, portMAX_DELAY);
  esp_err_t err = i2c_master_start(cmd);
  if (err != ESP_OK) {
    i2c_cmd_link_delete(cmd);
    if (i2c_mutex_)
      xSemaphoreGive(i2c_mutex_);
    return nullptr;
  }
  err =
      i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
  if (err != ESP_OK) {
    i2c_master_stop(cmd);
    i2c_cmd_link_delete(cmd);
    if (i2c_mutex_)
      xSemaphoreGive(i2c_mutex_);
    return nullptr;
  }
  return std::unique_ptr<RTC_I2C_WriteCmd>(
      new RTC_I2C_WriteCmd(cmd, i2c_mutex_));
}

std::unique_ptr<RTC_I2C_ReadCmd> RTC_I2C::BeginRead(uint8_t addr,
                                                    uint32_t num_bytes) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  if (!cmd)
    return nullptr;
  if (i2c_mutex_)
    xSemaphoreTake(i2c_mutex_, portMAX_DELAY);
  esp_err_t err = i2c_master_start(cmd);
  if (err != ESP_OK) {
    if (i2c_mutex_)
      xSemaphoreGive(i2c_mutex_);
    i2c_cmd_link_delete(cmd);
    return nullptr;
  }
  err =
      i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
  if (err != ESP_OK) {
    i2c_master_stop(cmd);
    if (i2c_mutex_)
      xSemaphoreGive(i2c_mutex_);
    i2c_cmd_link_delete(cmd);
    return nullptr;
  }
  return std::unique_ptr<RTC_I2C_ReadCmd>(new RTC_I2C_ReadCmd(cmd, i2c_mutex_));
}