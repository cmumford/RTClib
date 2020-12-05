/**
 * @section license License
 *
 * This file is subject to the terms and conditions defined in
 * file 'license.txt', which is part of this source code package.
 */

#include <unity.h>

#include <i2clib/master.h>
#include <i2clib/operation.h>
#include <rtclib/datetime.h>
#include <rtclib/ds1307.h>
#include <rtclib/ds3231.h>
#include <rtclib/pcf8523.h>
#include <rtclib/pcf8563.h>
#include <rtclib/timespan.h>

using i2c::Master;

using namespace rtc;

namespace {

/**
 * The I2C bus speed when running tests.
 *
 * Max of 1MHz recommended by:
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html#_CPPv4N12i2c_config_t9clk_speedE
 */
constexpr int kI2CClockHz = 100000;
constexpr uint8_t DS3231_I2C_ADDRESS = 0x68;
constexpr uint8_t DS1307_I2C_ADDRESS = 0x68;
constexpr uint8_t PCF8523_I2C_ADDRESS = 0x68;
constexpr uint8_t PCF8563_I2C_ADDRESS = 0x51;
constexpr TickType_t kStartupDelay = 1000 / portTICK_PERIOD_MS;

SemaphoreHandle_t g_i2c_mutex;
int g_test_clock;

DS3231 CreateDS3231() {
  return DS3231(Master(TEST_I2C_PORT, g_i2c_mutex));
}

DS1307 CreateDS1307() {
  return DS1307(Master(TEST_I2C_PORT, g_i2c_mutex));
}

PCF8563 CreatePCF8563() {
  return PCF8563(Master(TEST_I2C_PORT, g_i2c_mutex));
}

PCF8523 CreatePCF8523() {
  return PCF8523(Master(TEST_I2C_PORT, g_i2c_mutex));
}

void test_pcf8523_set_and_get_date() {
  auto rtc = CreatePCF8523();
  TEST_ASSERT_TRUE(rtc.begin());

  const DateTime dt(2020, 11, 14, 21, 26, 59);
  DateTime(rtc.adjust(dt));

  // This is a possible flakey test, as there is "daylight" between the set
  // and the get call, and the running clock could increment the time.
  // Instead this test will measure the delta, and only fail if longer than
  // a specified duration.
  DateTime now;
  TEST_ASSERT_TRUE(rtc.now(&now));
  const TimeSpan delta = now - dt;
  TEST_ASSERT_LESS_OR_EQUAL(2, std::abs(delta.totalseconds()));
}

void test_pcf8523_square_wave_pin_mode() {
  auto rtc = CreatePCF8523();
  TEST_ASSERT_TRUE(rtc.begin());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(PCF8523::SqwPinMode::Off));
  TEST_ASSERT_EQUAL(PCF8523::SqwPinMode::Off, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(PCF8523::SqwPinMode::Rate1Hz));
  TEST_ASSERT_EQUAL(PCF8523::SqwPinMode::Rate1Hz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(PCF8523::SqwPinMode::Rate32Hz));
  TEST_ASSERT_EQUAL(PCF8523::SqwPinMode::Rate32Hz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(PCF8523::SqwPinMode::Rate1kHz));
  TEST_ASSERT_EQUAL(PCF8523::SqwPinMode::Rate1kHz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(PCF8523::SqwPinMode::Rate4kHz));
  TEST_ASSERT_EQUAL(PCF8523::SqwPinMode::Rate4kHz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(PCF8523::SqwPinMode::Rate8kHz));
  TEST_ASSERT_EQUAL(PCF8523::SqwPinMode::Rate8kHz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(PCF8523::SqwPinMode::Rate16kHz));
  TEST_ASSERT_EQUAL(PCF8523::SqwPinMode::Rate16kHz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(PCF8523::SqwPinMode::Rate32kHz));
  TEST_ASSERT_EQUAL(PCF8523::SqwPinMode::Rate32kHz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(PCF8523::SqwPinMode::Off));
  TEST_ASSERT_EQUAL(PCF8523::SqwPinMode::Off, rtc.readSqwPinMode());
}

