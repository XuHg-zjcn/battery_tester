#!/usr/bin/env python3
#########################################################################
#  电池测试仪数据记录
#  Copyright (C) 2023-2024  Xu Ruijun
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
import datetime
import time
import gzip
import numpy as np


class Recorder:
    def __init__(self, dev, Nhist):
        self.dev = dev
        self.Nhist = Nhist
        self.mean_V = list()
        self.mean_A = list()
        self.ri = 0
        self.running = False
        self.sum_Q = 0
        self.sum_E = 0
        self.t0 = None  #用于计算已运行时间
        self.t_last = None  #最后一次数据时间或开始时间
        self.data = np.zeros((2, Nhist), dtype=np.uint16)
        fn = datetime.datetime.now().strftime('%Y%m%d_%H%M%S') + '.dat.gz'
        self.f = gzip.open(fn, 'wb')
        self.f.write(b"BatteryTest DataV1\n"
                     b"data:u32 tdelta(us);u16 vadc(LSB/16);u16 cadc(LSB/16)\n\0")
        #TODO: 保存完整EOF，避免读取时错误
        #TODO: 分次测试数据分别保存到不同文件
        self.t_wdata = time.monotonic_ns()
        self.ba_wdata = bytearray()

    def update(self, volt_adc, curr_adc):
        ts_ns = time.monotonic_ns()
        self.data[:,:-1] = self.data[:,1:]
        self.data[:,-1] = (volt_adc, curr_adc)
        volt_V, curr_A = self.dev.adc2si_calib(volt_adc, curr_adc)
        if self.running:
            ts = ts_ns / 1e9
            volt_V_calib, curr_A = self.dev.adc2si_calib(volt_adc, curr_adc)
            power = volt_V_calib*curr_A
            self.sum_Q += (ts-self.t_last)*curr_A
            self.sum_E += (ts-self.t_last)*power
            self.t_last = ts
            self.ri += 1
            if self.ri % 1000 == 0:
                mean_volt_adc = self.data[0,-1000:].mean()
                mean_curr_adc = self.data[1,-1000:].mean()
                mean_volt_V_calib, mean_curr_A = self.dev.adc2si_calib(mean_volt_adc, mean_curr_adc)
                self.mean_V.append(mean_volt_V_calib)
                self.mean_A.append(mean_curr_A)
        tdelta_us = (ts_ns - self.t_wdata)//1000
        data = struct.pack('IHH', tdelta_us, volt_adc, curr_adc)
        self.ba_wdata.extend(data)
        self.t_wdata += tdelta_us*1000
        if len(self.ba_wdata) >= 8000:
            self.f.write(self.ba_wdata)
            self.ba_wdata.clear()

    def clean(self):
        self.running = False
        self.mean_V = []
        self.mean_A = []
        self.sum_Q = 0
        self.sum_E = 0

    def start(self):
        ts = time.monotonic()
        self.t0 = ts
        self.t_last = ts
        self.running = True

    def resume(self):
        ts = time.monotonic()
        self.t0 += ts - self.t_last
        self.running = True

    def stop(self):
        self.running = False

    def open_gzip(self, path):
        self.clean()
        f = gzip.open(path, 'rb')
        head = f.read(1000)
        if head[:18] == b'BatteryTest DataV1':
            n = head.find(b'\0')
            f.seek(n+1)
            item_size = 8
            func = lambda i:struct.unpack('IHH', data[i*8:(i+1)*8])
        else:
            f.rewind()
            item_size = 12
            func = lambda i:struct.unpack('QHH', data[i*12:(i+1)*12])
        while True:
            try:
                data = f.read(item_size*1000)
            except Exception:
                break
            if len(data) != item_size*1000:
                break
            data_np = np.fromiter(map(func, range(1000)), np.dtype((int, 3)))
            volt_adc_mean = data_np[:,1].mean()
            curr_adc_mean = data_np[:,2].mean()
            volt_V_calib, curr_A = self.dev.adc2si_calib(volt_adc_mean, curr_adc_mean)
            self.mean_V.append(volt_V_calib)
            self.mean_A.append(curr_A)
