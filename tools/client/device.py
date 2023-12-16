#!/usr/bin/env python3
#########################################################################
#  电池测试仪驱动程序
#  Copyright (C) 2023  Xu Ruijun
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
import struct
import glob
from PyQt6.QtCore import QThread
import serial

import conf
import command as cmd


def get_serial():
    devpath = glob.glob('/dev/ttyUSB*')[0]
    ser = serial.Serial(devpath, 115200)
    return ser


class Device(QThread):
    def __init__(self, ser):
        super().__init__()
        self.ser = ser
        self.rec = None

    def set_para(self, curr, stop_vmin):
        pack = cmd.Cmd_Head + b'\x08' + \
               cmd.Cmd_SetPara + cmd.Para_curr + int.to_bytes(curr, 2, 'little') + \
               cmd.Cmd_SetPara + cmd.Para_stop_vmin + int.to_bytes(stop_vmin, 2, 'little')
        self.ser.write(pack)

    def set_mode(self, mode):
        self.ser.write(b'\xaaCMD\x02' + cmd.Cmd_SetMode + mode)

    def run(self):
        i = 0
        while True:
            raw = self.ser.read(6)
            values = struct.unpack('<HHH', raw)
            if values[0] != 65535:
                self.ser.read(1)
                print('rxerr')
                continue
            volt_adc, curr_adc = values[1:]
            self.rec.update(volt_adc, curr_adc)

    def adc2si_calib(self, volt_adc, curr_adc):
        curr_A = curr_adc*conf.A_LSB + conf.A_bias
        volt_V_samp = volt_adc*conf.V_LSB + conf.V_bias
        volt_V_calib = volt_V_samp + curr_A*conf.R_Line
        return volt_V_calib, curr_A

    def si2adc_calib(self, volt_V, curr_A):
        curr_adc = (curr_A - conf.A_bias)/conf.A_LSB
        volt_V_samp = volt_V - curr_A*conf.R_Line
        volt_adc = (volt_V_samp - conf.V_bias)/conf.V_LSB
        return volt_adc, curr_adc
