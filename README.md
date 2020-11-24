This is a fork of Adafruit's [RTCLib](https://github.com/adafruit/RTClib) - which,
in turn, is a fork of JeeLab's real time clock library for Arduino.

The primary reason for this fork is to port to
[ESP-IDF](https://docs.espressif.com/projects/esp-idf).

Additionally:

1. Return values (usually boolean) indicate success/failure
   of most calls.
2. API requires C++11.
3. Bug fixes.
4. Unit tests.
5. PulseView protocol decoders.

This is currently a [PlatformIO](https://platformio.org/) library.

This library depends on the [i2clib](https://github.com/cmumford/i2clib) library.

Here is a simple example of retrieving the current time
from the RTC:

```c++
#include <i2clib/master.h>
#include <rtclib/datetime.h>
#include <rtclib/ds3231.h>

using namespace rtc;

// Initialize the I2C bus - this needs to be done only once.
i2c::Master::Initialize(I2C_PORT, I2C_SDA_GPIO,
                        I2C_CLK_GPIO, I2C_CLOCK_SPEED);

// Create an I2C master for the RTC object so that
// it can communicate via the I2C bus.
i2c::Master master(I2C_PORT, /*i2c_mutex=*/nullptr);

// Allocate an DS3231 RTC object - giving it the I2C master.
DS3231 rtc(std::move(master));
rtc.begin();

// Retrieve the current time.
DateTime now;
rtc.now(&now);
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
