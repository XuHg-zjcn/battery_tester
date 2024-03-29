# 电池测试仪固件和上位机

# 使用方法
1. 连接
   - MCU使用CH32V203C8T6
   - PA1接电流采样信号
   - PA2接电压采样信号
   - PA15多级RC滤波后接NPN三极管和NMOS管
   - USART1(PA9/Rx, PA10/Tx)通过USB转串口连接电脑
1. 在`tools/client`目录下执行`./rx_test.py`打开上位机程序
1. 在限值中填入截止电压和放电电流

# 功能列表
## 已实现功能
- 恒流放电
- 截止电压
- OLED显示实时数据

## 上位机功能
- 显示波形
- 放电曲线
- 波形压缩保存

## 待实现功能
- 放电停止后上位机停止累计并提示
- 电池意外断开暂停并提示，待重新连接后继续
- 添加按键，脱离电脑使用
- 测试参数和数据储存到FLASH中，连接电脑后传输
- 测量内阻，电化学阻抗谱(EIS)
- 通信数据校验
- 充电
- 与BMS的通信接口(SMBus等)
- 同时测试多组电池
- 测量每节电芯电压

#版权和许可
请见各文件头部

本README.md文件使用以下许可证
```
Copyright (C) 2023-2024 Xu Ruijun

Copying and distribution of this README file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.  This file is offered as-is,
without any warranty.
```