#include <RTClib.h>

#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace {
constexpr bool ACK_CHECK_EN = true;
}

RTC_I2C_WriteCmd::RTC_I2C_WriteCmd(i2c_cmd_handle_t cmd,
                                   SemaphoreHandle_t i2c_mutex)
    : cmd_(cmd), i2c_mutex_(i2c_mutex) {}

RTC_I2C_WriteCmd::~RTC_I2C_WriteCmd() {
  End();
}

bool RTC_I2C_WriteCmd::WriteByte(uint8_t val) {
  return i2c_master_write(cmd_, &val, sizeof(val), ACK_CHECK_EN) == ESP_OK;
}

bool RTC_I2C_WriteCmd::End() {
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