void test_pcf8563_set_and_get_date() {
  auto rtc = CreatePCF8563();
  TEST_ASSERT_TRUE(rtc.begin());

  const DateTime dt(2020, 11, 14, 21, 26, 59);
  DateTime(rtc.adjust(dt));

  // This is a possible flakey test, as there is "daylight" between the set
  // and the get call, and the running clock could increment the time.
  // Instead this test will measure the delta, and only fail if longer than
  // a specified duration.
  DateTime now;
  TEST_ASSERT_TRUE(rtc.now(&now));
  const TimeSpan delta = now - dt;
  TEST_ASSERT_LESS_OR_EQUAL(2, std::abs(delta.totalseconds()));
}

void test_pcf8563_square_wave_pin_mode() {
  auto rtc = CreatePCF8563();
  TEST_ASSERT_TRUE(rtc.begin());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(PCF8563::SqwPinMode::Off));
  TEST_ASSERT_EQUAL(PCF8563::SqwPinMode::Off, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(PCF8563::SqwPinMode::Rate1Hz));
  TEST_ASSERT_EQUAL(PCF8563::SqwPinMode::Rate1Hz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(PCF8563::SqwPinMode::Rate32Hz));
  TEST_ASSERT_EQUAL(PCF8563::SqwPinMode::Rate32Hz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(PCF8563::SqwPinMode::Rate1kHz));
  TEST_ASSERT_EQUAL(PCF8563::SqwPinMode::Rate1kHz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(PCF8563::SqwPinMode::Rate32Hz));
  TEST_ASSERT_EQUAL(PCF8563::SqwPinMode::Rate32Hz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(PCF8563::SqwPinMode::Off));
  TEST_ASSERT_EQUAL(PCF8563::SqwPinMode::Off, rtc.readSqwPinMode());
}

void test_ds1307_set_and_get_date() {
  auto rtc = CreateDS1307();
  TEST_ASSERT_TRUE(rtc.begin());

  const DateTime dt(2020, 11, 14, 21, 26, 59);
  DateTime(rtc.adjust(dt));

  // This is a possible flakey test, as there is "daylight" between the set
  // and the get call, and the running clock could increment the time.
  // Instead this test will measure the delta, and only fail if longer than
  // a specified duration.
  DateTime now;
  TEST_ASSERT_TRUE(rtc.now(&now));
  const TimeSpan delta = now - dt;
  TEST_ASSERT_LESS_OR_EQUAL(2, std::abs(delta.totalseconds()));
}

void test_ds1307_square_wave_pin_mode() {
  auto rtc = CreateDS1307();
  TEST_ASSERT_TRUE(rtc.begin());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(DS1307::SqwPinMode::Off));
  TEST_ASSERT_EQUAL(DS1307::SqwPinMode::Off, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(DS1307::SqwPinMode::On));
  TEST_ASSERT_EQUAL(DS1307::SqwPinMode::On, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(DS1307::SqwPinMode::Rate1Hz));
  TEST_ASSERT_EQUAL(DS1307::SqwPinMode::Rate1Hz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(DS1307::SqwPinMode::Rate4kHz));
  TEST_ASSERT_EQUAL(DS1307::SqwPinMode::Rate4kHz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(DS1307::SqwPinMode::Rate8kHz));
  TEST_ASSERT_EQUAL(DS1307::SqwPinMode::Rate8kHz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(DS1307::SqwPinMode::Rate32kHz));
  TEST_ASSERT_EQUAL(DS1307::SqwPinMode::Rate32kHz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(DS1307::SqwPinMode::Off));
  TEST_ASSERT_EQUAL(DS1307::SqwPinMode::Off, rtc.readSqwPinMode());
}

void test_ds3231_set_and_get_date() {
  auto rtc = CreateDS3231();
  TEST_ASSERT_TRUE(rtc.begin());

  const DateTime dt(2020, 11, 14, 21, 26, 59);
  DateTime(rtc.adjust(dt));

  // This is a possible flakey test, as there is "daylight" between the set
  // and the get call, and the running clock could increment the time.
  // Instead this test will measure the delta, and only fail if longer than
  // a specified duration.
  DateTime now;
  TEST_ASSERT_TRUE(rtc.now(&now));
  const TimeSpan delta = now - dt;
  TEST_ASSERT_LESS_OR_EQUAL(2, std::abs(delta.totalseconds()));
}

