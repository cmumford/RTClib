#include <RTClib.h>

#include <unity.h>

constexpr i2c_port_t kRTCI2CPort = TUNER1_I2C_PORT;
/**
 * The I2C bus speed when running tests.
 *
 * Max of 1MHz recommended by:
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html#_CPPv4N12i2c_config_t9clk_speedE
 */
constexpr int kI2CClockHz = 100000;

SemaphoreHandle_t g_i2c_mutex;

namespace {

std::unique_ptr<rtc::DS3231> CreateClock() {
  std::unique_ptr<rtc::I2CMaster> master(
      new rtc::I2CMaster(kRTCI2CPort, g_i2c_mutex));
  std::unique_ptr<rtc::DS3231> rtc(new rtc::DS3231(std::move(master)));
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

}  // namespace

void test_init_rtc() {
  // TODO: Move to setup().
  TEST_ASSERT_EQUAL(ESP_OK, InitializeI2C());
}

void test_set_and_get_date() {
  auto rtc = CreateClock();
  TEST_ASSERT_NOT_NULL(rtc);
  TEST_ASSERT_TRUE(rtc->begin());

  const rtc::DateTime dt(2020, 11, 14, 21, 26, 59);
  TEST_ASSERT_TRUE(rtc->adjust(dt));

  // This is a possible race condition as there is "daylight" between the set
  // and the get call, and the time could change.
  const rtc::DateTime now = rtc->now();
  TEST_ASSERT_TRUE(dt.operator==(now));
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

void test_square_pin_mode() {
  auto rtc = CreateClock();
  TEST_ASSERT_NOT_NULL(rtc);
  TEST_ASSERT_TRUE(rtc->begin());

  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(rtc::DS3231_OFF));
  TEST_ASSERT_EQUAL(rtc::DS3231_OFF, rtc->readSqwPinMode());

  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(rtc::DS3231_SquareWave1Hz));
  TEST_ASSERT_EQUAL(rtc::DS3231_SquareWave1Hz, rtc->readSqwPinMode());

  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(rtc::DS3231_SquareWave1kHz));
  TEST_ASSERT_EQUAL(rtc::DS3231_SquareWave1kHz, rtc->readSqwPinMode());

  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(rtc::DS3231_SquareWave4kHz));
  TEST_ASSERT_EQUAL(rtc::DS3231_SquareWave4kHz, rtc->readSqwPinMode());

  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(rtc::DS3231_SquareWave8kHz));
  TEST_ASSERT_EQUAL(rtc::DS3231_SquareWave8kHz, rtc->readSqwPinMode());
}

void process() {
  g_i2c_mutex = xSemaphoreCreateMutex();

  UNITY_BEGIN();

  RUN_TEST(test_init_rtc);
  RUN_TEST(test_set_and_get_date);
  RUN_TEST(test_32k);
  RUN_TEST(test_temperature);
  RUN_TEST(test_square_pin_mode);

  UNITY_END();
}

extern "C" void app_main() {
  process();
}