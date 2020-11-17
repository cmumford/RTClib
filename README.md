This is a fork of Adafruit's [RTCLib](https://github.com/adafruit/RTClib) - which,
in turn, is a fork of JeeLab's real time clock library for Arduino.

The primary reason for this fork is to port to
[ESP-IDF](https://docs.espressif.com/projects/esp-idf).

Addionally:

1. Return values (usually boolean) indicate success/failure
   of most calls.
2. Moved to C++11.
3. Bug fixes.
4. Unit tests.

This is currently a [PlatformIO](https://platformio.org/) library.

The implementation is mostly platform independent, as platform specific
code is restricted to a few files. Support for Arduino might be added
back in the future.

## Code formatting and clang-format

```shell
clang-format -i <source_file>`
```
