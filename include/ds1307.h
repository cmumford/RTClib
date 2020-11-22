/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 *
 * This is a fork of Adafruit's RTClib library.
 * https://github.com/adafruit/RTClib
 */

#ifndef RTC_DS1307_H_
#define RTC_DS1307_H_

#include <memory>

#include "rtc_datetime.h"
namespace rtc {

class I2CMaster;

/**************************************************************************/
/*!
    @brief  RTC based on the DS1307 chip connected via I2C.
*/
/**************************************************************************/
class DS1307 {
 public:
  enum class SqwPinMode {
    Off,       // Set SQW/OUT pin set to zero.
    On,        // Set SQW/OUT pin set to 1.
    Rate1Hz,   // 1Hz square wave.
    Rate4kHz,  // 4kHz square wave.
    Rate8kHz,  // 8kHz square wave.
    Rate32kHz  // 32kHz square wave.
  };

  DS1307(std::unique_ptr<I2CMaster> i2c);

  /**
   * Start I2C for the DS1307 and test succesful connection.
   *
   * @return True if DS1307 is found, false otherwise.
   */
  bool begin(void);

  /**
   * Set the date and time in the DS1307.
   *
   * @param dt DateTime object containing the desired date/time
   * @return True if successful, false if not.
   */
  bool adjust(const DateTime& dt);

  /**
   * Is the DS1307 running?
   *
   * Check the Clock Halt bit in register 0.
   *
   * @return true if running, false if not.
   */
  bool isRunning(void);

  /**
   * Get the current date and time from the DS1307.
   *
   * @param now Address to write the current date/time.
   * @return true if successful, false if not.
   */
  bool now(DateTime* now);

  /**
   * Read the current mode of the SQW pin.
   *
   * @return The current square pin mode.
   */
  SqwPinMode readSqwPinMode();

  /**
   * Change the SQW pin mode.
   *
   * @param mode The desired mode.
   * @return true if successful, false if not.
   */
  bool writeSqwPinMode(SqwPinMode mode);

  /**
   * Read data from the DS1307's NVRAM.
   * @param buf Pointer to a buffer to store the data - make sure it's large
   *            enough to hold size bytes
   * @param num_bytes Number of bytes to read
   * @param address Starting NVRAM address, from 0 to 55
   * @return true if successful, false if not.
   */
  bool readnvram(uint8_t address, void* buf, size_t num_bytes);

  /**
   * Write data to the DS1307 NVRAM.
   *
   * @param address Starting NVRAM address, from 0 to 55
   * @param buf Pointer to buffer containing the data to write
   * @param num_bytes Number of bytes in buf to write to NVRAM
   * @return true if successful, false if not.
   */
  bool writeNVRAM(uint8_t address, const void* buf, size_t num_bytes);

 private:
  std::unique_ptr<I2CMaster> const i2c_;
};

}  // namespace rtc

#endif  // RTC_DS1307_H_
