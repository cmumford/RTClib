#
# This file is part of the libsigrokdecode project.
#
# Copyright (C) 2012-2020 Uwe Hermann <uwe@hermann-uwe.de>
# Copyright (C) 2013 Matt Ranostay <mranostay@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#

import re
import sigrokdecode as srd
from common.srdhelper import bcd2int, SrdIntEnum

regs = (
    'Seconds', 'Minutes', 'Hours', 'Day', 'Date', 'Month', 'Year',
    'Alarm_1_Secs', 'Alarm_1_Mins', 'Alarm_1_Hours', 'Alarm_1_Day',
    'Alarm_2_Mins', 'Alarm_2_Hours', 'Alarm_2_Day_Date', 'Control', 'Status',
    'AOF', 'Temp_MSB', 'Temp_LSB',
)

bits = (
    'Clock halt', 'Seconds', 'Reserved', 'Minutes', '12/24 hours', 'AM/PM',
    'Hours', 'Day', 'Date', 'Month', 'Year', 'OUT', 'SQWE', 'RS', 'RAM',
    'ALM1', 'ALM2', 'INTCN', 'CONV', 'BBSQW', 'EOSC', 'A1F', 'A2F', 'Busy',
    'EN32K', 'OSF', 'AE', 'DY/DT', 'Century', 'Temp_MSB', 'Temp_LSB', 'AOF',
)

rates = {
    0b00: '1Hz',
    0b01: '1kHz',
    0b10: '4kHz',
    0b11: '8kHz',
}

DS3231_I2C_ADDRESS = 0x68


def regs_and_bits():
    l = [('reg_' + r.lower(), r + ' register') for r in regs]
    l += [('bit_' + re.sub(r'\/| ', '_', b).lower(), b + ' bit') for b in bits]
    return tuple(l)


a = ['REG_' + r.upper() for r in regs] + \
    ['BIT_' + re.sub('\/| ', '_', b).upper() for b in bits] + \
    ['READ_DATE_TIME', 'WRITE_DATE_TIME', 'READ_REG', 'WRITE_REG', 'WARNING']
Ann = SrdIntEnum.from_list('Ann', a)


class DateTime(object):
    days_of_week = (
        'Sunday', 'Monday', 'Tuesday', 'Wednesday',
        'Thursday', 'Friday', 'Saturday',
    )

    def __init__(self):
        self.hours = -1
        self.minutes = -1
        self.seconds = -1
        self.days = -1
        self.date = -1
        self.months = -1
        self.years = -1

    def __str__(self):
        # TODO: Handle read/write of only parts of these items.
        return '%s, %02d.%02d.%4d %02d:%02d:%02d' % (
            DateTime.days_of_week[self.days - 1], self.date, self.months,
            self.years, self.hours, self.minutes, self.seconds)

    def isEmpty(self):
        return self.hours == -1 and \
            self.minutes == -1 and \
            self.seconds == -1 and \
            self.days == -1 and \
            self.date == -1 and \
            self.months == -1 and \
            self.years == -1


class Alarm1(object):
    def __init__(self):
        self.m1 = -1
        self.m2 = -1
        self.m3 = -1
        self.m4 = -1
        self.seconds = -1
        self.minutes = -1
        self.hours = -1
        self.dow = -1
        self.dom = -1
        self.dy_dt = -1

    def __str__(self):
        when = [self.m1, self.m2, self.m3, self.m4]
        if when == [1, 1, 1, 1]:
            str = 'once/sec.'
        elif when == [1, 1, 1, 0]:
            str = 'when SS=%02d' % self.seconds
        elif when == [1, 1, 0, 0]:
            str = 'when MM:SS=%02d:%02d' % (self.minutes, self.seconds)
        elif when == [1, 0, 0, 0]:
            str = 'when HH:MM:SS=%02d:%02d:%02d' % (
                self.hours, self.minutes, self.seconds)
        elif when == [0, 0, 0, 0]:
            if self.dy_dt:
                str = 'when DY/HH:MM:SS=%d/%02d:%02d:%02d' % (
                    self.dow, self.hours, self.minutes, self.seconds)
            else:
                str = 'when DT/HH:MM:SS=%02d/%02d:%02d:%02d' % (
                    self.dom, self.hours, self.minutes, self.seconds)
        else:
            str = ' (incomplete register set)'
        return str

    def isEmpty(self):
        return self.seconds == -1 and \
            self.minutes == -1 and \
            self.hours == -1 and \
            self.dow == -1 and \
            self.dom == -1


