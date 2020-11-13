/*!
  @section license License

  Original library by JeeLabs https://jeelabs.org/pub/docs/rtclib/, released to
  the public domain.

  This version: MIT (see LICENSE)
*/

#include <RTClib.h>

/** Alignment between the milis() timescale and the Unix timescale. These
  two variables are updated on each call to now(), which prevents
  rollover issues. Note that lastMillis is **not** the millis() value
  of the last call to now(): it's the millis() value corresponding to
  the last **full second** of Unix time. */
uint32_t RTC_Millis::lastMillis;
uint32_t RTC_Millis::lastUnix;

namespace {

uint32_t millisSinceStart() {
  // TODO: Implement me.
  return 0;
}

}  // anonymous namespace

/**************************************************************************/
/*!
    @brief  Set the current date/time of the RTC_Millis clock.
    @param dt DateTime object with the desired date and time
*/
/**************************************************************************/
void RTC_Millis::adjust(const DateTime& dt) {
  lastMillis = millisSinceStart();
  lastUnix = dt.unixtime();
}

/**************************************************************************/
/*!
    @brief  Return a DateTime object containing the current date/time.
            Note that computing (millis() - lastMillis) is rollover-safe as long
            as this method is called at least once every 49.7 days.
    @return DateTime object containing current time
*/
/**************************************************************************/
DateTime RTC_Millis::now() {
  uint32_t elapsedSeconds = (millisSinceStart() - lastMillis) / 1000;
  lastMillis += elapsedSeconds * 1000;
  lastUnix += elapsedSeconds;
  return lastUnix;
}
