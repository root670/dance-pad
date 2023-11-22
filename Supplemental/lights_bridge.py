#
# MAME Lights Bridge for DDR
#
# Sends output messages for lights from MAME to serial devices as a SextetStream
# To use this, launch MAME with `-output network` or change the `output` setting
# in `mame.ini` to `network`.
#
# Copyright 2021 Wesley Castro
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

import socket
import sys
import time

from serial import Serial
from serial.serialutil import SerialException


SERIAL_PORTS = ['COM3', 'COM54']
MAME_IP = '127.0.0.1'
MAME_PORT = 8000  # This is hardcoded in MAME and cannot be changed


class LightsEncoder:

    LIGHTS_ALL_OFF = b'@@@@@@@@@@@@@\n'

    def __init__(self, serial_names):
        self._data = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        self._serial_names = serial_names
        self._serial = [None] * len(self._serial_names)

    def connect_to_mame(self):
        print('Connecting to MAME...', flush=True, end='')
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        while True:
            try:
                self._socket.connect((MAME_IP, MAME_PORT))
                break

            except ConnectionRefusedError:
                print('.', end='', flush=True)
                continue

        print(' Connected.')
        self._socket.settimeout(1.0)

    def disconnect_all_serial(self):
        for index in range(len(self._serial)):
            self._serial[index].close()
            self._serial[index] = None

    def connect_to_serial(self):
        for index in range(len(self._serial)):
            if self._serial[index] is None:
                port_name = self._serial_names[index]
                print(
                    f'Connecting to serial port {index+1}/{len(self._serial)} ({port_name})...',
                    flush=True,
                    end=''
                )
                while True:
                    try:
                        self._serial[index] = Serial(port_name)
                        break

                    except SerialException:
                        print('.', end='', flush=True)
                        time.sleep(1.0)
                        continue

                print(' Connected.')

    def send_to_serial(self, data):
        for index in range(len(self._serial)):
            try:
                self._serial[index].write(data)

            except SerialException:
                print(f'Lost connection to serial port {self._serial_names[index]}')
                self._serial[index] = None
                self.connect_to_serial()

    @staticmethod
    def to_printable(b):
        return chr(((b + 0x10) & 0x3F) + 0x30)

    def listen(self):
        try:
            while True:
                try:
                    commands = self._socket.recv(1024).decode('ascii').split('\r')[:-1]

                except socket.timeout:
                    continue

                send_update = False
                for command in commands:
                    split = command.split(' = ')
                    if len(split) != 2:
                        print('Received bad data from MAME')
                        continue

                    name, value = split

                    # Tuples of (byte offset, bit(s) within byte when active)
                    name_to_offset = {
                        'body left high': (0, 0x01),
                        'body right high': (0, 0x02),
                        'body left low': (0, 0x04),
                        'body right low': (0, 0x08),
                        'speaker': (0, 0x30),  # Bass left and bass right
                        'lamp0': (1, 0x13),  # Player 1 menu left, right, and start
                        'foot 1p left': (3, 0x01),
                        'foot 1p right': (3, 0x02),
                        'foot 1p up': (3, 0x04),
                        'foot 1p down': (3, 0x08),
                        'lamp1': (7, 0x13),  # Player 2 menu left, right, and start
                        'foot 2p left': (9, 0x01),
                        'foot 2p right': (9, 0x02),
                        'foot 2p up': (9, 0x04),
                        'foot 2p down': (9, 0x08),
                    }

                    if name not in name_to_offset.keys():
                        continue

                    byte_offset, bit_active = name_to_offset[name]

                    if value == '1':
                        self._data[byte_offset] |= bit_active
                    else:
                        self._data[byte_offset] &= ~bit_active

                    send_update = True

                if send_update:
                    out = (''.join(map(self.to_printable, self._data)) + '\n').encode('ascii')

                    self.send_to_serial(out)

        except ConnectionResetError:
            print('Lost connection to MAME')
            self.send_to_serial(LightsEncoder.LIGHTS_ALL_OFF)
            self.disconnect_all_serial()
            return


if __name__ == '__main__':

    encoder = LightsEncoder(SERIAL_PORTS)

    while True:
        try:
            encoder.connect_to_mame()
            encoder.connect_to_serial()
            encoder.listen()
        except KeyboardInterrupt:
            exit(0)
