#include <cstdint>
#include <memory>
#include <string>

#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#pragma once

namespace rtc {

/**
 * A single I2C command operation.
 *
 * The command is started when this instance is created, and this instance
 * will automatically stop the operation when deleted.
 */
class I2COperation {
 public:
  ~I2COperation();

  bool Read(void* val, size_t num_bytes);
  bool WriteByte(uint8_t val);
  bool End();

 private:
  friend class I2CMaster;

  I2COperation(i2c_cmd_handle_t cmd, SemaphoreHandle_t i2c_mutex);

  i2c_cmd_handle_t cmd_;
  SemaphoreHandle_t i2c_mutex_;
};

/**
 * Does all I2C bus interaction for the various RTC's.
 */
class I2CMaster {
 public:
  I2CMaster(i2c_port_t i2c_num = I2C_NUM_0,
            SemaphoreHandle_t i2c_mutex = nullptr);
  ~I2CMaster();

  bool WriteRegister(uint8_t addr, uint8_t reg, uint8_t val);
  bool ReadRegister(uint8_t addr, uint8_t reg, uint8_t* val);
  bool Ping(uint8_t addr);

  /**
   * Start a read/write operation to the I2C slave.
   */
  std::unique_ptr<I2COperation> BeginOp(uint8_t addr);

 private:
  i2c_port_t i2c_num_;
  SemaphoreHandle_t i2c_mutex_;
};

} // rtc namespace