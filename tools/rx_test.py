#!/usr/bin/env python3
#########################################################################
#  电池测试仪上位机测试程序
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
import pyqtgraph as pg
import numpy as np
import serial
import struct
import threading
import glob

NCH = 2
Nhist = 10000

class DataPloter:
    def __init__(self, plot, NCH, Nhist, Ntmp=10):
        self.plot = plot
        self.NCH = NCH
        self.Nhist = Nhist
        self.Ntmp = Ntmp
        self.data = np.zeros((NCH, Nhist), dtype=np.uint16)
        self.curves = [plot.plot() for i in range(NCH)]
        self.i = 0

    def update(self, newdata):
        self.data[:,:-1] = self.data[:,1:]
        self.data[:,-1] = newdata

    def update_plot(self):
        for i in range(self.NCH):
            self.curves[i].setData(self.data[i])

class Recver(pg.QtCore.QThread):
    def __init__(self, ser, ploter):
        super().__init__()
        self.ser = ser
        self.ploter = ploter

    def run(self):
        i = 0
        while True:
            raw = self.ser.read(6)
            values = struct.unpack('<HHH', raw)
            if values[0] != 65535:
                self.ser.read(1)
                print('rxerr')
                continue
            self.ploter.update(values[1:])

if __name__ == '__main__':
    devpath = glob.glob('/dev/ttyUSB*')[0]
    ser = serial.Serial(devpath, 115200)
    app = pg.mkQApp()
    pw = pg.PlotWidget()
    pw.showGrid(x=True, y=True)
    pw.setRange(xRange=[0, Nhist], yRange=[0, 4095], padding=0)
    pw.setTitle('串口数据')
    ploter = DataPloter(pw, NCH=NCH, Nhist=Nhist)
    recver = Recver(ser, ploter)
    recver.start()
    timer = pg.QtCore.QTimer()
    timer.timeout.connect(ploter.update_plot)  # 定时刷新数据显示
    timer.start(20)  # 多少ms调用一次
    pw.show()
    app.exec()
