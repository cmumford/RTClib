/*!
  @section license License

  This version: MIT (see LICENSE)
*/

#include <RTClib.h>

namespace rtc {

namespace {
constexpr bool ACK_CHECK_EN = true;
constexpr TickType_t kI2CCmdWaitTicks = 1000 / portTICK_RATE_MS;
}  // namespace

I2CMaster::I2CMaster(i2c_port_t i2c_num, SemaphoreHandle_t i2c_mutex)
    : i2c_num_(i2c_num), i2c_mutex_(i2c_mutex) {}

I2CMaster::~I2CMaster() = default;

bool I2CMaster::WriteRegister(uint8_t addr, uint8_t reg, uint8_t val) {
  if (i2c_mutex_)
    xSemaphoreTake(i2c_mutex_, portMAX_DELAY);
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  esp_err_t err = i2c_master_start(cmd);
  if (err != ESP_OK)
    goto WRITE_END;
  err =
      i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
  if (err != ESP_OK)
    goto WRITE_END;
  err = i2c_master_write(cmd, reinterpret_cast<uint8_t*>(&reg), sizeof(reg),
                         ACK_CHECK_EN);
  if (err != ESP_OK)
    goto WRITE_END;
  err = i2c_master_write(cmd, reinterpret_cast<uint8_t*>(&val), sizeof(val),
                         ACK_CHECK_EN);
  if (err != ESP_OK)
    goto WRITE_END;
  err = i2c_master_stop(cmd);
  if (err != ESP_OK)
    goto WRITE_END;
  err = i2c_master_cmd_begin(i2c_num_, cmd, kI2CCmdWaitTicks);

WRITE_END:
  i2c_cmd_link_delete(cmd);
  if (i2c_mutex_)
    xSemaphoreGive(i2c_mutex_);
  return err;
}

bool I2CMaster::ReadRegister(uint8_t addr, uint8_t reg, uint8_t* val) {
  if (i2c_mutex_)
    xSemaphoreTake(i2c_mutex_, portMAX_DELAY);
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  esp_err_t err = i2c_master_start(cmd);
  if (err != ESP_OK)
    goto READ_END;
  err =
      i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
  if (err != ESP_OK)
    goto READ_END;
  err = i2c_master_write(cmd, reinterpret_cast<uint8_t*>(&reg), sizeof(reg),
                         ACK_CHECK_EN);
  if (err != ESP_OK)
    goto READ_END;
  err = i2c_master_read(cmd, reinterpret_cast<uint8_t*>(&val), sizeof(val),
                        I2C_MASTER_ACK);
  if (err != ESP_OK)
    goto READ_END;
  err = i2c_master_stop(cmd);
  if (err != ESP_OK)
    goto READ_END;
  err = i2c_master_cmd_begin(i2c_num_, cmd, kI2CCmdWaitTicks);

READ_END:
  i2c_cmd_link_delete(cmd);
  if (i2c_mutex_)
    xSemaphoreGive(i2c_mutex_);
  return err;
}

bool I2CMaster::Ping(uint8_t addr) {
  return true;
}

std::unique_ptr<I2COperation> I2CMaster::BeginOp(uint8_t addr) {
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
  return std::unique_ptr<I2COperation>(new I2COperation(cmd, i2c_mutex_));
}

} // rtc namespace