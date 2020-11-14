#include <RTClib.h>

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include <esp_log.h>

#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace rtc {

namespace {
constexpr char TAG[] = "I2C-op";
}

I2COperation::I2COperation(i2c_cmd_handle_t cmd, SemaphoreHandle_t i2c_mutex)
    : cmd_(cmd), i2c_mutex_(i2c_mutex) {}

I2COperation::~I2COperation() {
  End();
}

bool I2COperation::Read(void* dst, size_t num_bytes) {
  const esp_err_t err = i2c_master_read(cmd_, static_cast<uint8_t*>(dst),
                                        num_bytes, I2C_MASTER_ACK);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "Read failure: %s", esp_err_to_name(err));
  return err == ESP_OK;
}

bool I2COperation::WriteByte(uint8_t val) {
  const esp_err_t err = i2c_master_write_byte(cmd_, val, I2C_MASTER_ACK);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "Write failure: %s", esp_err_to_name(err));
  return err == ESP_OK;
}

bool I2COperation::End() {
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

} // rtc namespace