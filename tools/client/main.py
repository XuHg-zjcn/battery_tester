#!/usr/bin/env python3
#########################################################################
#  电池测试仪上位机测试程序
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
from PyQt6.QtWidgets import QApplication, QMainWindow

import device
import record
import gui


if __name__ == '__main__':
    sers = device.get_serials()
    ser = device.open_serial(sers[0]) if len(sers)>0 else None
    app = QApplication([])
    win = QMainWindow()
    dev = device.Device(ser)
    rec = record.Recorder(dev, Nhist=10000)
    dev.rec = rec
    dvc = gui.DataViewControl(win, dev, rec)
    dev.dvc = dvc
    if ser is not None:
        dev.start()
    win.show()
    app.exec()
