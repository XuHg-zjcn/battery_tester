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

def adc2si_calib(volt_adc, curr_adc):
    curr_A = curr_adc*conf.A_LSB + conf.A_bias
    volt_V_samp = volt_adc*conf.V_LSB + conf.V_bias
    volt_V_calib = volt_V_samp + curr_A*conf.R_Line
    return volt_V_calib, curr_A

def si2adc_calib(volt_V, curr_A):
    curr_adc = (curr_A - conf.A_bias)/conf.A_LSB
    volt_V_samp = volt_V - curr_A*conf.R_Line
    volt_adc = (volt_V_samp - conf.V_bias)/conf.V_LSB
    return volt_adc, curr_adc

class DataViewControl:
    def __init__(self, ui, dev, NCH, Nhist):
        self.ui = ui
        self.dev = dev
        self.NCH = NCH
        self.Nhist = Nhist
        self.data = np.zeros((NCH, Nhist), dtype=np.uint16)
        self.mean_V = list()
        self.mean_A = list()
        self.curves = [ui.plotwidget.plot() for i in range(NCH)]
        self.ri = 0
        self.running = False
        self.sum_Q = 0
        self.sum_E = 0
        self.start = None
        self.last = None
        self.timer_plot = pg.QtCore.QTimer()
        self.timer_plot.timeout.connect(self.update_wave)  # 定时刷新数据显示
        self.timer_plot.start(50)  # 多少ms调用一次
        self.timer_value = pg.QtCore.QTimer()
        self.timer_value.timeout.connect(self.update_showvalue)
        self.timer_value.start(100)
        self.ui.checkBox_raw.stateChanged.connect(self.checkBox_raw_stateChanged)
        self.ui.pushButton_startstop.clicked.connect(self.pushButton_startstop_clicked)
        self.ui.pushButton_updatelimit.clicked.connect(self.pushButton_updatelimit_clicked)
        self.ui.comboBox_show.currentIndexChanged.connect(self.comboBox_show_changed)

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
            self.ri += 1
            if self.ri % 1000 == 0:
                mean_volt_adc = self.data[0,-1000:].mean()
                mean_curr_adc = self.data[1,-1000:].mean()
                mean_volt_V_calib, mean_curr_A = adc2si_calib(mean_volt_adc, mean_curr_adc)
                self.mean_V.append(mean_volt_V_calib)
                self.mean_A.append(mean_curr_A)

    def update_wave(self):
        for i in range(self.NCH):
            self.curves[i].setData(self.data[i])

    def update_curve(self):
        self.curves[0].setData(self.mean_V)
        self.curves[1].setData(self.mean_A)

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
            stop_vmin_V = float(s1)
            curr_A = float(s2)
            stop_vmin_adc, curr_adc = si2adc_calib(stop_vmin_V, curr_A)
            stop_vmin_adc = round(stop_vmin_adc)
            curr_adc = round(curr_adc)
        self.dev.set_para(curr_adc, stop_vmin_adc)

    def pushButton_startstop_clicked(self):
        if self.running:
            self.dev.set_mode(cmd.Mode_Stop)
            #TODO: 询问是否清除数据
            self.running = False
            self.ui.pushButton_startstop.setText('点击开始')
        else:
            self.dev.set_mode(cmd.Mode_ConsCurr)
            ts = time.monotonic()
            self.start = ts
            self.last = ts
            self.running = True
            self.ui.pushButton_startstop.setText('点击停止')

    def comboBox_show_changed(self, index):
        if index == 0:
            self.timer_plot.stop()
            self.timer_plot.timeout.disconnect(self.update_curve)
            self.timer_plot.timeout.connect(self.update_wave)
            self.timer_plot.setInterval(50)
            self.timer_plot.start()
        elif index == 1:
            self.timer_plot.stop()
            self.timer_plot.timeout.disconnect(self.update_wave)
            self.timer_plot.timeout.connect(self.update_curve)
            self.timer_plot.setInterval(200)
            self.timer_plot.start()


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
    win.show()
    app.exec()