class Alarm2(object):
    def __init__(self):
        self.m2 = -1
        self.m3 = -1
        self.m4 = -1
        self.minutes = -1
        self.hours = -1
        self.dow = -1
        self.dom = -1
        self.dy_dt = -1

    def isEmpty(self):
        return self.minutes == -1 and \
            self.hours == -1 and \
            self.dow == -1 and \
            self.dom == -1

    def __str__(self):
        when = [self.m2, self.m3, self.m4]
        if when == [1, 1, 1]:
            str = 'once/min.'
        elif when == [1, 1, 0]:
            str = 'when MM=%02d' % self.minutes
        elif when == [1, 0, 0]:
            str = 'when HH:MM=%02d:%02d' % (self.hours, self.minutes)
        elif when == [0, 0, 0]:
            if self.dy_dt:
                str = 'when DY/HH:MM=%d/%02d:%02d' % (
                    self.dow, self.hours, self.minutes)
            else:
                str = 'when DT/HH:MM=%02d/%02d:%02d' % (
                    self.dom, self.hours, self.minutes)
        else:
            str = 'DD/DT/HH:MM=%d/%d/%02d:%02d' % (
                self.dom, self.dow, self.hours, self.minutes)
        return str


class Status(object):
    pass


class Control(object):
    pass


class Temperature(object):
    def __init__(self):
        self.msb = None
        self.lsb = None

    def __str__(self):
        if self.msb is None:
            return 'LSB only: %.2f °C' % (0.25 * self.lsb)
        elif self.lsb is None:
            return 'MSB only: %.2f °C' % (self.msb)
        else:
            return '%.1f °C' % (0.25 * self.lsb + self.msb)


