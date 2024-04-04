/*************************************************************************
 *  电池测试仪旧数据格式转换
 *  Copyright (C) 2023  Xu Ruijun
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>

#define ITEM_I_SIZE  (12)
#define ITEM_O_SIZE  (8)
#define BUFF_ITEM    (1024)
#define BUFF_I_SIZE (ITEM_I_SIZE*BUFF_ITEM)
#define BUFF_O_SIZE (ITEM_O_SIZE*BUFF_ITEM)

//为了未来改进可自定义保存内容，现在头部暂时添加格式描述文本
const char FileHead[] =
"BatteryTest DataV1\n"
"data:u32 tdelta(us);u16 vadc(LSB/16);u16 cadc(LSB/16)\n";

#pragma pack(1)
typedef struct{
	uint64_t ts64;
	uint16_t vadc;
	uint16_t cadc;
}stru_in;

typedef struct{
	uint32_t tdelta;
	uint16_t vadc;
	uint16_t cadc;
}stru_out;
#pragma pack()

int main(int argc, const char **argv)
{
	assert(sizeof(stru_in) == ITEM_I_SIZE);
	assert(sizeof(stru_out) == ITEM_O_SIZE);
	uint8_t buff_in[BUFF_I_SIZE];
	uint8_t buff_out[BUFF_O_SIZE];
	if(argc != 3){
		printf("usage `convert input.dat.gz output.dat.gz`\n");
		exit(0);
	}
	gzFile fin = gzopen(argv[1], "rb");
	gzFile fout = gzopen(argv[2], "wb");

	// Write Head
	gzwrite(fout, FileHead, sizeof(FileHead));

	// init ts_last
	gzread(fin, buff_in, BUFF_I_SIZE);
	uint64_t ts_last = *(uint64_t *)&buff_in[0];
	gzrewind(fin);

	int64_t blocks = 0;
	int64_t points = 0;
	while(1){
		int remain = gzread(fin, buff_in, BUFF_I_SIZE);
		if(remain <= 0){
			break;
		}
		int i = 0;
		while(remain >= sizeof(stru_in)){
			stru_in *item_in = (stru_in *)&buff_in[sizeof(stru_in)*i];
			stru_out *item_out = (stru_out *)&buff_out[sizeof(stru_out)*i];
			uint64_t tdelta = (item_in->ts64 - ts_last)/1000;
			if(tdelta > UINT32_MAX){
				printf("time delta out point=%ld, ts64=%ld, tdelta=%ld\n", points, item_in->ts64, tdelta);
			}
			item_out->tdelta = tdelta;
			item_out->vadc = item_in->vadc;
			item_out->cadc = item_in->cadc;
			ts_last += tdelta*1000;
			i++;
			remain -= sizeof(stru_in);
		}
		if(++blocks%100 == 0){
		    printf("\rblock:%ld", blocks);
		    fflush(stdout);
		}
		gzwrite(fout, buff_out, sizeof(stru_out)*i);
		points += i;
	}
	printf("\rfinish: %ld blocks, %ld points\n", blocks, points);
	gzclose(fin);
	gzclose(fout);
	return 0;
}
