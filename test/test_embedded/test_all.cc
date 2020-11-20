#include <RTClib.h>

#include <unity.h>

/**
 * The I2C bus speed when running tests.
 *
 * Max of 1MHz recommended by:
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html#_CPPv4N12i2c_config_t9clk_speedE
 */
constexpr int kI2CClockHz = 100000;
constexpr i2c_port_t kRTCI2CPort = TEST_I2C_PORT1;

SemaphoreHandle_t g_i2c_mutex;

using namespace rtc;

namespace {

std::unique_ptr<DS3231> CreateClock() {
  std::unique_ptr<I2CMaster> master(new I2CMaster(kRTCI2CPort, g_i2c_mutex));
  std::unique_ptr<DS3231> rtc(new DS3231(std::move(master)));
  return rtc;
}

esp_err_t InitializeI2C() {
  const i2c_config_t config = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = I2C_SDA_GPIO,
      .scl_io_num = I2C_CLK_GPIO,
      .sda_pullup_en = GPIO_PULLUP_DISABLE,
      .scl_pullup_en = GPIO_PULLUP_DISABLE,
      .master = {.clk_speed = kI2CClockHz},
  };

  esp_err_t err = i2c_param_config(kRTCI2CPort, &config);
  if (err != ESP_OK)
    return err;

  return i2c_driver_install(kRTCI2CPort, I2C_MODE_MASTER, 0, 0, 0);
}

void test_init_rtc() {
  // TODO: Move to setup().
  TEST_ASSERT_EQUAL(ESP_OK, InitializeI2C());
}

void test_set_and_get_date() {
  auto rtc = CreateClock();
  TEST_ASSERT_NOT_NULL(rtc);
  TEST_ASSERT_TRUE(rtc->begin());

  const DateTime dt(2020, 11, 14, 21, 26, 59);
  DateTime(rtc->adjust(dt));

  // This is a possible flakey test, as there is "daylight" between the set
  // and the get call, and the running clock could increment the time.
  // Instead this test will measure the delta, and only fail if longer than
  // a specified duration.
  DateTime now;
  TEST_ASSERT_TRUE(rtc->now(&now));
  const TimeSpan delta = now - dt;
  TEST_ASSERT_LESS_OR_EQUAL(2, std::abs(delta.totalseconds()));
}

void test_32k() {
  auto rtc = CreateClock();
  TEST_ASSERT_NOT_NULL(rtc);
  TEST_ASSERT_TRUE(rtc->begin());

  rtc->enable32K();
  TEST_ASSERT_TRUE(rtc->isEnabled32K());
  rtc->disable32K();
  TEST_ASSERT_FALSE(rtc->isEnabled32K());
}

void test_temperature() {
  auto rtc = CreateClock();
  TEST_ASSERT_NOT_NULL(rtc);
  TEST_ASSERT_TRUE(rtc->begin());

  const float temp = rtc->getTemperature();
  TEST_ASSERT_GREATER_OR_EQUAL(0, temp);
}

void test_square_wave_pin_mode() {
  auto rtc = CreateClock();
  TEST_ASSERT_NOT_NULL(rtc);
  TEST_ASSERT_TRUE(rtc->begin());

  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(DS3231::SqwPinMode::Alarm));
  TEST_ASSERT_EQUAL(DS3231::SqwPinMode::Alarm, rtc->readSqwPinMode());

  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(DS3231::SqwPinMode::Rate1Hz));
  TEST_ASSERT_EQUAL(DS3231::SqwPinMode::Rate1Hz, rtc->readSqwPinMode());

  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(DS3231::SqwPinMode::Rate1kHz));
  TEST_ASSERT_EQUAL(DS3231::SqwPinMode::Rate1kHz, rtc->readSqwPinMode());

  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(DS3231::SqwPinMode::Rate4kHz));
  TEST_ASSERT_EQUAL(DS3231::SqwPinMode::Rate4kHz, rtc->readSqwPinMode());

  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(DS3231::SqwPinMode::Rate8kHz));
  TEST_ASSERT_EQUAL(DS3231::SqwPinMode::Rate8kHz, rtc->readSqwPinMode());

  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(DS3231::SqwPinMode::Alarm));
  TEST_ASSERT_EQUAL(DS3231::SqwPinMode::Alarm, rtc->readSqwPinMode());
}

void test_alarm1() {
  std::unique_ptr<I2CMaster> master(new I2CMaster(kRTCI2CPort, g_i2c_mutex));
  I2CMaster* const i2c_master = master.get();
  std::unique_ptr<DS3231> rtc(new DS3231(std::move(master)));
  TEST_ASSERT_NOT_NULL(rtc);
  TEST_ASSERT_TRUE(rtc->begin());

  // Enable square wave and verify alarm set failure.
  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(DS3231::SqwPinMode::Rate1Hz));

  const DateTime dt(2021, 1, 12, 7, 13, 31);
  TEST_ASSERT_FALSE(rtc->setAlarm1(dt, DS3231::Alarm1Mode::Hour));

  // Now set to alarm mode.
  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(DS3231::SqwPinMode::Alarm));
  TEST_ASSERT_TRUE(rtc->setAlarm1(dt, DS3231::Alarm1Mode::Hour));

  auto op = i2c_master->CreateReadOp(0x68, 0x07, "test_alarm1");
  TEST_ASSERT_NOT_NULL(op);
  uint8_t values[4];
  op->Read(values, sizeof(values));
  TEST_ASSERT_TRUE(op->Execute());
  // TODO: Verify all register values once alarms are completed.
}

void test_alarm2() {
  std::unique_ptr<I2CMaster> master(new I2CMaster(kRTCI2CPort, g_i2c_mutex));
  I2CMaster* const i2c_master = master.get();
  std::unique_ptr<DS3231> rtc(new DS3231(std::move(master)));
  TEST_ASSERT_NOT_NULL(rtc);
  TEST_ASSERT_TRUE(rtc->begin());

  // Enable square wave and verify alarm set failure.
  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(DS3231::SqwPinMode::Rate1Hz));

  const DateTime dt(2021, 2, 13, 8, 14, 32);
  TEST_ASSERT_FALSE(rtc->setAlarm2(dt, DS3231::Alarm2Mode::Hour));

  // Now set to alarm mode.
  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(DS3231::SqwPinMode::Alarm));

  auto op = i2c_master->CreateReadOp(0x68, 0x0b, "test_alarm2");
  TEST_ASSERT_NOT_NULL(op);
  uint8_t values[3];
  op->Read(values, sizeof(values));
  TEST_ASSERT_TRUE(op->Execute());
  // TODO: Verify all register values once alarms are completed.
}

void test_agingOffset() {
  auto rtc = CreateClock();
  TEST_ASSERT_NOT_NULL(rtc);
  TEST_ASSERT_TRUE(rtc->begin());

  int8_t offset;
  TEST_ASSERT_TRUE(rtc->getAgingOffset(&offset));
}

void process() {
  g_i2c_mutex = xSemaphoreCreateMutex();

  UNITY_BEGIN();

  RUN_TEST(test_init_rtc);
  RUN_TEST(test_set_and_get_date);
  RUN_TEST(test_32k);
  RUN_TEST(test_temperature);
  RUN_TEST(test_square_wave_pin_mode);
  RUN_TEST(test_alarm1);
  RUN_TEST(test_alarm2);
  RUN_TEST(test_agingOffset);

  UNITY_END();
}

}  // namespace

extern "C" void app_main() {
  process();
}
