# Amp = A_LSB*adc_value + A_bias
# Volt = V_LSB*adc_value + V_bias + R_Line*Amp
V_LSB = 5.020/4096/16
V_bias = -0.022
A_LSB = 3.299/4096/16
A_bias = 0.033
R_Line = 0.163

A_stop = 0.05
V_det = 0.5
