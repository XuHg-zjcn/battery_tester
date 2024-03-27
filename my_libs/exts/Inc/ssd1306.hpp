/*
 * ssd1306.cpp
 *
 *  Created on: May 12, 2021
 *      Author: xrj
 */

#include "mylibs_config.hpp"
#include "c_i2c.hpp"
#include "s_i2c.hpp"

#if defined(SSD1306_I2C_Dev) && !defined(__SSD1306_HPP__)
#define __SSD1306_HPP__

/* I2C slave address of SSD1306
 *   MSB  .   .   .   .   .   .  LSB
 * bit7   6   5   4   3   2   1   0
 *  | 0 | 1 | 1 | 1 | 1 | 0 |SA0|R/W|
 */
#define SA0         0     // State of D/C Pin
#define Addr_OLED  (0x3C | SA0)

/* control byte, in HAL as mem_addr.
 *   MSB  .   .   .   .   .   .  LSB
 * bit7   6   5   4   3   2   1   0
 *  |Co |D/C| 0 | 0 | 0 | 0 | 0 | 0 |
 *
 */
#define ConByte_Contin 0x80
#define ConByte_Data   0x40
#define ConByte_Cmd    0x00

/* Control Byte
 *   MSB  .   .   .   .   .   .  LSB
 * bit7   6   5   4   3   2   1   0
 *  |Co |D/C| 0 | 0 | 0 | 0 | 0 | 0 |
 *
 *  Co  – Continuation bit
 *  D/C – Data / Command Selection bit
 *  SA0 – Slave address bit
 */

//from datasheet 9.1 COMMAND TABLE
// ...    : no param          commd_bytes(...);
// ..._nb : n bits at last    commd_bytes(..._nb | bits);
// ..._nB : n Bytes params    commd_bytes(..._nB, Byte1, Byte2, ..., ByteN);
// ..._nbmB: n bits, m Bytes  commd_bytes(..._nbmB | bits, Byte1, ..., ByteM);
typedef enum{
	Display_Follows_RAM = 0xA4,
	Display_Ignores_RAM = 0xA5,

	Normal_Display  = 0xA6,
	Inverse_Display = 0xA7,

	Display_OFF = 0xAE,
	Display_ON  = 0xAF,

	H_Scroll_1b6B     = 0x26, //advice use:
	VH_Scroll_2b5B    = 0x28, //SSD1306::VH_scroll(...);
	Deactivate_Scroll = 0x2E, //SSD1306::Scroll_Disable();
	Activate_Scroll   = 0x2F,
	V_Scorll_Area_2B  = 0xA3,

	Addressing_Mode_1B = 0x20,

//commands in Horizontal/Vertical Mode
	Set_Column_Addr_2B = 0x21,
	Set_Page_Addr_2B   = 0x22,

//commands in Page mode
	Low_Col_Addr_4b    = 0x00,  //low 4bit
	High_Col_Addr_4b   = 0x10,  //high 4bit
	Start_Page_Addr_4b = 0xB0,

	OutScan_Norm = 0xC0,
	OutScan_Inv  = 0xC8,

	Start_Line_6b = 0x40,
	Contrast_1B   = 0x81,

	Seg_Remap0 = 0xA0,
	Seg_Remap1 = 0xA1,

	Multiplex_Ratio_1B = 0xA8,
	Display_Offset_1B  = 0xD3,

	Clock_Setting_1B    = 0xD5,
	Prechange_Period_1B = 0xD9,
	COM_HW_Pins_Conf_1B = 0xDA,
	V_COMH_Deselect_1B = 0xDB,
	Charge_Pump_Set_1B = 0x8D,
	SSD1306_Nop = 0xE3,
}SSD1306_Commd;

typedef enum{
	Horz_Mode = 0x00,
	Vert_Mode = 0x01,
	Page_Mode = 0x02
}SSD1306_AddrMode;

//only for commd with suffix `_nb` or `nbmB`
inline SSD1306_Commd operator|(SSD1306_Commd commd, int bits){
	return (SSD1306_Commd)((int)commd|bits);
}

#define U64_TOP            (1ULL)     //use << operate
#define U64_BOTTOM         (1ULL<<63) //use >> operate

typedef enum{
	Scroll_Right = (int)H_Scroll_1b6B | 0b0,
	Scroll_Left  = (int)H_Scroll_1b6B | 0b1,
	Scroll_VertRight = (int)VH_Scroll_2b5B | 0b01,
	Scroll_VertLeft  = (int)VH_Scroll_2b5B | 0b10
}ScrollType;

typedef enum{
	Interval_05_Frames = 0x0,
	Interval_64_Frames = 0x1,
	Interval_128_Frames = 0x2,
	Interval_256_Frames = 0x3,
	Interval_03_Frames = 0x4,
	Interval_04_Frames = 0x5,
	Interval_25_Frames = 0x6,
	Interval_02_Frames = 0x7,
}FrameOfStep;

#pragma pack(1)
typedef struct{
	ScrollType type:8;    //   | 0 0 1 0 X X X X
	uint8_t dummp;        // A | 0 0 0 0 0 0 0 0
	uint8_t sta_page;     // B | * * * * * B B B
	FrameOfStep frames:8; // C | * * * * * C C C
	uint8_t end_page;     // D | * * * * * D D D
	uint8_t v_offset;     // E | * * E E E E E E
	uint8_t dummp_ff;     // F | 1 1 1 1 1 1 1 1
}ScrollSetupCommd;
#pragma pack()


class SSD1306{
private:
	uint32_t timeout;
	uint8_t col_i;
	int n_bytes(SSD1306_Commd Byte0);
#ifdef CMSIS_OS2_H_
	osSemaphoreId_t lock;
#endif
public:
	SSD1306_I2C_Dev* dev;
	SSD1306(SSD1306_I2C_Dev *dev);
	void commd_bytes(SSD1306_Commd Byte0, ...);
	void ScrollSetup(ScrollSetupCommd* commd);
	void OnOffScroll(bool IsActivate);
	void Init();
	void fill(uint8_t data);
	void plot_128(uint8_t *data, uint8_t bias, uint8_t maxh);
	void VH_scroll(int dx, int dy, uint8_t sta_page, uint8_t end_page, FrameOfStep frames);
	void Scroll_Disable();
	void Scroll_Step();
	void append_column(uint64_t col);
	void gif_show(uint8_t *imgs, uint32_t n_imgs, uint32_t ms);
	void frame_callback();
	void setVHAddr(SSD1306_AddrMode mode, uint8_t col_s, uint8_t col_e, uint8_t page_s, uint8_t page_e);
	void setPageAddr(uint8_t col_s, uint8_t page_s);
	void text_3x5(char* str, uint8_t y);
	void text_5x7(char* str);
	void write_data(uint8_t* p, uint32_t size);
};

#endif /* __SSD1306_HPP__ */
