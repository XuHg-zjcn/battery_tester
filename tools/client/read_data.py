#!/usr/bin/env python3
#########################################################################
#  电池测试仪读取数据测试程序
#  Copyright (C) 2024  Xu Ruijun
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.
#########################################################################
import device
import time

def find_end(data):
    s = 0
    while True:
        x = data.find(b'\x39\xe3\x39\xe3', s)
        if x%4 == 0 or x < 0:
            break
        else:
            s = (x//4)*4+4
    return x

#TODO: 通过GUI界面操作，读出，保存，打开等操作
if __name__ == '__main__':
    ser = device.get_serial()
    dev = device.Device(ser)
    dev.set_para(report_ms=0)
    time.sleep(0.1)
    ser.read_all()
    raddr = 0x5000
    f = open('flash_data.bin', 'wb')
    while True:
        data = dev.read_data(raddr, 128)
        x = find_end(data)
        if x < 0:
            raddr += 128
            f.write(data)
        else:
            data_x = data[:x]
            if len(data_x) > 0:
                f.write(data_x)
            break
    print('got', f.tell(), 'bytes')
    f.close()
