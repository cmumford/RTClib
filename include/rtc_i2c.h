#include <cstdint>
#include <memory>
#include <string>

#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#pragma once

namespace rtc {

enum class OperationType { READ, WRITE };

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
   * @brief Queue a write operation.
   *
   * @param val The data to write
   * @param num_bytes The size of \p val.
   *
   * @return true if successfully enqueued, false upon error.
   */
  bool Write(const void* val, size_t num_bytes);

  /**
   * Restart the I2C operation.
   *
   * When this instance is created the operation is already started. This will
   * enqueue another start into this operation.
   */
  bool Restart(uint8_t i2c_address, uint8_t reg, OperationType type);

  /**
   * Execute all queued tasks.
   */
  bool Execute();

 private:
  friend class I2CMaster;

  I2COperation(i2c_cmd_handle_t cmd,
               i2c_port_t i2c_num,
               SemaphoreHandle_t i2c_mutex,
               const char* op_name);

  i2c_cmd_handle_t cmd_;         // The started command.
  i2c_port_t i2c_num_;           // I2C bus or port number.
  SemaphoreHandle_t i2c_mutex_;  // Mutex used for synchronization.
  const char* name_;             // The operation name - used for debugging.
};

/**
 * Does all I2C bus interaction for the various RTC's.
 */
class I2CMaster {
 public:
  /**
   * @brief Initialize the I2C bus.
   *
   * @param i2c_bus The I2C bus (or port).
   * @param sda_gpio The SDA (AKA SDIO) gpio pin number.
   * @param scl_gpio The SCL (AKA SDCL) gpio pin number.
   * @param clk_speed The I2C clock speed.
   *
   * @return true when successful, false when not.
   */
  static bool Initialize(uint8_t i2c_bus,
                         uint8_t sda_gpio,
                         uint8_t scl_gpio,
                         uint32_t clk_speed);

  I2CMaster(i2c_port_t i2c_num = I2C_NUM_0,
            SemaphoreHandle_t i2c_mutex = nullptr);
  ~I2CMaster();

  bool WriteRegister(uint8_t addr, uint8_t reg, uint8_t val);
  bool ReadRegister(uint8_t addr, uint8_t reg, uint8_t* val);
  bool Ping(uint8_t addr);

  /**
   * Start an I2C write operation to the I2C slave address.
   *
   * @return The operation pointer - null if error creating operation.
   */
  std::unique_ptr<I2COperation> CreateWriteOp(uint8_t slave_addr,
                                              uint8_t reg,
                                              const char* op_name);

  /**
   * Start an I2C read operation to the I2C slave address.
   *
   * @return The operation pointer - null if error creating operation.
   */
  std::unique_ptr<I2COperation> CreateReadOp(uint8_t slave_addr,
                                             uint8_t reg,
                                             const char* op_name);

 private:
  i2c_port_t i2c_num_;
  SemaphoreHandle_t i2c_mutex_;
};

}  // namespace rtc
