#include <RTClib.h>

#include <unity.h>

constexpr i2c_port_t kRTCI2CPort = TUNER1_I2C_PORT;
/**
 * The I2C bus speed when running tests.
 *
 * Max of 1MHz recommended by:
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html#_CPPv4N12i2c_config_t9clk_speedE
 */
constexpr uint16_t kI2CClockHz = 100000;

SemaphoreHandle_t g_i2c_mutex;

void test_init_rtc() {
  const i2c_config_t config = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = I2C_SDA_GPIO,
      .scl_io_num = I2C_CLK_GPIO,
      .sda_pullup_en = GPIO_PULLUP_DISABLE,
      .scl_pullup_en = GPIO_PULLUP_DISABLE,
      .master = {.clk_speed = kI2CClockHz},
  };

  esp_err_t err = i2c_param_config(kRTCI2CPort, &config);
  TEST_ASSERT_EQUAL(ESP_OK, err);

  err = i2c_driver_install(kRTCI2CPort, I2C_MODE_MASTER, 0, 0, 0);
  TEST_ASSERT_EQUAL(ESP_OK, err);
}

void test_rtc() {
  std::unique_ptr<rtc::I2CMaster> master(
      new rtc::I2CMaster(kRTCI2CPort, g_i2c_mutex));
  std::unique_ptr<rtc::DS3231> rtc(new rtc::DS3231(std::move(master)));
  TEST_ASSERT_NOT_NULL(rtc);
  TEST_ASSERT_TRUE(rtc->begin());

  const rtc::DateTime dt(2020, 11, 14, 21, 26, 59);
  TEST_ASSERT_TRUE(rtc->adjust(dt));

  const rtc::DateTime now = rtc->now();
  TEST_ASSERT_TRUE(dt.operator==(now));

  const float temp = rtc->getTemperature();
  TEST_ASSERT_GREATER_OR_EQUAL(0, temp);

  rtc->enable32K();
  TEST_ASSERT_TRUE(rtc->isEnabled32K());
  rtc->disable32K();
  TEST_ASSERT_FALSE(rtc->isEnabled32K());

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
  RUN_TEST(test_rtc);

  UNITY_END();
}

extern "C" void app_main() {
  process();
}