class Decoder(srd.Decoder):
    api_version = 3
    id = 'ds3231'
    name = 'DS3231'
    longname = 'Maxim DS3231'
    desc = 'Realtime clock module protocol.'
    license = 'gplv2+'
    inputs = ['i2c']
    outputs = []
    tags = ['Clock/timing', 'IC']
    annotations = regs_and_bits() + (
        ('read_date_time', 'Read op'),
        ('write_date_time', 'Write op'),
        ('read_reg', 'Register read'),
        ('write_reg', 'Register write'),
        ('warning', 'Warning'),
    )
    annotation_rows = (
        ('bits', 'Bits', Ann.prefixes('BIT_')),
        ('regs', 'Registers', Ann.prefixes('REG_')),
        ('date_time', 'Date/time', Ann.prefixes('READ_ WRITE_')),
        ('warnings', 'Warnings', (Ann.WARNING,)),
    )

    def __init__(self):
        self.state = 'IDLE'
        self.time = DateTime()
        self.bits = []
        self.registers_accessed = []
        self.status = None
        self.control = None
        self.temp = None
        self.alarm1 = Alarm1()
        self.alarm2 = Alarm2()

    def reset(self):
        # Do the initialization in the constructor method to avoid Python
        # linter warnings about using undefined member variables.
        self.__init__()

    def start(self):
        self.out_ann = self.register(srd.OUTPUT_ANN)

    def putx(self, data):
        self.put(self.ss, self.es, self.out_ann, data)

    def putd(self, bit1, bit2, data):
        self.put(self.bits[bit1][1], self.bits[bit2][2], self.out_ann, data)

    def putr(self, bit):
        self.put(self.bits[bit][1], self.bits[bit][2], self.out_ann,
                 [Ann.BIT_RESERVED, ['Reserved bit', 'Reserved', 'Rsvd', 'R']])

    def handle_reg_0x00(self, b):  # Seconds
        self.putr(7)
        self.time.seconds = bcd2int(b & 0x7f)
        self.putd(
            6, 0, [
                Ann.BIT_SECONDS, [
                    'Second: %d' %
                    self.time.seconds, 'Secs: %d' %
                    self.time.seconds, 'S']])
        self.putd(7, 0, [Ann.REG_SECONDS, ['Seconds', 'Secs', 'S']])

    def handle_reg_0x01(self, b):
        self.putr(7)
        self.time.minutes = bcd2int(b & 0x7f)
        self.putd(
            6, 0, [
                Ann.BIT_MINUTES, [
                    'Minute: %d' %
                    self.time.minutes, 'M: %d' %
                    self.time.minutes, 'M']])
        self.putd(7, 0, [Ann.REG_MINUTES, ['Minutes', 'Mins', 'M']])

    def handle_reg_0x02(self, b):
        self.putr(7)
        self.time.hours = bcd2int(b & 0x1f)
        self.putd(
            4, 0, [
                Ann.BIT_HOURS, [
                    'Hour: %d' %
                    self.time.hours, 'H: %d' %
                    self.time.hours, 'H']])
        twentyfour = 1 if bcd2int(b & 0b01000000) else 0
        PM = 1 if bcd2int(b & 0b00100000) else 0
        if twentyfour:
            self.putd(
                6, 6, [
                    Ann.BIT_12_24_HOURS, [
                        '12-hour mode', '12 hr. mode', '12']])
        else:
            self.putd(
                6, 6, [
                    Ann.BIT_12_24_HOURS, [
                        '24-hour mode', '24 hr. mode', '24']])
        if PM:
            self.putd(5, 5, [Ann.BIT_AM_PM, ['PM', 'PM', 'P']])
        else:
            self.putd(5, 5, [Ann.BIT_AM_PM, ['AM', 'AM', 'A']])
        self.putd(7, 0, [Ann.REG_HOURS, ['Hours', 'Hrs', 'H']])

    def handle_reg_0x03(self, b):  # Day of week
        self.putr(7)
        self.putr(6)
        self.putr(5)
        self.putr(4)
        self.putr(3)
        self.time.days = bcd2int(b & 0b111)
        self.putd(
            2, 0, [
                Ann.BIT_DAY, [
                    'Day of week: %d' %
                    self.time.days, 'DOW:%d' %
                    self.time.days, 'D']])
        self.putd(7, 0, [Ann.REG_DAY, ['Day of week', 'DOW', 'D']])

    def handle_reg_0x04(self, b):
        self.putr(7)
        self.putr(6)
        self.time.date = bcd2int(b & 0x7f)
        self.putd(
            5, 0, [
                Ann.BIT_DATE, [
                    'Date: %d' %
                    self.time.date, 'D: %d' %
                    self.time.date, 'D']])
        self.putd(7, 0, [Ann.REG_DATE, ['Date', 'D', 'D']])

    def handle_reg_0x05(self, b):
        self.putr(6)
        self.putr(5)
        self.time.months = bcd2int(b & 0x0f)
        self.putd(
            4, 0, [
                Ann.BIT_MONTH, [
                    'Month: %d' %
                    self.time.months, 'M: %d' %
                    self.time.months, 'M']])
        c = bcd2int(b & 0x80)
        self.putd(
            7, 7, [
                Ann.BIT_CENTURY, [
                    'Century: %d' %
                    c, 'C: %d' %
                    c, 'C']])
        self.putd(7, 0, [Ann.REG_MONTH, ['Month', 'Mon.', 'M']])

    def handle_reg_0x06(self, b):
        self.time.years = bcd2int(b & 0xff)
        self.putd(
            7, 0, [
                Ann.BIT_YEAR, [
                    'Year: %d' %
                    self.time.years, 'Y: %d' %
                    self.time.years, 'Y']])
        self.putd(7, 0, [Ann.REG_YEAR, ['Year', 'Yr.', 'Y']])

    def handle_reg_0x07(self, b):  # Alarm 1 seconds
        self.alarm1.m1 = bcd2int(b & 0b10000000) >> 7
        self.putd(
            7, 7, [
                Ann.BIT_AE, [
                    'A1M1: %d' %
                    self.alarm1.m1, 'A1: %d' %
                    self.alarm1.m1, 'A1']])
        self.alarm1.seconds = bcd2int(b & 0b01111111)
        self.putd(
            6, 0, [
                Ann.BIT_SECONDS, [
                    'Seconds: %d' %
                    self.alarm1.seconds, 'S: %d' %
                    self.alarm1.seconds, 'S']])
        self.putd(7, 0, [Ann.REG_ALARM_1_SECS, ['Seconds', 'Secs', 'S']])

    def handle_reg_0x08(self, b):  # Alarm 1 minutes
        self.alarm1.m2 = bcd2int(b & 0b10000000) >> 7
        self.putd(
            7, 7, [
                Ann.BIT_AE, [
                    'A1M2: %d' %
                    self.alarm1.m2, 'A2: %d' %
                    self.alarm1.m2, 'A2']])
        self.alarm1.minutes = bcd2int(b & 0b01111111)
        self.putd(
            6, 0, [
                Ann.BIT_MINUTES, [
                    'Minutes: %d' %
                    self.alarm1.minutes, 'MM: %d' %
                    self.alarm1.minutes, 'M']])
        self.putd(7, 0, [Ann.REG_ALARM_1_MINS, ['Minutes', 'Mins', 'M']])

    def handle_reg_0x09(self, b):  # Alarm 1 hours
        self.alarm1.m3 = bcd2int(b & 0b10000000) >> 7
        self.putd(
            7, 7, [
                Ann.BIT_AE, [
                    'A1M3: %d' %
                    self.alarm1.m3, 'A3: %d' %
                    self.alarm1.m3, 'A3']])
        self.alarm1.hours = bcd2int(b & 0b01111111)
        self.putd(
            6, 0, [
                Ann.BIT_HOURS, [
                    'Hours: %d' %
                    self.alarm1.hours, 'HH: %d' %
                    self.alarm1.hours, 'H']])
        self.putd(7, 0, [Ann.REG_ALARM_1_HOURS, ['Hours', 'Hrs.', 'H']])

    def handle_reg_0x0a(self, b):  # Alarm 1 day/date
        self.alarm1.m4 = bcd2int(b & 0b10000000) >> 7
        self.alarm1.dy_dt = 1 if bcd2int(b & 0b01000000) else 0
        date = bcd2int(b & 0b00111111)
        self.putd(
            7, 7, [
                Ann.BIT_AE, [
                    'A1M4: %d' %
                    self.alarm1.m4, 'A1: %d' %
                    self.alarm1.m4, 'A4']])
        if self.alarm1.dy_dt:
            self.putd(6, 6, [Ann.BIT_DY_DT, ['Day', 'Day', 'D']])
            self.putd(
                5, 0, [
                    Ann.BIT_DAY, [
                        'Day of week: %d' %
                        date, 'DOW: %d' %
                        date, 'D']])
            self.alarm1.dow = date
        else:
            self.putd(6, 6, [Ann.BIT_DY_DT, ['Date', 'Date', 'D']])
            self.putd(
                5, 0, [
                    Ann.BIT_DATE, [
                        'Day of month: %d' %
                        date, 'DOM: %d' %
                        date, 'D']])
            self.alarm1.dom = date
        self.putd(7, 0, [Ann.REG_ALARM_1_DAY, ['Day/Date', 'DT', 'D']])

    def handle_reg_0x0b(self, b):  # Alarm 2 minutes
        self.alarm2.m2 = bcd2int(b & 0b10000000) >> 7
        self.putd(
            7, 7, [
                Ann.BIT_AE, [
                    'A2M2: %d' %
                    self.alarm2.m2, 'A2: %d' %
                    self.alarm2.m2, 'A2']])
        self.alarm2.minutes = bcd2int(b & 0b01111111)
        self.putd(
            6, 0, [
                Ann.BIT_MINUTES, [
                    'Minutes: %d' %
                    self.alarm2.minutes, 'Min: %d' %
                    self.alarm2.minutes, 'M']])
        self.putd(7, 0, [Ann.REG_ALARM_2_MINS, ['Minutes', 'Mins', 'M']])

    def handle_reg_0x0c(self, b):  # Alarm 2 hours
        self.alarm2.m3 = bcd2int(b & 0b10000000) >> 7
        self.putd(
            7, 7, [
                Ann.BIT_AE, [
                    'A2M3: %d' %
                    self.alarm2.m3, 'A3: %d' %
                    self.alarm2.m3, 'A3']])
        t4 = 1 if bcd2int(b & 0b01000000) else 0
        am = 1 if bcd2int(b & 0b00100000) else 0
        self.alarm2.hours = bcd2int(b & 0b00011111)
        self.putd(
            6, 6, [
                Ann.BIT_12_24_HOURS, [
                    '12/24: %d' %
                    t4, '24: %d' %
                    t4, '24']])
        self.putd(
            5, 5, [
                Ann.BIT_AM_PM, [
                    'AM/PM: %d' %
                    am, 'AM: %d' %
                    am, 'AM']])
        self.putd(
            4, 0, [
                Ann.BIT_HOURS, [
                    'Hour: %d' %
                    self.alarm2.hours, 'H: %d' %
                    self.alarm2.hours, 'H']])
        self.putd(7, 0, [Ann.REG_ALARM_2_HOURS, ['Hours', 'Hrs', 'H']])

    def handle_reg_0x0d(self, b):  # Alarm 2 Day/Date
        self.alarm2.m4 = bcd2int(b & 0b10000000) >> 7
        self.putd(
            7, 7, [
                Ann.BIT_AE, [
                    'A2M4: %d' %
                    self.alarm2.m4, 'A4: %d' %
                    self.alarm2.m4, 'A4']])
        self.alarm2.dy_dt = 1 if bcd2int(b & 0b01000000) else 0
        date = bcd2int(b & 0b00111111)
        if self.alarm2.dy_dt:
            self.putd(6, 6, [Ann.BIT_DY_DT, ['Day of week', 'DT', 'D']])
            self.putd(
                5, 0, [
                    Ann.BIT_DAY, [
                        'Day of week: %d' %
                        date, 'DOW: %d' %
                        date, 'D']])
            self.alarm2.dow = date
        else:
            self.putd(6, 6, [Ann.BIT_DY_DT, ['Day of month', 'DT', 'D']])
            self.putd(
                5, 0, [
                    Ann.BIT_DATE, [
                        'Day of month: %d' %
                        date, 'DOM: %d' %
                        date, 'D']])
            self.alarm2.dom = date
        self.putd(7, 0, [Ann.REG_ALARM_2_DAY_DATE, ['Day/Date', 'Dy/Dt', 'D']])

    def handle_reg_0x0e(self, b):  # Control register
        alarm1 = 1 if (b & (1 << 0)) else 0
        alarm2 = 1 if (b & (1 << 1)) else 0
        intcn = 1 if (b & (1 << 2)) else 0
        rs = ((b & 0b11000) >> 3)
        conv = 1 if (b & (1 << 5)) else 0
        bbsqw = 1 if (b & (1 << 6)) else 0
        eosc = 1 if (b & (1 << 7)) else 0

        if rs == 0b00:
            rsn = '%d: (1Hz)' % rs
        elif rs == 0b01:
            rsn = '%d (1kHz)' % rs
        elif rs == 0b10:
            rsn = '%d (4kHz)' % rs
        elif rs == 0b11:
            rsn = '%d (8kHz)' % rs

        if intcn == 0:
            intcnmsg = 'Alarm mode'
        else:
            intcnmsg = 'SqWave disabled'

        if eosc == 0:
            eoscmsg = 'Started'
        else:
            eoscmsg = 'Off when on Vbat'

        self.putd(
            0, 0, [
                Ann.BIT_ALM1, [
                    'Alarm 1: %d' %
                    alarm1, 'ALM1: %d' %
                    alarm1, 'A']])
        self.putd(
            1, 1, [
                Ann.BIT_ALM2, [
                    'Alarm 2: %d' %
                    alarm2, 'ALM2: %d' %
                    alarm2, 'A']])
        self.putd(
            2, 2, [
                Ann.BIT_INTCN, [
                    'Interrupt control: %d (%s)' %
                    (intcn, intcnmsg), 'INTCN: %d' %
                    intcn, 'I']])
        self.putd(4, 3, [Ann.BIT_RS, ['RS: %s' % rsn, 'RS: %d' % rs, 'R']])
        self.putd(
            5, 5, [
                Ann.BIT_CONV, [
                    'CONV: %d' %
                    conv, 'CONV: %d' %
                    conv, 'C']])
        self.putd(
            6, 6, [
                Ann.BIT_BBSQW, [
                    'BBSQW: %d' %
                    bbsqw, 'BBSQ: %d' %
                    bbsqw, 'S']])
        self.putd(
            7, 7, [
                Ann.BIT_EOSC, [
                    'Enable oscillator: %d (%s)' %
                    (eosc, eoscmsg), 'EOSC: %d' %
                    eosc, 'E']])
        self.putd(7, 0, [Ann.REG_CONTROL, ['Control', 'Ctrl', 'C']])
        self.control = Control()

    def handle_reg_0x0f(self, b):  # Status
        self.putr(6)
        self.putr(5)
        self.putr(4)
        a1f = 1 if (b & (1 << 0)) else 0
        a2f = 1 if (b & (1 << 1)) else 0
        BSY = 1 if (b & (1 << 2)) else 0
        EN32kHz = 1 if (b & (1 << 3)) else 0
        osf = 1 if (b & (1 << 7)) else 0

        if osf == 1:
            osfmsg = '[is/was] Stopped'
        else:
            osfmsg = 'Not stopped'

        self.putd(
            0, 0, [
                Ann.BIT_A1F, [
                    'Alarm flag 1: %d' %
                    a1f, 'AF1: %d' %
                    a1f, 'A1']])
        self.putd(
            1, 1, [
                Ann.BIT_A2F, [
                    'Alarm flag 2: %d' %
                    a2f, 'AF2: %d' %
                    a2f, 'A2']])
        self.putd(
            2, 2, [
                Ann.BIT_BUSY, [
                    'Busy: %d' %
                    BSY, 'AF2: %d' %
                    BSY, 'B']])
        self.putd(
            3, 3, [
                Ann.BIT_EN32K, [
                    '32K: %d' %
                    EN32kHz, '32: %d' %
                    EN32kHz, '3']])
        self.putd(
            7, 7, [
                Ann.BIT_OSF, [
                    'OSF: %d (%s)' %
                    (osf, osfmsg), 'OSF: %d' %
                    osf, 'O']])
        self.putd(7, 0, [Ann.REG_STATUS, ['Status', 'St', 'S']])
        self.status = Status()

    def handle_reg_0x10(self, b):  # Aging offset.
        temp = b
        self.putd(
            7, 0, [
                Ann.BIT_AOF, [
                    'Aging offset: %d' %
                    temp, 'Aging: %d' %
                    temp, 'A']])
        self.putd(7, 0, [Ann.REG_AOF, ['Aging offset', 'AOF', 'A']])

    def handle_reg_0x11(self, b):
        temp = b
        self.putd(
            7, 0, [
                Ann.BIT_TEMP_MSB, [
                    'Tmsb: %d °C' %
                    temp, 'Tm: %d' %
                    temp, 'T']])
        self.putd(7, 0, [Ann.REG_TEMP_MSB, ['Temp (MSB)', 'Tmsb', 'T']])
        if not self.temp:
            self.temp = Temperature()
        self.temp.msb = temp

    def handle_reg_0x12(self, b):
        temp = 0.25 * ((b & 0b11000000) >> 6)
        self.putr(5)
        self.putr(4)
        self.putr(3)
        self.putr(2)
        self.putr(1)
        self.putr(0)
        self.putd(
            7, 6, [
                Ann.BIT_TEMP_LSB, [
                    'Tlsb: %.2f °C' %
                    temp, 'Tl: %.2f' %
                    temp, 'T']])
        self.putd(7, 0, [Ann.REG_TEMP_LSB, ['Temp (LSB)', 'Tlsb', 'T']])
        if not self.temp:
            self.temp = Temperature()
        self.temp.lsb = temp

    def output_datetime(self, cls, rw):
        # TODO: Handle read/write of only parts of these items.
        msg = ''

        if self.status:
            if msg != '':
                msg = msg + ', '
            msg = 'Status register'
        if self.control:
            if msg != '':
                msg = msg + ', '
            msg = 'Control register'
        if self.temp:
            if msg != '':
                msg = msg + ', '
            msg += 'Temp: %s' % self.temp
        if not self.time.isEmpty():
            if msg != '':
                msg = msg + ', '
            msg += 'Date/Time: %s' % self.time
        if not self.alarm1.isEmpty():
            if msg != '':
                msg = msg + ', '
            msg += 'Alarm 1: %s' % self.alarm1
        if not self.alarm2.isEmpty():
            if msg != '':
                msg = msg + ', '
            msg += 'Alarm 2: %s' % self.alarm2

        if msg == '':
            msg = '??'

        self.put(self.ss_block, self.es, self.out_ann,
                 [cls, ['%s %s' % (rw, msg)]])

    def handle_reg(self, b):
        r = self.reg
        fn = getattr(self, 'handle_reg_0x%02x' % r)
        fn(b)
        # Honor address auto-increment feature of the DS3231. When the
        # address reaches 0x13, it will wrap around to address 0.
        self.reg += 1
        if self.reg > 0x13:
            self.reg = 0

    def is_correct_chip(self, addr):
        if addr == DS3231_I2C_ADDRESS:
            return True
        self.put(
            self.ss_block, self.es, self.out_ann, [
                Ann.WARNING, [
                    'Ignoring non-DS3231 data (slave 0x%02X)' %
                    addr]])
        return False

    def decode(self, ss, es, data):
        cmd, databyte = data

        # Collect the 'BITS' packet, then return. The next packet is
        # guaranteed to belong to these bits we just stored.
        if cmd == 'BITS':
            self.bits = databyte
            return

        # Store the start/end samples of this I²C packet.
        self.ss, self.es = ss, es

        # State machine.
        if self.state == 'IDLE':
            # Wait for an I²C START condition.
            if cmd != 'START':
                return
            self.reset()
            self.state = 'GET SLAVE ADDR'
            self.ss_block = ss
        elif self.state == 'GET SLAVE ADDR':
            # Wait for an address write operation.
            if cmd != 'ADDRESS WRITE':
                return
            if not self.is_correct_chip(databyte):
                self.state = 'IDLE'
                return
            self.state = 'GET REG ADDR'
        elif self.state == 'GET REG ADDR':
            # Wait for a data write (master selects the slave register).
            if cmd != 'DATA WRITE':
                return
            self.reg = databyte
            self.state = 'WRITE RTC REGS'
        elif self.state == 'WRITE RTC REGS':
            # If we see a Repeated Start here, it's an RTC read.
            if cmd == 'START REPEAT':
                self.state = 'READ RTC REGS'
                return
            # Otherwise: Get data bytes until a STOP condition occurs.
            if cmd == 'DATA WRITE':
                self.handle_reg(databyte)
            elif cmd == 'STOP':
                self.output_datetime(Ann.WRITE_DATE_TIME, 'Written')
                self.state = 'IDLE'
            else:
                pass  # TODO
        elif self.state == 'READ RTC REGS':
            # Wait for an address read operation.
            if cmd != 'ADDRESS READ':
                return
            if not self.is_correct_chip(databyte):
                self.state = 'IDLE'
                return
            self.state = 'READ RTC REGS2'
        elif self.state == 'READ RTC REGS2':
            if cmd == 'DATA READ':
                self.handle_reg(databyte)
            elif cmd == 'STOP':
                self.output_datetime(Ann.READ_DATE_TIME, 'Read')
                self.state = 'IDLE'
            else:
                pass  # TODO?
        else:
            pass
