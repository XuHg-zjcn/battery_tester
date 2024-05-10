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
import math
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
        self.fPID = 72000000.0/256/16

    def set_para(self, curr=None, stop_vmin=None, report_ms=None, wave_amp=None, wave_logfmin=None, wave_logfmax=None, wave_logdfdt=None):
        pack = bytearray(cmd.Cmd_Head + b'\x08')
        if curr is not None:
            pack.extend(cmd.Cmd_SetPara + cmd.Para_curr + int.to_bytes(curr, 2, 'little'))
        if stop_vmin is not None:
            pack.extend(cmd.Cmd_SetPara + cmd.Para_stop_vmin + int.to_bytes(stop_vmin, 2, 'little'))
        if report_ms is not None:
            pack.extend(cmd.Cmd_SetPara + cmd.Para_report_ms + int.to_bytes(report_ms, 2, 'little'))
        if wave_amp is not None:
            pack.extend(cmd.Cmd_SetPara + cmd.Para_wave_amp + int.to_bytes(wave_amp, 2, 'little'))
        if wave_logfmin is not None:
            pack.extend(cmd.Cmd_SetPara + cmd.Para_wave_logfmin + int.to_bytes(wave_logfmin, 2, 'little'))
        if wave_logfmax is not None:
            pack.extend(cmd.Cmd_SetPara + cmd.Para_wave_logfmax + int.to_bytes(wave_logfmax, 2, 'little'))
        if wave_logdfdt is not None:
            pack.extend(cmd.Cmd_SetPara + cmd.Para_wave_logdfdt + int.to_bytes(wave_logdfdt&0xffff, 2, 'little'))
        pack[4] = len(pack)-5
        self.ser.write(pack)
        print('send pack:', pack)

    def set_mode(self, mode):
        pack = b'\xaaCMD\x02' + cmd.Cmd_SetMode + mode
        self.ser.write(pack)
        print('send pack:', pack)

    def run(self):
        i = 0
        while True:
            raw = self.ser.read(6)
            values = struct.unpack('<HHH', raw)
            if values[0] == 0xffff:
                volt_adc, curr_adc = values[1:]
                self.rec.update(curr_adc, volt_adc)
            elif raw == b'\xff\xfeSTOP':
                self.rec.stop()
                self.dvc.ui.pushButton_StartStop.setText('开始/恢复')
                print('stop')
            else:
                self.ser.read(1)
                print('rxerr', raw)
                continue

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

    def si2logf(self, f):
        return round(0x10000+math.log2(f/self.fPID)*0x800)

    def s_dec_to_logdfdt(self, s_decHz):
        return round(math.log2(10)/s_decHz/self.fPID*0x08000000)