void test_ds3231_32k() {
  auto rtc = CreateDS3231();
  TEST_ASSERT_TRUE(rtc.begin());

  rtc.enable32K();
  TEST_ASSERT_TRUE(rtc.isEnabled32K());
  rtc.disable32K();
  TEST_ASSERT_FALSE(rtc.isEnabled32K());
}

void test_ds3231_temperature() {
  auto rtc = CreateDS3231();
  TEST_ASSERT_TRUE(rtc.begin());

  const float temp = rtc.getTemperature();
  TEST_ASSERT_GREATER_OR_EQUAL(0, temp);
}

void test_ds3231_square_wave_pin_mode() {
  auto rtc = CreateDS3231();
  TEST_ASSERT_TRUE(rtc.begin());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(DS3231::SqwPinMode::Off));
  TEST_ASSERT_EQUAL(DS3231::SqwPinMode::Off, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(DS3231::SqwPinMode::Rate1Hz));
  TEST_ASSERT_EQUAL(DS3231::SqwPinMode::Rate1Hz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(DS3231::SqwPinMode::Rate1kHz));
  TEST_ASSERT_EQUAL(DS3231::SqwPinMode::Rate1kHz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(DS3231::SqwPinMode::Rate4kHz));
  TEST_ASSERT_EQUAL(DS3231::SqwPinMode::Rate4kHz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(DS3231::SqwPinMode::Rate8kHz));
  TEST_ASSERT_EQUAL(DS3231::SqwPinMode::Rate8kHz, rtc.readSqwPinMode());

  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(DS3231::SqwPinMode::Off));
  TEST_ASSERT_EQUAL(DS3231::SqwPinMode::Off, rtc.readSqwPinMode());
}

void test_ds3231_alarm1() {
  DS3231 rtc(Master(TEST_I2C_PORT, g_i2c_mutex));
  TEST_ASSERT_TRUE(rtc.begin());

  // Enable square wave and verify alarm set failure.
  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(DS3231::SqwPinMode::Rate1Hz));

  const DateTime dt(2021, 1, 12, 7, 13, 31);
  TEST_ASSERT_FALSE(rtc.setAlarm1(dt, DS3231::Alarm1Mode::Hour));

  // Now set to alarm mode.
  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(DS3231::SqwPinMode::Off));
  TEST_ASSERT_TRUE(rtc.setAlarm1(dt, DS3231::Alarm1Mode::Hour));

  Master master(TEST_I2C_PORT, g_i2c_mutex);
  auto op = master.CreateReadOp(0x68, 0x07, "test_ds3231_alarm1");
  TEST_ASSERT_TRUE(op.ready());
  uint8_t values[4];
  op.Read(values, sizeof(values));
  TEST_ASSERT_TRUE(op.Execute());
  // TODO: Verify all register values once alarms are completed.
}

void test_ds3231_alarm2() {
  DS3231 rtc(Master(TEST_I2C_PORT, g_i2c_mutex));
  TEST_ASSERT_TRUE(rtc.begin());

  // Enable square wave and verify alarm set failure.
  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(DS3231::SqwPinMode::Rate1Hz));

  const DateTime dt(2021, 2, 13, 8, 14, 32);
  TEST_ASSERT_FALSE(rtc.setAlarm2(dt, DS3231::Alarm2Mode::Hour));

  // Now set to alarm mode.
  TEST_ASSERT_TRUE(rtc.writeSqwPinMode(DS3231::SqwPinMode::Off));

  Master master(TEST_I2C_PORT, g_i2c_mutex);
  auto op = master.CreateReadOp(0x68, 0x0b, "test_ds3231_alarm2");
  TEST_ASSERT_TRUE(op.ready());
  uint8_t values[3];
  op.Read(values, sizeof(values));
  TEST_ASSERT_TRUE(op.Execute());
  // TODO: Verify all register values once alarms are completed.
}

void test_ds3231_agingOffset() {
  auto rtc = CreateDS3231();
  TEST_ASSERT_TRUE(rtc.begin());

  int8_t offset;
  TEST_ASSERT_TRUE(rtc.getAgingOffset(&offset));
}

