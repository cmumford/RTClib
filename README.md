This is a fork of Adafruit's [RTCLib](https://github.com/adafruit/RTClib) - which,
in turn, is a fork of JeeLab's real time clock library for Arduino.

The primary reason for this fork is to port to
[ESP-IDF](https://docs.espressif.com/projects/esp-idf).

Addionally:

1. Return values (usually boolean) indicate success/failure
   of most calls.
2. API requires C++11.
3. Bug fixes.
4. Unit tests.
5. PulseView protocol decoders.

This is currently a [PlatformIO](https://platformio.org/) library.

Here is a simple example of retrieving the current time
from the RTC:

```c++
using namespace rtc;

// Initialize I2C.
I2CMaster::Initialize(I2C_PORT, I2C_SDA_GPIO,
                      I2C_CLK_GPIO, I2C_CLOCK_SPEED);

// Create an I2C master to do all the I2C stuff.
std::unique_ptr<I2CMaster> master(new I2CMaster(I2C_PORT, nullptr));

// Allocate an DS3231 RTC object.
std::unique_ptr<DS3231> rtc(new DS3231(std::move(master)));
rtc->begin();

DateTime now;
rtc->now(&now);
```

## Developer Notes

The implementation is largely platform independent, platform specific
code is restricted to a few files. Support for Arduino might be re-added
n the future.

### Code formatting

All code should be formatted as so:

```shell
clang-format -i <source_file>`
```

### Running Tests

All tests run **on hardware** and expect to have the appropriate real-time
clocks attached. See platformio.ini to learn the necessary GPIO pins for
your hardware. Run the tests as follows:

```sh
make test
```
