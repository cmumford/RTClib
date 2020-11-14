#include <RTClib.h>

#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

RTC_I2C_ReadCmd::RTC_I2C_ReadCmd(i2c_cmd_handle_t cmd,
                                 SemaphoreHandle_t i2c_mutex)
    : cmd_(cmd), i2c_mutex_(i2c_mutex) {}

RTC_I2C_ReadCmd::~RTC_I2C_ReadCmd() {
  End();
}

bool RTC_I2C_ReadCmd::ReadByte(uint8_t* val) {
  return i2c_master_read(cmd_, val, sizeof(*val), I2C_MASTER_ACK) == ESP_OK;
}

bool RTC_I2C_ReadCmd::End() {
  if (!cmd_)
    return true;

  const esp_err_t err = i2c_master_stop(cmd_);
  i2c_cmd_link_delete(cmd_);
  cmd_ = nullptr;

  if (i2c_mutex_) {
    xSemaphoreGive(i2c_mutex_);
    i2c_mutex_ = nullptr;
  }

  return err == ESP_OK;
}
