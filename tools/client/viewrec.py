#!/usr/bin/env python3
import sys
import gzip
import struct
import numpy as np
import pyqtgraph as pg

import device
import conf


def get_fn():
    if len(sys.argv) == 2:
        return sys.argv[1]
    else:
        print('usage `./viewrec.py *.dat.gz`')
        return None

def read_data(fn):
    Ts_ns = []
    Volt_adc = []
    Curr_adc = []
    f = gzip.open(fn, 'rb')
    isize = 8+2+2
    while True:
        try:
            data = f.read(isize)
        except EOFError:
            break
        if len(data) != isize:
            break
        t, v, c = struct.unpack('QHH', data)
        Ts_ns.append(t)
        Volt_adc.append(v)
        Curr_adc.append(c)
    return Ts_ns, Volt_adc, Curr_adc

fn = get_fn()
if fn is None:
    sys.exit()
_, Volt_adc, Curr_adc = read_data(fn)
Volt_adc = np.array(Volt_adc)
Curr_adc = np.array(Curr_adc)
dev = device.Device(None)
Volt_V, Curr_A = dev.adc2si_calib(Volt_adc, Curr_adc)

app = pg.mkQApp()
pw = pg.PlotWidget()
pw.showGrid(x=True, y=True)
pw.setTitle(fn)
pw.plot().setData(Volt_V)
pw.plot().setData(Curr_A)
pw.show()
app.exec()
