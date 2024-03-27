/*
 * ssd1306.cpp
 *
 *  Created on: May 12, 2021
 *      Author: xrj
 */

#include "ssd1306.hpp"
#ifdef __SSD1306_HPP__

#include <cstdarg>
#include <cstring>
#include "myints.h"
#include "font_3x5.h"
#include "font_5x7.h"
#include "ops.hpp"
#include "delay.hpp"

/*
 * coding rules:
 * use blocking transmit or receive for commands
 */


SSD1306::SSD1306(SSD1306_I2C_Dev *dev)
{
	this->dev = dev;
	col_i = 0xff;
}

bool isIn(u8 elem, const u8* p, u32 n)
{
	const u8 *p2 = p+n;
	while(*p!=elem && p++<p2);
	return p<p2;
}

const u8 b1[] = {0x2E, 0x2F, 0xA0, 0xA1, 0xAE, 0xAF, 0xC0, 0xC8, 0xE3};
const u8 b2[] = {0x20, 0x81, 0x8D, 0xA8, 0xD3, 0xD5, 0xD9, 0xDA, 0xDB};
const u8 b3[] = {0x21, 0x22, 0xA3};

int SSD1306::n_bytes(SSD1306_Commd Byte0)
{
	if((Byte0&0xC0) == 0x40){  //Set Display Start Line
		return 1;
	}
	u8 H4 = Byte0&0xF0;
	if(H4 == 0x00 ||  //Set Lower/Higher Column Start Address for Page Addressing Mode
	   H4 == 0xB0){   //Set Page Start Address for Page Addressing Mode
		return 1;
	}
	if((Byte0&0xFC) == 0xA4){  //Entire display ON, Set Normal/Inverse Display
		return 1;
	}
	//find Byte0 in list
	if(isIn(Byte0, b1, 9)){
		return 1;
	}if(isIn(Byte0, b2, 9)){
		return 2;
	}if(isIn(Byte0, b3, 3)){
		return 3;
	}
	switch((u8)Byte0){
	case 0x26: case 0x27:
		return 7;
	case 0x29: case 0x2A:
		return 8;
	default:
		return 0;
	}
}

void SSD1306::commd_bytes(SSD1306_Commd Byte0, ...)
{
	int size = n_bytes(Byte0);
	if(size == 0){
		return;
	}
	uint8_t bytes[8];
	bytes[0] = Byte0;
    va_list list;
    va_start(list, Byte0);
    for(int i=1;i<size;i++){
    	bytes[i] = va_arg(list, int);
    }
    va_end(list);
    dev->Mem_write(ConByte_Cmd, bytes, size);
}

void SSD1306::ScrollSetup(ScrollSetupCommd* commd)
{
	uint16_t size;
	commd->dummp = 0x00;
	switch(commd->type){
	case Scroll_Right: case Scroll_Left:
		size = 7;
		commd->v_offset = 0x00;
		commd->dummp_ff = 0xff;
		break;
	case Scroll_VertRight: case Scroll_VertLeft:
		size = 6;
		break;
	default:
		return;
	}
	dev->Mem_write(ConByte_Cmd, (uint8_t*)commd, size);
}

void SSD1306::OnOffScroll(bool IsActivate)
{
	SSD1306_Commd commd = IsActivate ? Activate_Scroll : Deactivate_Scroll;
	commd_bytes(commd);
}

void SSD1306::Init()
{
	  XDelayMs(500);
	  commd_bytes(Display_OFF);
	  commd_bytes(Low_Col_Addr_4b  | 0b0000);
	  commd_bytes(High_Col_Addr_4b | 0b0000);
	  commd_bytes(Start_Line_6b    | 0b000000);
	  commd_bytes(Contrast_1B, 0x1F);  //亮度
	  commd_bytes(Normal_Display);
	  commd_bytes(Multiplex_Ratio_1B,  0x3F);
	  commd_bytes(Display_Offset_1B,   0x00);
	  commd_bytes(Clock_Setting_1B,    0xF0);
	  commd_bytes(Prechange_Period_1B, 0xF1);
	  commd_bytes(COM_HW_Pins_Conf_1B, 0x12);
	  commd_bytes(V_COMH_Deselect_1B,  0x40);
	  commd_bytes(Charge_Pump_Set_1B,  0x14);  //电荷泵

	  commd_bytes(Addressing_Mode_1B, Vert_Mode);
	  commd_bytes(Seg_Remap0);   //左右翻转
	  commd_bytes(OutScan_Norm); //上下翻转
	  commd_bytes(Display_Follows_RAM);
	  commd_bytes(Display_ON);
}

void SSD1306::fill(uint8_t data)
{
	uint8_t* x128 = (uint8_t*)XMalloc(1024);
	memset(x128, data, 1024);
	commd_bytes(Addressing_Mode_1B, Vert_Mode);
	commd_bytes(Set_Column_Addr_2B, 0, 127);  //page0-page1
	commd_bytes(Set_Page_Addr_2B, 0, 7);  //page0-page1
	dev->Mem_write(ConByte_Data, x128, 1024);
	XFree(x128);
}

