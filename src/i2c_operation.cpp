#include <RTClib.h>

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>

#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace rtc {

namespace {
constexpr char TAG[] = "I2C-op";
constexpr TickType_t kI2CCmdWaitTicks = 5000 / portTICK_RATE_MS;
}  // namespace

I2COperation::I2COperation(i2c_cmd_handle_t cmd,
                           i2c_port_t i2c_num,
                           SemaphoreHandle_t i2c_mutex)
    : cmd_(cmd), i2c_num_(i2c_num), i2c_mutex_(i2c_mutex) {}

I2COperation::~I2COperation() {
  if (cmd_) {
    ESP_LOGW(TAG, "Operation created but never executed (doing so now).");
    Execute();
  }
}

bool I2COperation::Read(void* dst, size_t num_bytes) {
  return i2c_master_read(cmd_, static_cast<uint8_t*>(dst), num_bytes,
                         I2C_MASTER_ACK) == ESP_OK;
}

bool I2COperation::WriteByte(uint8_t val) {
  return i2c_master_write_byte(cmd_, val, I2C_MASTER_ACK) == ESP_OK;
}

bool I2COperation::Execute() {
  if (!cmd_)
    return true;

  esp_err_t err = i2c_master_stop(cmd_);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "i2c_master_stop failed: %s", esp_err_to_name(err));

  if (i2c_mutex_)
    xSemaphoreTake(i2c_mutex_, portMAX_DELAY);

  err = i2c_master_cmd_begin(i2c_num_, cmd_, kI2CCmdWaitTicks);

  if (i2c_mutex_)
    xSemaphoreGive(i2c_mutex_);

  i2c_cmd_link_delete(cmd_);
  cmd_ = nullptr;

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "i2c_master_cmd_begin failed: %s", esp_err_to_name(err));
    return false;
  }

  ESP_LOGV(TAG, "I2C operation completed successfully.");
  return true;
}

}  // namespace rtc