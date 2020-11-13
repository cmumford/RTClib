/*!
  @section license License

  Original library by JeeLabs https://jeelabs.org/pub/docs/rtclib/, released to
  the public domain.

  This version: MIT (see LICENSE)
*/

#include <RTClib.h>

/** Number of microseconds reported by micros() per "true" (calibrated) second.
 */
uint32_t RTC_Micros::microsPerSecond = 1000000;

/** The timing logic is identical to RTC_Millis. */
uint32_t RTC_Micros::lastMicros;
uint32_t RTC_Micros::lastUnix;

/**************************************************************************/
/*!
    @brief  Set the current date/time of the RTC_Micros clock.
    @param dt DateTime object with the desired date and time
*/
/**************************************************************************/
void RTC_Micros::adjust(const DateTime& dt) {
  lastMicros = micros();
  lastUnix = dt.unixtime();
}

/**************************************************************************/
/*!
    @brief  Adjust the RTC_Micros clock to compensate for system clock drift
    @param ppm Adjustment to make
*/
/**************************************************************************/
// A positive adjustment makes the clock faster.
void RTC_Micros::adjustDrift(int ppm) {
  microsPerSecond = 1000000 - ppm;
}

/**************************************************************************/
/*!
    @brief  Get the current date/time from the RTC_Micros clock.
    @return DateTime object containing the current date/time
*/
/**************************************************************************/
DateTime RTC_Micros::now() {
  uint32_t elapsedSeconds = (micros() - lastMicros) / microsPerSecond;
  lastMicros += elapsedSeconds * microsPerSecond;
  lastUnix += elapsedSeconds;
  return lastUnix;
}
