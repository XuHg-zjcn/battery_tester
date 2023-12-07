Cmd_Head = b'\xAACMD'

Cmd_None = b'\x00'
Cmd_SetMode = b'\x01'
Cmd_GetMode = b'\x02'
Cmd_SetPara = b'\x03'
Cmd_GetPara = b'\x04'

Mode_Stop = b'\x00'
Mode_ConsCurr = b'\x01'
Mode_ConsVolt = b'\x02'
Mode_ConsPower = b'\x03'
Mode_ConsResis = b'\x04'

Para_curr = b'\x00'
Para_stop_vmin = b'\x01'
Para_report_ms = b'\x02'
