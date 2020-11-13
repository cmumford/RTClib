/*!
  @section license License

  Original library by JeeLabs https://jeelabs.org/pub/docs/rtclib/, released to
  the public domain.

  This version: MIT (see LICENSE)
*/

#include <RTClib.h>

/**************************************************************************/
/*!
    @brief  Add a TimeSpan to the DateTime object
    @param span TimeSpan object
    @return New DateTime object with span added to it.
*/
/**************************************************************************/
DateTime DateTime::operator+(const TimeSpan& span) {
  return DateTime(unixtime() + span.totalseconds());
}

/**************************************************************************/
/*!
    @brief  Subtract a TimeSpan from the DateTime object
    @param span TimeSpan object
    @return New DateTime object with span subtracted from it.
*/
/**************************************************************************/
DateTime DateTime::operator-(const TimeSpan& span) {
  return DateTime(unixtime() - span.totalseconds());
}

/**************************************************************************/
/*!
    @brief  Subtract one DateTime from another

    @note Since a TimeSpan cannot be negative, the subtracted DateTime
        should be less (earlier) than or equal to the one it is
        subtracted from.

    @param right The DateTime object to subtract from self (the left object)
    @return TimeSpan of the difference between DateTimes.
*/
/**************************************************************************/
TimeSpan DateTime::operator-(const DateTime& right) {
  return TimeSpan(unixtime() - right.unixtime());
}

/**************************************************************************/
/*!
    @author Anton Rieutskyi
    @brief  Test if one DateTime is less (earlier) than another.
    @warning if one or both DateTime objects are invalid, returned value is
        meaningless
    @see use `isValid()` method to check if DateTime object is valid
    @param right Comparison DateTime object
    @return True if the left DateTime is earlier than the right one,
        false otherwise.
*/
/**************************************************************************/
bool DateTime::operator<(const DateTime& right) const {
  return (yOff + 2000U < right.year() ||
          (yOff + 2000U == right.year() &&
           (m < right.month() ||
            (m == right.month() &&
             (d < right.day() ||
              (d == right.day() &&
               (hh < right.hour() ||
                (hh == right.hour() &&
                 (mm < right.minute() ||
                  (mm == right.minute() && ss < right.second()))))))))));
}

/**************************************************************************/
/*!
    @author Anton Rieutskyi
    @brief  Test if two DateTime objects are equal.
    @warning if one or both DateTime objects are invalid, returned value is
        meaningless
    @see use `isValid()` method to check if DateTime object is valid
    @param right Comparison DateTime object
    @return True if both DateTime objects are the same, false otherwise.
*/
/**************************************************************************/
bool DateTime::operator==(const DateTime& right) const {
  return (right.year() == yOff + 2000U && right.month() == m &&
          right.day() == d && right.hour() == hh && right.minute() == mm &&
          right.second() == ss);
}

/**************************************************************************/
/*!
    @brief  Return a ISO 8601 timestamp as a `String` object.

    The generated timestamp conforms to one of the predefined, ISO
    8601-compatible formats for representing the date (if _opt_ is
    `TIMESTAMP_DATE`), the time (`TIMESTAMP_TIME`), or both
    (`TIMESTAMP_FULL`).

    @see The `toString()` method provides more general string formatting.

    @param opt Format of the timestamp
    @return Timestamp string, e.g. "2020-04-16T18:34:56".
*/
/**************************************************************************/
String DateTime::timestamp(timestampOpt opt) {
  char buffer[25];  // large enough for any DateTime, including invalid ones

  // Generate timestamp according to opt
  switch (opt) {
    case TIMESTAMP_TIME:
      // Only time
      sprintf(buffer, "%02d:%02d:%02d", hh, mm, ss);
      break;
    case TIMESTAMP_DATE:
      // Only date
      sprintf(buffer, "%u-%02d-%02d", 2000U + yOff, m, d);
      break;
    default:
      // Full
      sprintf(buffer, "%u-%02d-%02dT%02d:%02d:%02d", 2000U + yOff, m, d, hh, mm,
              ss);
  }
  return String(buffer);
}

/**************************************************************************/
/*!
    @brief  Create a new TimeSpan object in seconds
    @param seconds Number of seconds
*/
/**************************************************************************/
TimeSpan::TimeSpan(int32_t seconds) : _seconds(seconds) {}

/**************************************************************************/
/*!
    @brief  Create a new TimeSpan object using a number of
   days/hours/minutes/seconds e.g. Make a TimeSpan of 3 hours and 45 minutes:
   new TimeSpan(0, 3, 45, 0);
    @param days Number of days
    @param hours Number of hours
    @param minutes Number of minutes
    @param seconds Number of seconds
*/
/**************************************************************************/
TimeSpan::TimeSpan(int16_t days, int8_t hours, int8_t minutes, int8_t seconds)
    : _seconds((int32_t)days * 86400L + (int32_t)hours * 3600 +
               (int32_t)minutes * 60 + seconds) {}

/**************************************************************************/
/*!
    @brief  Copy constructor, make a new TimeSpan using an existing one
    @param copy The TimeSpan to copy
*/
/**************************************************************************/
TimeSpan::TimeSpan(const TimeSpan& copy) : _seconds(copy._seconds) {}

/**************************************************************************/
/*!
    @brief  Add two TimeSpans
    @param right TimeSpan to add
    @return New TimeSpan object, sum of left and right
*/
/**************************************************************************/
TimeSpan TimeSpan::operator+(const TimeSpan& right) {
  return TimeSpan(_seconds + right._seconds);
}

/**************************************************************************/
/*!
    @brief  Subtract a TimeSpan
    @param right TimeSpan to subtract
    @return New TimeSpan object, right subtracted from left
*/
/**************************************************************************/
TimeSpan TimeSpan::operator-(const TimeSpan& right) {
  return TimeSpan(_seconds - right._seconds);
}
