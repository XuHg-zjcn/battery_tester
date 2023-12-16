#!/usr/bin/env python3
#########################################################################
#  电池测试仪数据记录
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
import time
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

    def update(self, volt_adc, curr_adc):
        self.data[:,:-1] = self.data[:,1:]
        self.data[:,-1] = (volt_adc, curr_adc)
        volt_V, curr_A = self.dev.adc2si_calib(volt_adc, curr_adc)
        if self.running:
            ts = time.monotonic()
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
