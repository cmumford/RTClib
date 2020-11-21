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
constexpr uint8_t DS3231_I2C_ADDRESS = 0x68;

SemaphoreHandle_t g_i2c_mutex;

using namespace rtc;

namespace {

std::unique_ptr<DS3231> CreateDS3231() {
  std::unique_ptr<I2CMaster> master(new I2CMaster(kRTCI2CPort, g_i2c_mutex));
  std::unique_ptr<DS3231> rtc(new DS3231(std::move(master)));
  return rtc;
}

void test_ds3231_set_and_get_date() {
  auto rtc = CreateDS3231();
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

void test_ds3231_32k() {
  auto rtc = CreateDS3231();
  TEST_ASSERT_NOT_NULL(rtc);
  TEST_ASSERT_TRUE(rtc->begin());

  rtc->enable32K();
  TEST_ASSERT_TRUE(rtc->isEnabled32K());
  rtc->disable32K();
  TEST_ASSERT_FALSE(rtc->isEnabled32K());
}

void test_ds3231_temperature() {
  auto rtc = CreateDS3231();
  TEST_ASSERT_NOT_NULL(rtc);
  TEST_ASSERT_TRUE(rtc->begin());

  const float temp = rtc->getTemperature();
  TEST_ASSERT_GREATER_OR_EQUAL(0, temp);
}

void test_ds3231_square_wave_pin_mode() {
  auto rtc = CreateDS3231();
  TEST_ASSERT_NOT_NULL(rtc);
  TEST_ASSERT_TRUE(rtc->begin());

  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(DS3231::SqwPinMode::Off));
  TEST_ASSERT_EQUAL(DS3231::SqwPinMode::Off, rtc->readSqwPinMode());

  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(DS3231::SqwPinMode::Rate1Hz));
  TEST_ASSERT_EQUAL(DS3231::SqwPinMode::Rate1Hz, rtc->readSqwPinMode());

  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(DS3231::SqwPinMode::Rate1kHz));
  TEST_ASSERT_EQUAL(DS3231::SqwPinMode::Rate1kHz, rtc->readSqwPinMode());

  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(DS3231::SqwPinMode::Rate4kHz));
  TEST_ASSERT_EQUAL(DS3231::SqwPinMode::Rate4kHz, rtc->readSqwPinMode());

  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(DS3231::SqwPinMode::Rate8kHz));
  TEST_ASSERT_EQUAL(DS3231::SqwPinMode::Rate8kHz, rtc->readSqwPinMode());

  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(DS3231::SqwPinMode::Off));
  TEST_ASSERT_EQUAL(DS3231::SqwPinMode::Off, rtc->readSqwPinMode());
}

void test_ds3231_alarm1() {
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
  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(DS3231::SqwPinMode::Off));
  TEST_ASSERT_TRUE(rtc->setAlarm1(dt, DS3231::Alarm1Mode::Hour));

  auto op = i2c_master->CreateReadOp(0x68, 0x07, "test_ds3231_alarm1");
  TEST_ASSERT_NOT_NULL(op);
  uint8_t values[4];
  op->Read(values, sizeof(values));
  TEST_ASSERT_TRUE(op->Execute());
  // TODO: Verify all register values once alarms are completed.
}

void test_ds3231_alarm2() {
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
  TEST_ASSERT_TRUE(rtc->writeSqwPinMode(DS3231::SqwPinMode::Off));

  auto op = i2c_master->CreateReadOp(0x68, 0x0b, "test_ds3231_alarm2");
  TEST_ASSERT_NOT_NULL(op);
  uint8_t values[3];
  op->Read(values, sizeof(values));
  TEST_ASSERT_TRUE(op->Execute());
  // TODO: Verify all register values once alarms are completed.
}

void test_ds3231_agingOffset() {
  auto rtc = CreateDS3231();
  TEST_ASSERT_NOT_NULL(rtc);
  TEST_ASSERT_TRUE(rtc->begin());

  int8_t offset;
  TEST_ASSERT_TRUE(rtc->getAgingOffset(&offset));
}

void process() {
  g_i2c_mutex = xSemaphoreCreateMutex();

  I2CMaster::Initialize(kRTCI2CPort, I2C_SDA_GPIO, I2C_CLK_GPIO, kI2CClockHz);

  UNITY_BEGIN();

  RUN_TEST(test_ds3231_set_and_get_date);
  RUN_TEST(test_ds3231_32k);
  RUN_TEST(test_ds3231_temperature);
  RUN_TEST(test_ds3231_square_wave_pin_mode);
  RUN_TEST(test_ds3231_alarm1);
  RUN_TEST(test_ds3231_alarm2);
  RUN_TEST(test_ds3231_agingOffset);

  UNITY_END();
}

}  // namespace

// Called before each test.
void setUp(void) {
  std::unique_ptr<I2CMaster> master(new I2CMaster(kRTCI2CPort, g_i2c_mutex));
  uint8_t registers[0x13] = {0};
  auto op = master->CreateWriteOp(DS3231_I2C_ADDRESS, 0x0, "setUp");
  op->Write(registers, sizeof(registers));
  op->Execute();
}

extern "C" void app_main() {
  process();
}
