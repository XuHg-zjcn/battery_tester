#!/usr/bin/env python3
#########################################################################
#  电池测试仪校准测试程序
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


#TODO: 通过GUI界面操作
if __name__ == '__main__':
    sers = device.get_serials()
    ser = device.open_serial(sers[0])
    dev = device.Device(ser)
    dev.set_para(report_ms=0)
    time.sleep(0.1)
    ser.read_all()
    V_r = float(input('电压量程(V):'))
    A_r = float(input('电流量程(A):'))
    V_bias = float(input('电压偏置(V):'))
    A_bias = float(input('电流偏置(A):'))
    R_ohm = float(input('电阻补偿(Ω):'))
    data = dev.calib(V_r=V_r, A_r=A_r, V_bias=V_bias, A_bias=A_bias, R_ohm=R_ohm)
    time.sleep(0.1)
    data_r = dev.read_data(0x6000, len(data))
    if data == data_r:
        print('校验成功')
    else:
        print('校验出错')
        print('write:', data)
        print('read:', data_r)
