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

  /**
   * Queue a read to be executed later.
   *
   * @param val The address to which to write the bytes read.
   *            this must be valid until this operation is either executed
   *            or deleted (which ever is first).
   * @param num_bytes The number of bytes to read.
   *
   * @return true if the read is successfully queued (but not executed), false
   *         if not.
   */
  bool Read(void* val, size_t num_bytes);

  /**
   * Queue the write of a byte in this operation.
   */
  bool WriteByte(uint8_t val);

  /**
   * Execute all queued tasks.
   */
  bool Execute();

 private:
  friend class I2CMaster;

  I2COperation(i2c_cmd_handle_t cmd,
               i2c_port_t i2c_num,
               SemaphoreHandle_t i2c_mutex);

  i2c_cmd_handle_t cmd_;
  i2c_port_t i2c_num_;
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
   * Start an I2C operation to the I2C slave address.
   *
   * @return The operation pointer - null if error creating operation.
   */
  std::unique_ptr<I2COperation> CreateOp(uint8_t addr);

 private:
  i2c_port_t i2c_num_;
  SemaphoreHandle_t i2c_mutex_;
};

}  // namespace rtc