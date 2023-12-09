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
import serial
import struct
import threading
import glob
import datetime
import time

import pyqtgraph as pg
import numpy as np
from PyQt6.QtCore import QThread
from PyQt6.QtWidgets import QApplication, QMainWindow

from main_window import Ui_MainWindow
import conf
import command as cmd

NCH = 2
Nhist = 10000

def get_serial():
    devpath = glob.glob('/dev/ttyUSB*')[0]
    ser = serial.Serial(devpath, 115200)
    return ser

def adc2si_calib(volt, curr):
    curr_A = curr*conf.A_LSB
    volt_V = volt*conf.V_LSB
    volt_V_calib = volt_V + curr_A*conf.R_Line
    return volt_V_calib, curr_A

class DataViewControl:
    def __init__(self, ui, dev, NCH, Nhist, Ntmp=10):
        self.ui = ui
        self.dev = dev
        self.NCH = NCH
        self.Nhist = Nhist
        self.Ntmp = Ntmp
        self.data = np.zeros((NCH, Nhist), dtype=np.uint16)
        self.curves = [ui.plotwidget.plot() for i in range(NCH)]
        self.i = 0
        self.running = False
        self.sum_Q = 0
        self.sum_E = 0
        self.start = None
        self.last = None
        self.ui.checkBox_raw.stateChanged.connect(self.checkBox_raw_stateChanged)
        self.ui.pushButton_startstop.clicked.connect(self.pushButton_startstop_clicked)
        self.ui.pushButton_updatelimit.clicked.connect(self.pushButton_updatelimit_clicked)

    def update(self, newdata):
        self.data[:,:-1] = self.data[:,1:]
        self.data[:,-1] = newdata
        if self.running:
            ts = time.monotonic()
            volt_V_calib, curr_A = adc2si_calib(newdata[0], newdata[1])
            power = volt_V_calib*curr_A
            self.sum_Q += (ts-self.last)*curr_A
            self.sum_E += (ts-self.last)*power
            self.last = ts

    def update_plot(self):
        for i in range(self.NCH):
            self.curves[i].setData(self.data[i])

    def update_showvalue(self):
        isRAW = self.ui.checkBox_raw.isChecked()
        volt = int(self.data[0, -1])
        curr = int(self.data[1, -1])
        if not isRAW:
            volt, curr = adc2si_calib(volt, curr)
        power = volt*curr
        fmt = '{}' if isRAW else '{:.3f}'
        self.ui.lineEdit_Volt_curr.setText(fmt.format(volt))
        self.ui.lineEdit_Curr_curr.setText(fmt.format(curr))
        self.ui.lineEdit_Power_curr.setText(fmt.format(power))
        if self.running:
            Q = self.sum_Q/3600
            E = self.sum_E/3600
            fmt = '{:.0f}' if isRAW else '{:.3f}'
            if isRAW:
                Q /= conf.A_LSB
                E /= conf.A_LSB * conf.V_LSB
            self.ui.lineEdit_Qu_curr.setText(fmt.format(Q))
            self.ui.lineEdit_Ene_curr.setText(fmt.format(E))
            total_time_str = str(datetime.timedelta(seconds=int(self.last-self.start)))
            self.ui.lineEdit_Time_curr.setText(total_time_str)

    def checkBox_raw_stateChanged(self, state):
        if state:
            self.ui.label_Volt_unit.setText('LSB')
            self.ui.label_Curr_unit.setText('LSB')
            self.ui.label_Power_unit.setText('LSB²')
            self.ui.label_Qu_unit.setText('LSB.h')
            self.ui.label_Ene_unit.setText('LSB².h')
        else:
            self.ui.label_Volt_unit.setText('V')
            self.ui.label_Curr_unit.setText('A')
            self.ui.label_Power_unit.setText('W')
            self.ui.label_Qu_unit.setText('Ah')
            self.ui.label_Ene_unit.setText('Wh')

    def pushButton_updatelimit_clicked(self):
        isRAW = self.ui.checkBox_raw.isChecked()
        s1 = self.ui.lineEdit_Volt_limit.text()
        s2 = self.ui.lineEdit_Curr_limit.text()
        if isRAW:
            stop_vmin_adc = int(s1)
            curr_adc = int(s2)
        else:
            stop_vmin_V_calib = float(s1)
            curr_A = float(s2)
            curr_adc = round(curr_A/conf.A_LSB)
            stop_vmin_V_samp = stop_vmin_V_calib - curr_A*conf.R_Line
            stop_vmin_adc = round(stop_vmin_V_samp/conf.V_LSB)
        self.dev.set_para(curr_adc, stop_vmin_adc)

    def pushButton_startstop_clicked(self):
        if self.running:
            self.dev.set_mode(cmd.Mode_Stop)
            self.running = False
            self.ui.pushButton_startstop.setText('点击开始')
        else:
            self.dev.set_mode(cmd.Mode_ConsCurr)
            ts = time.monotonic()
            self.start = ts
            self.last = ts
            self.running = True
            self.ui.pushButton_startstop.setText('点击停止')

class Device(QThread):
    def __init__(self, ser):
        super().__init__()
        self.ser = ser
        self.dvc = None

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
            self.dvc.update(values[1:])

if __name__ == '__main__':
    ser = get_serial()
    app = QApplication([])
    win = QMainWindow()
    ui = Ui_MainWindow()
    ui.setupUi(win)
    ui.plotwidget.showGrid(x=True, y=True)
    ui.plotwidget.setRange(xRange=[0, Nhist], yRange=[0, 4095], padding=0)
    ui.plotwidget.setTitle('串口数据')
    dev = Device(ser)
    dvc = DataViewControl(ui, dev, NCH=NCH, Nhist=Nhist)
    dev.dvc = dvc
    dev.start()
    timer_plot = pg.QtCore.QTimer()
    timer_plot.timeout.connect(dvc.update_plot)  # 定时刷新数据显示
    timer_plot.start(50)  # 多少ms调用一次
    timer_value = pg.QtCore.QTimer()
    timer_value.timeout.connect(dvc.update_showvalue)
    timer_value.start(100)
    win.show()
    app.exec()
