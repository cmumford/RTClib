/*!
  @section license License

  Original library by JeeLabs https://jeelabs.org/pub/docs/rtclib/, released to
  the public domain.

  This version: MIT (see LICENSE)
*/

#include <RTClib.h>

#include <cstdint>

namespace rtc {

/** Number of microseconds reported by micros() per "true" (calibrated) second.
 */
uint32_t Micros::microsPerSecond = 1000000;

/** The timing logic is identical to RTC_Millis. */
uint32_t Micros::lastMicros;
uint32_t Micros::lastUnix;

namespace {

uint32_t microsSinceStart() {
  // TODO: Implement me.
  return 0;
}

}  // anonymous namespace

/**************************************************************************/
/*!
    @brief  Set the current date/time of the RTC_Micros clock.
    @param dt DateTime object with the desired date and time
*/
/**************************************************************************/
void Micros::adjust(const DateTime& dt) {
  lastMicros = microsSinceStart();
  lastUnix = dt.unixtime();
}

/**************************************************************************/
/*!
    @brief  Adjust the RTC_Micros clock to compensate for system clock drift
    @param ppm Adjustment to make
*/
/**************************************************************************/
// A positive adjustment makes the clock faster.
void Micros::adjustDrift(int ppm) {
  microsPerSecond = 1000000 - ppm;
}

/**************************************************************************/
/*!
    @brief  Get the current date/time from the RTC_Micros clock.
    @return DateTime object containing the current date/time
*/
/**************************************************************************/
DateTime Micros::now() {
  uint32_t elapsedSeconds = (microsSinceStart() - lastMicros) / microsPerSecond;
  lastMicros += elapsedSeconds * microsPerSecond;
  lastUnix += elapsedSeconds;
  return lastUnix;
}

}  // namespace rtc