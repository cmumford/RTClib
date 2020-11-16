/**************************************************************************/
/*!
  Original library by JeeLabs http://news.jeelabs.org/code/, released to the
  public domain

  License: MIT (see LICENSE)

  This is a fork of JeeLab's fantastic real time clock library for Arduino.

  For details on using this library with an RTC module like the DS1307, PCF8523,
  or DS3231, see the guide at:
  https://learn.adafruit.com/ds1307-real-time-clock-breakout-board-kit/overview

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
*/
/**************************************************************************/

#pragma once

#define SECONDS_PER_DAY 86400L  ///< 60 * 60 * 24
#define SECONDS_FROM_1970_TO_2000 \
  946684800  ///< Unixtime for 2000-01-01 00:00:00, useful for initialization