void process() {
  g_i2c_mutex = xSemaphoreCreateMutex();

  //    RTC  | SDA | SCL | Slave Addr
  // ================================
  // DS3231  | 21  | 22  | 0x68
  // DS1307  | 19  | 18  | 0x68
  // PCF8523 |  4  | 15  | 0x68
  // PCF8563 | 21  | 22  | 0x51

  UNITY_BEGIN();

  Master::Initialize(TEST_I2C_PORT, DS3231_I2C_SDA_GPIO, DS3231_I2C_CLK_GPIO,
                     kI2CClockHz);
  g_test_clock = 3231;
  RUN_TEST(test_ds3231_set_and_get_date);
  RUN_TEST(test_ds3231_32k);
  RUN_TEST(test_ds3231_temperature);
  RUN_TEST(test_ds3231_square_wave_pin_mode);
  RUN_TEST(test_ds3231_alarm1);
  RUN_TEST(test_ds3231_alarm2);
  RUN_TEST(test_ds3231_agingOffset);
  Master::Shutdown(TEST_I2C_PORT);

  Master::Initialize(TEST_I2C_PORT, DS1307_I2C_SDA_GPIO, DS1307_I2C_CLK_GPIO,
                     kI2CClockHz);
  g_test_clock = 1307;
  RUN_TEST(test_ds1307_set_and_get_date);
  RUN_TEST(test_ds1307_square_wave_pin_mode);
  Master::Shutdown(TEST_I2C_PORT);

  Master::Initialize(TEST_I2C_PORT, PCF8523_I2C_SDA_GPIO, PCF8523_I2C_CLK_GPIO,
                     kI2CClockHz);
  g_test_clock = 8523;
  RUN_TEST(test_pcf8523_set_and_get_date);
  RUN_TEST(test_pcf8523_square_wave_pin_mode);
  Master::Shutdown(TEST_I2C_PORT);

  Master::Initialize(TEST_I2C_PORT, PCF8563_I2C_SDA_GPIO, PCF8563_I2C_CLK_GPIO,
                     kI2CClockHz);
  g_test_clock = 8563;
  RUN_TEST(test_pcf8563_set_and_get_date);
  RUN_TEST(test_pcf8563_square_wave_pin_mode);
  Master::Shutdown(TEST_I2C_PORT);

  UNITY_END();
}

bool clear_ds3231_registers() {
  Master master(TEST_I2C_PORT, g_i2c_mutex);
  uint8_t registers[1 + 0x12] = {0};
  auto op = master.CreateWriteOp(DS3231_I2C_ADDRESS, 0x0, "clear_DS3231");
  if (!op.ready())
    return false;
  if (!op.Write(registers, sizeof(registers)))
    return false;
  return op.Execute();
}

bool clear_ds1307_registers() {
  Master master(TEST_I2C_PORT, g_i2c_mutex);
  uint8_t registers[1 + 0x07] = {0};
  auto op = master.CreateWriteOp(DS1307_I2C_ADDRESS, 0x0, "clear_DS1307");
  if (!op.ready())
    return false;
  if (!op.Write(registers, sizeof(registers)))
    return false;
  return op.Execute();
}

bool clear_pcf8523_registers() {
  Master master(TEST_I2C_PORT, g_i2c_mutex);
  uint8_t registers[1 + 0x13] = {0};
  auto op = master.CreateWriteOp(PCF8523_I2C_ADDRESS, 0x0, "clear_PCF8523");
  if (!op.ready())
    return false;
  if (!op.Write(registers, sizeof(registers)))
    return false;
  return op.Execute();
}

bool clear_pcf8563_registers() {
  Master master(TEST_I2C_PORT, g_i2c_mutex);
  uint8_t registers[1 + 0x0f] = {0};
  auto op = master.CreateWriteOp(PCF8563_I2C_ADDRESS, 0x0, "clear_PCF8563");
  if (!op.ready())
    return false;
  if (!op.Write(registers, sizeof(registers)))
    return false;
  return op.Execute();
}

}  // namespace

// Called before each test.
void setUp(void) {
  switch (g_test_clock) {
    case 8523:
      clear_pcf8523_registers();
      break;
    case 8563:
      clear_pcf8563_registers();
      break;
    case 3231:
      clear_ds3231_registers();
      break;
    case 1307:
      clear_ds1307_registers();
      break;
  }
}

void WaitForDebugMonitor() {
  // Poor man's way of waiting till the monitor has connected.
  vTaskDelay(kStartupDelay);
}

extern "C" void app_main() {
  WaitForDebugMonitor();
  process();
}
