##
# This file is part of the libsigrokdecode project.
##
# Copyright (C) 2012-2014 Uwe Hermann <uwe@hermann-uwe.de>
##
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
##
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
##
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
##

import sigrokdecode as srd
from common.srdhelper import bcd2int


def reg_list():
    l = []
    for i in range(8 + 1):
        l.append(('reg-0x%02x' % i, 'Register 0x%02x' % i))

    return tuple(l)


class Decoder(srd.Decoder):
    api_version = 3
    id = 'rtcds3231'
    name = 'RTC-DS3231'
    longname = 'DS3231 RTC'
    desc = 'Realtime clock module protocol.'
    license = 'gplv2+'
    inputs = ['i2c']
    outputs = []
    tags = ['Clock/timing']
    annotations = reg_list() + (
        ('read', 'Read'),
        ('write', 'Write'),
        ('bit-reserved', 'Reserved bit'),
        ('bit-vl', 'VL bit'),
        ('bit-century', 'Century bit'),
        ('reg-read', 'Register read'),
        ('reg-write', 'Register write'),
    )
    annotation_rows = (
        ('bits', 'Bits', tuple(range(0, 8 + 1)) + (11, 12, 13)),
        ('regs', 'Register accesses', (14, 15)),
        ('date-time', 'Date/time', (9, 10)),
    )
    registers = {
        0x0: 'TIME:SECS',
        0x1: 'TIME:MINS',
        0x2: 'TIME:HRS',
        0x3: 'TIME:DOW',
        0x4: 'TIME:DATE',
        0x5: 'TIME:MONTH',
        0x6: 'TIME:YEAR',
        0x7: 'ALM1:SECS',
        0x8: 'ALM1:MINS',
        0x9: 'ALM1:HRS',
        0xA: 'ALM1:DAY',
        0xB: 'ALM2:MINS',
        0xC: 'ALM2:HRS',
        0xD: 'ALM2:DAY',
        0xE: 'CONTROL',
        0xF: 'STATUS',
        0x11: 'TEMP:MSB',
        0x12: 'TEMP:LSB',
    }

    def __init__(self):
        self.state = 'IDLE'

    def reset(self):
        self.state = 'IDLE'

    def regName(reg_num):
        if reg_num in Decoder.registers:
            return Decoder.registers[reg_num]
        return 'Register: 0x%02x' % reg_num

    def start(self):
        self.out_ann = self.register(srd.OUTPUT_ANN)

    def putx(self, data):
        self.put(self.ss, self.es, self.out_ann, data)

    def putd(self, bit1, bit2, data):
        self.put(self.bits[bit1][1], self.bits[bit2][2], self.out_ann, data)

    def putr(self, bit):
        self.put(self.bits[bit][1], self.bits[bit][2], self.out_ann,
                 [11, ['Reserved bit', 'Reserved', 'Rsvd', 'R']])

    def handle_reg_0x00(self, b):  # Seconds
        self.putr(7)
        s = bcd2int(b & 0x7f)
        self.putd(6, 0, [4, ['Second: %d' % s, 'S: %d' % s, 'S']])

    def handle_reg_0x01(self, b):
        self.putr(7)
        m = bcd2int(b & 0x7f)
        self.putd(6, 0, [4, ['Minute: %d' % m, 'M: %d' % m, 'M']])

    def handle_reg_0x02(self, b):
        self.putr(7)
        h = bcd2int(b & 0x1f)
        self.putd(4, 0, [4, ['Hour: %d' % h, 'H: %d' % h, 'H']])
        tewntyfour = 1 if bcd2int(b & 0x20) else 0
        self.putd(6, 6, [4, ['12/24: %d' %
                             tewntyfour, '24: %d' % tewntyfour, '2']])
        am = 1 if bcd2int(b & 0x10) else 0
        self.putd(5, 5, [4, ['AM/PM: %d' % am, 'AM: %d' % am, 'A']])

    def handle_reg_0x03(self, b):  # Day o f week
        self.putr(7)
        self.putr(6)
        self.putr(5)
        self.putr(4)
        self.putr(3)
        dow = bcd2int(b & 0b111)
        self.putd(2, 0, [4, ['DOW: %d' % dow, 'DOW', 'D']])

    def handle_reg_0x04(self, b):
        self.putr(7)
        self.putr(6)
        d = bcd2int(b & 0x7f)
        self.putd(6, 0, [4, ['Date: %d' % d, 'D: %d' % d, 'D']])

    def handle_reg_0x05(self, b):
        self.putr(6)
        self.putr(5)
        m = bcd2int(b & 0x0f)
        self.putd(4, 0, [4, ['Month: %d' % m, 'M: %d' % m, 'M']])
        c = bcd2int(b & 0x80)
        self.putd(7, 7, [4, ['Century: %d' % c, 'C: %d' % c, 'C']])

    def handle_reg_0x06(self, b):
        y = bcd2int(b & 0xff)
        self.putd(7, 0, [4, ['Year: %d' % y, 'Y: %d' % y, 'Y']])

    def handle_reg_0x07(self, b):  # Alarm 1 seconds
        pass

    def handle_reg_0x08(self, b):  # Alarm 1 minutes
        pass

    def handle_reg_0x09(self, b):  # Alarm 1 hours
        pass

    def handle_reg_0x0a(self, b):  # Alarm 1 day/date
        pass

    def handle_reg_0x0b(self, b):  # Alarm 2 minutes
        a2m2 = bcd2int(b & 0b10000000) >> 7
        self.putd(7, 7, [4, ['A2M2: %d' % a2m2, 'A2: %d' % a2m2, 'A2']])
        m = bcd2int(b & 0b01111111)
        self.putd(6, 0, [4, ['Minutes: %d' % m, 'Min: %d' % m, 'M']])

    def handle_reg_0x0c(self, b):  # Alarm 2 hours
        a2m3 = bcd2int(b & 0b10000000) >> 7
        self.putd(7, 7, [4, ['A2M3: %d' % a2m3, 'A3: %d' % a2m3, 'A3']])
        t4 = 1 if bcd2int(b & 0b01000000) else 0
        self.putd(6, 6, [4, ['12/24: %d' % t4, '24: %d' % t4, '24']])
        am = 1 if bcd2int(b & 0b00100000) else 0
        self.putd(5, 5, [4, ['AM/PM: %d' % am, 'AM: %d' % am, 'AM']])
        hh = bcd2int(b & 0b00011111)
        self.putd(4, 0, [4, ['Hour: %d' % hh, 'H: %d' % hh, 'H']])

    def handle_reg_0x0d(self, b):  # Alarm 2 Day/Date
        pass

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

        self.putd(0, 0, [4, ['Alarm 1: %d' % alarm1, 'ALM1: %d' % alarm1, 'A']])
        self.putd(1, 1, [4, ['Alarm 2: %d' % alarm2, 'ALM2: %d' % alarm2, 'A']])
        self.putd(2, 2, [4, ['Interrupt control: %d (%s)' % (intcn, intcnmsg), 'INTCN: %d' % intcn, 'I']])
        self.putd(4, 3, [4, ['RS: %s' % rsn, 'RS: %d' % rs, 'R']])
        self.putd(5, 5, [4, ['CONV: %d' % conv, 'CONV: %d' % conv, 'C']])
        self.putd(6, 6, [4, ['BBSQW: %d' % bbsqw, 'BBSQ: %d' % bbsqw, 'S']])
        self.putd(7, 7, [4, ['Enable oscillator: %d (%s)' % (eosc, eoscmsg), 'EOSC: %d' % eosc, 'E']])

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

        self.putd(0, 0, [4, ['Alarm flag 1: %d' % a1f, 'AF1: %d' % a1f, 'A1']])
        self.putd(1, 1, [4, ['Alarm flag 2: %d' % a2f, 'AF2: %d' % a2f, 'A2']])
        self.putd(2, 2, [4, ['Busy: %d' % BSY, 'AF2: %d' % BSY, 'B']])
        self.putd(3, 3, [4, ['32K: %d' % EN32kHz, '32: %d' % EN32kHz, '3']])
        self.putd(7, 7, [4, ['OSF: %d (%s)' % (osf, osfmsg), 'OSF: %d' % osf, 'O']])

    def handle_reg_0x11(self, b):
        temp = b
        self.putd(7, 0, [4, ['Tmsb: %d °C' % temp, 'Tm: %d' % temp, 'T']])

    def handle_reg_0x12(self, b):
        temp = 0.25 * ((b & 0b11000000) >> 6)
        self.putd(7, 6, [4, ['Tlsb: %.2f °C' % temp, 'Tl: %.2f' % temp, 'T']])
        self.putr(5)
        self.putr(4)
        self.putr(3)
        self.putr(2)
        self.putr(1)
        self.putr(0)

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
            self.state = 'GET SLAVE ADDR'
            self.ss_block = ss
        elif self.state == 'GET SLAVE ADDR':
            # Wait for an address write operation.
            # TODO: We should only handle packets to the RTC slave (0xa2/0xa3).
            if cmd != 'ADDRESS WRITE':
                return
            self.state = 'GET REG ADDR'
        elif self.state == 'GET REG ADDR':
            # Wait for a data write (master selects the slave register).
            if cmd != 'DATA WRITE':
                return
            self.reg = databyte
            self.state = 'WRITE RTC REGS'
        elif self.state == 'WRITE RTC REGS':
            # If we see a Repeated Start here, it's probably an RTC read.
            if cmd == 'START REPEAT':
                self.state = 'READ RTC REGS'
                return
            # Otherwise: Get data bytes until a STOP condition occurs.
            if cmd == 'DATA WRITE':
                rn = Decoder.regName(self.reg)
                self.putx([15, ['Register %s (0x%02X)' % (rn, self.reg), rn,
                                'WR %s' % rn, 'WR', 'W']])
                handle_reg = getattr(self, 'handle_reg_0x%02x' % self.reg)
                handle_reg(databyte)
                self.reg += 1
                # TODO: Check for NACK!
            elif cmd == 'STOP':
                # TODO: Handle read/write of only parts of these items.
                d = 'write data.'
                self.put(self.ss_block, es, self.out_ann,
                         [9, ['Write: %s' % d, 'Write: %s' % d,
                              'W: %s' % d]])
                self.state = 'IDLE'
            else:
                pass  # TODO
        elif self.state == 'READ RTC REGS':
            # Wait for an address read operation.
            if cmd == 'ADDRESS READ':
                self.state = 'READ RTC REGS2'
                return
            else:
                pass  # TODO
        elif self.state == 'READ RTC REGS2':
            if cmd == 'DATA READ':
                rn = Decoder.regName(self.reg)
                self.putx([15, ['Register: %s (0x%02X)' % (rn, self.reg), rn,
                                'RR %s' % rn, 'RR', 'R']])
                handle_reg = getattr(self, 'handle_reg_0x%02x' % self.reg)
                handle_reg(databyte)
                self.reg += 1
                # TODO: Check for NACK!
            elif cmd == 'STOP':
                d = 'read data.'
                self.put(self.ss_block, es, self.out_ann,
                         [10, ['Read: %s' % d, 'Read: %s' % d,
                               'R: %s' % d]])
                self.state = 'IDLE'
            else:
                pass  # TODO?
