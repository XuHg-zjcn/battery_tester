#!/usr/bin/env python3
#########################################################################
#  电池测试仪上位机图形界面
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
import datetime
import pyqtgraph as pg
from PyQt6.QtWidgets import QMessageBox, QFileDialog

from main_window import Ui_MainWindow
import command as cmd
import conf
import device

NCH = 2

class DataViewControl:
    def __init__(self, win, dev, rec):
        self.win = win
        self.dev = dev
        self.rec = rec
        self.ui = Ui_MainWindow()
        self.ui.setupUi(win)
        self.ui.plotwidget.showGrid(x=True, y=True)
        #self.ui.plotwidget.setRange(xRange=[0, Nhist], yRange=[0, 4095], padding=0)
        self.ui.plotwidget.setTitle('串口数据')
        self.curves = [self.ui.plotwidget.plot() for i in range(NCH)]
        self.timer_plot = pg.QtCore.QTimer()
        self.timer_plot.timeout.connect(self.update_wave)  # 定时刷新数据显示
        self.timer_plot.start(50)  # 多少ms调用一次
        self.timer_value = pg.QtCore.QTimer()
        self.timer_value.timeout.connect(self.update_showvalue)
        self.timer_value.start(100)
        self.ui.checkBox_raw.stateChanged.connect(self.checkBox_raw_stateChanged)
        self.ui.pushButton_StartStop.clicked.connect(self.pushButton_startstop_clicked)
        self.ui.pushButton_UpdateLimit.clicked.connect(self.pushButton_updatelimit_clicked)
        self.ui.comboBox_show.currentIndexChanged.connect(self.comboBox_show_changed)
        self.ui.action_OpenFile.triggered.connect(self.action_openfile)
        self.ui.checkBox_flashdata.stateChanged.connect(self.checkBox_flashdata_stateChanged)
        self.ui.menu_device.aboutToShow.connect(self.update_device_list)
        self.ui.menu_device.triggered.connect(self.menu_device_triggered)

    def update_wave(self):
        if self.ui.checkBox_raw.isChecked():
            for i in range(NCH):
                self.curves[i].setData(self.rec.data[i])
        else:
            wave_V, wave_A = self.dev.adc2si_calib(self.rec.data[0], self.rec.data[1])
            self.curves[0].setData(wave_V)
            self.curves[1].setData(wave_A)

    def update_curve(self):
        self.curves[0].setData(self.rec.mean_V)
        self.curves[1].setData(self.rec.mean_A)

    def update_showvalue(self):
        isRAW = self.ui.checkBox_raw.isChecked()
        volt = int(self.rec.data[0, -1])
        curr = int(self.rec.data[1, -1])
        if not isRAW:
            volt, curr = self.dev.adc2si_calib(volt, curr)
        power = volt*curr
        fmt = '{}' if isRAW else '{:.3f}'
        self.ui.lineEdit_Volt_curr.setText(fmt.format(volt))
        self.ui.lineEdit_Curr_curr.setText(fmt.format(curr))
        self.ui.lineEdit_Power_curr.setText(fmt.format(power))
        if self.rec.running:
            Q = self.rec.sum_Q/3600
            E = self.rec.sum_E/3600
            fmt = '{:.0f}' if isRAW else '{:.3f}'
            if isRAW:
                Q /= conf.A_LSB
                E /= conf.A_LSB * conf.V_LSB
            self.ui.lineEdit_Qu_curr.setText(fmt.format(Q))
            self.ui.lineEdit_Ene_curr.setText(fmt.format(E))
            total_time_str = str(datetime.timedelta(seconds=int(self.rec.t_last-self.rec.t0)))
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

    def checkBox_flashdata_stateChanged(self, state):
        if state:
            self.dev.set_para(save_ms=5000)
        else:
            self.dev.set_para(save_ms=0)

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
            stop_vmin_adc, curr_adc = self.dev.si2adc_calib(stop_vmin_V, curr_A)
            stop_vmin_adc = round(stop_vmin_adc)
            curr_adc = round(curr_adc)
        mode = self.ui.comboBox_mode.currentIndex()
        if mode == 0: # 恒流放电
            self.dev.set_para(curr_adc, stop_vmin_adc)
        elif mode == 1: # 扫频电流
            waveamp = round(self.dev.si2adc_calib(0, float(self.ui.lineEdit_Power_limit.text()))[1])
            logfmin = self.dev.si2logf(float(self.ui.lineEdit_Qu_limit.text()))
            logfmax = self.dev.si2logf(float(self.ui.lineEdit_Ene_limit.text()))
            logdfdt = self.dev.s_dec_to_logdfdt(float(self.ui.lineEdit_Time_limit.text()))
            self.dev.set_para(curr=curr_adc,
                              stop_vmin=stop_vmin_adc,
                              wave_amp=waveamp,
                              wave_logfmin=logfmin,
                              wave_logfmax=logfmax,
                              wave_logdfdt=logdfdt)

    def pushButton_startstop_clicked(self):
        if self.rec.running:
            self.stop()
        else:
            if self.rec.t0 is not None:
                ret = QMessageBox.question(self.win, '准备开始', '是否清除上次数据',
                        QMessageBox.StandardButton.Yes|QMessageBox.StandardButton.No,
                        QMessageBox.StandardButton.No)
                if ret == QMessageBox.StandardButton.Yes:
                    self.start_clean_data()
                else:
                     self.start_no_clean()
            else:
                self.start_clean_data()

    def get_comboxBox_mode_cmd(self):
        lst = [cmd.Mode_ConsCurr, cmd.Mode_CurrWave]
        index = self.ui.comboBox_mode.currentIndex()
        return lst[index]

    def start_clean_data(self):
        self.rec.clean()
        self.dev.set_mode(self.get_comboxBox_mode_cmd())
        self.rec.start()
        self.ui.pushButton_StartStop.setText('停止/暂停')

    def start_no_clean(self):
        self.dev.set_mode(self.get_comboxBox_mode_cmd())
        self.rec.resume()
        self.ui.pushButton_StartStop.setText('停止/暂停')

    def stop(self):
        self.dev.set_mode(cmd.Mode_Stop)
        self.rec.stop()
        self.ui.pushButton_StartStop.setText('开始/恢复')

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

    def action_openfile(self):
        path = QFileDialog.getOpenFileName()
        if path[0]:
            if path[0][-7:] == '.dat.gz':
                self.timer_plot.stop()
                self.rec.open_gzip(path[0])
                self.update_curve()

    def update_device_list(self):
        ser = device.get_serials()
        if len(ser) == 0:
            ser = ['没有设备']
        acts = self.ui.menu_device.actions()
        if len(acts) >= 3 and len(acts) == len(ser)+2:
            for a, b in zip(acts[2:], ser):
                if a.text() != b:
                    break
            else:
                return
        for a in acts[2:]:
            self.ui.menu_device.removeAction(a)
        for b in ser:
            self.ui.menu_device.addAction(b)

    def menu_device_triggered(self, act):
        text = act.text()
        if text == '断开连接':
            self.dev.ser = None
        elif text[:8] == '/dev/tty':
            self.dev.ser = device.open_serial(text)
            self.dev.start()
