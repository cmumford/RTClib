/*!
  @section license License

  This version: MIT (see LICENSE)
*/

#include <rtc_i2c.h>

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include <esp_log.h>

namespace rtc {

namespace {
constexpr char TAG[] = "I2C-master";
constexpr bool ACK_CHECK_EN = true;
constexpr TickType_t kI2CCmdWaitTicks = 1000 / portTICK_RATE_MS;
}  // namespace

I2CMaster::I2CMaster(i2c_port_t i2c_num, SemaphoreHandle_t i2c_mutex)
    : i2c_num_(i2c_num), i2c_mutex_(i2c_mutex) {}

I2CMaster::~I2CMaster() = default;

bool I2CMaster::WriteRegister(uint8_t addr, uint8_t reg, uint8_t val) {
  std::unique_ptr<I2COperation> op = CreateWriteOp(addr, "WriteRegister");
  if (!op)
    return false;
  if (!op->WriteByte(reg))
    return false;
  if (!op->WriteByte(val))
    return false;
  return op->Execute();
}

bool I2CMaster::ReadRegister(uint8_t addr, uint8_t reg, uint8_t* val) {
  std::unique_ptr<I2COperation> op = CreateWriteOp(addr, "ReadRegister");
  if (!op)
    return false;
  if (!op->WriteByte(reg))
    return false;
  if (!op->Restart(addr, OperationType::READ))
    return false;
  if (!op->Read(val, sizeof(*val)))
    return false;
  return op->Execute();
}

bool I2CMaster::Ping(uint8_t addr) {
  return true;
}

std::unique_ptr<I2COperation> I2CMaster::CreateWriteOp(uint8_t addr,
                                                       const char* op_name) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  if (!cmd)
    return nullptr;
  esp_err_t err = i2c_master_start(cmd);
  if (err != ESP_OK) {
    i2c_cmd_link_delete(cmd);
    return nullptr;
  }
  err =
      i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
  if (err != ESP_OK) {
    i2c_cmd_link_delete(cmd);
    return nullptr;
  }
  return std::unique_ptr<I2COperation>(
      new I2COperation(cmd, i2c_num_, i2c_mutex_, op_name));
}

std::unique_ptr<I2COperation> I2CMaster::CreateReadOp(uint8_t addr,
                                                      const char* op_name) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  if (!cmd)
    return nullptr;
  esp_err_t err = i2c_master_start(cmd);
  if (err != ESP_OK) {
    i2c_cmd_link_delete(cmd);
    return nullptr;
  }
  err = i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
  if (err != ESP_OK) {
    i2c_cmd_link_delete(cmd);
    return nullptr;
  }
  return std::unique_ptr<I2COperation>(
      new I2COperation(cmd, i2c_num_, i2c_mutex_, op_name));
}

}  // namespace rtc