void SSD1306::plot_128(uint8_t *data, uint8_t bias, uint8_t maxh)
{
	uint64_t col;
	uint32_t data2;
	commd_bytes(Addressing_Mode_1B, Vert_Mode);
	commd_bytes(Set_Column_Addr_2B, 0, 127);  //page0-page1
	commd_bytes(Set_Page_Addr_2B, 0, 7);  //page0-page1
	for(int i=0;i<128;i++){
		data2 = value_upper(*data, maxh) + bias;
		col = U64_BOTTOM>>(data2);  //ULL = uint64_t
		dev->Mem_write(ConByte_Data, (uint8_t*)&col, 8);
		data++;
	}
}

void SSD1306::VH_scroll(int dx, int dy, uint8_t sta_page, uint8_t end_page, FrameOfStep frames)
{
	ScrollSetupCommd commd;
	switch(dx){
	case -1:
		col_i = 127;
		break;
	case 1:
		col_i = 0;
		break;
	default:
		return;
	}
	if(dy!=0){
		commd.type = dx==-1 ? Scroll_VertLeft : Scroll_VertRight;
	}else{
		commd.type = dx==-1 ? Scroll_Left : Scroll_Right;
	}
	commd.sta_page = sta_page;
	commd.frames = frames;
	commd.end_page = end_page;
	commd.v_offset = dy<0 ? 63+dy : dy;
	ScrollSetup(&commd);
	commd_bytes(V_Scorll_Area_2B, 0, 64);
	commd_bytes(Activate_Scroll);
	commd_bytes(Set_Column_Addr_2B, col_i, col_i==0?1:0);  //page0-page1
	commd_bytes(Set_Page_Addr_2B, sta_page, end_page);
}

void SSD1306::Scroll_Disable()
{
	commd_bytes(Deactivate_Scroll);
	col_i = 0xff;
}

/*
 * @note: 芯片原本不支持单步滚动，只能打开一下立马关闭。
 * 实验结果：有不确定性，可能会一次滚动多步，也可能不滚动。
 */
void SSD1306::Scroll_Step()
{
	commd_bytes(Activate_Scroll);
	commd_bytes(Deactivate_Scroll);
}

void SSD1306::append_column(uint64_t col)
{
	if(col_i != 0xff){  //滚动模式
		//减少传输错误影响，避免大面积混乱
		//实验结果：必须先传输列修改，否则会停止滚动
		commd_bytes(Set_Column_Addr_2B, col_i, col_i==0?1:0);
		commd_bytes(Set_Page_Addr_2B, 0, 7);
	}
	dev->Mem_write(ConByte_Data, (uint8_t*)&col, 8);
}

void SSD1306::gif_show(uint8_t *imgs, uint32_t n_img, uint32_t ms)
{
	commd_bytes(Addressing_Mode_1B, Vert_Mode);
	commd_bytes(Set_Column_Addr_2B, 0, 127);
	commd_bytes(Set_Page_Addr_2B, 0, 7);
	for(uint32_t i=0;i<n_img;i++){
		dev->Mem_write(ConByte_Data, imgs+i*1024, 1024);
		XDelayMs(ms);
	}
}

void SSD1306::setVHAddr(SSD1306_AddrMode mode, uint8_t col_s, uint8_t col_e, uint8_t page_s, uint8_t page_e)
{
	if(mode == Horz_Mode || mode == Vert_Mode){
		commd_bytes(Addressing_Mode_1B, mode);
		commd_bytes(Set_Column_Addr_2B, col_s, col_e);
		commd_bytes(Set_Page_Addr_2B, page_s, page_e);
	}
}

void SSD1306::setPageAddr(uint8_t col_s, uint8_t page_s)
{
	commd_bytes(Addressing_Mode_1B, Page_Mode);
	commd_bytes(Start_Page_Addr_4b | (page_s & 0xf));
	commd_bytes(Low_Col_Addr_4b    | ( col_s & 0xf));
	commd_bytes(High_Col_Addr_4b   | ((col_s>>4)&0xf));
}

/*
 * show 3x5 ASCII text, support external table
 * @param str: string
 * @param y: 0<=y<=3
 */
void SSD1306::text_3x5(char* str, uint8_t y)
{
	uint16_t c16 = 0;
	uint8_t c8[4] = {0,0,0,0};
	while(*str){
		c16 = font3x5[(u8)(*str++)];
		for(int i=0;i<3;i++){
			c8[i] = (c16&0x1f) << y;
			c16 >>= 5;
		}
		dev->Mem_write(ConByte_Data, c8, 4);
	}
}

void SSD1306::text_5x7(char* str)
{
	while(*str){
		if(0x20<=*str && *str<=0x7f){
			dev->Mem_write(ConByte_Data, (uint8_t*)&font5x7[(*str-0x20)*5], 5);
		}else{
			dev->Mem_write(ConByte_Data, (uint8_t*)&font5x7[0], 5);
		}
		dev->Mem_write(ConByte_Data, (uint8_t*)&font5x7[0], 1);
		str++;
	}
}

void SSD1306::write_data(uint8_t* p, uint32_t size)
{
	dev->Mem_write(ConByte_Data, p, size);
}

#endif
