#ifndef __SSD1305__H
#define __SSD1305__H

#include "main.h"
#include "delay.h"
//硬件引脚定义
#define	GPIO_1305		        GPIOA

#define DI_1305_SET		GPIO_1305->BSRR = SSD1305_DI_Pin
#define DI_1305_CLR		GPIO_1305->BRR = SSD1305_DI_Pin
#define CLK_1305_SET	GPIO_1305->BSRR = SSD1305_CLK_Pin
#define CLK_1305_CLR	GPIO_1305->BRR = SSD1305_CLK_Pin
#define DC_1305_SET		GPIO_1305->BSRR = SSD1305_DC_Pin
#define DC_1305_CLR		GPIO_1305->BRR = SSD1305_DC_Pin
#define RES_1305_SET	GPIO_1305->BSRR = SSD1305_RST_Pin
#define RES_1305_CLR	GPIO_1305->BRR = SSD1305_RST_Pin
#define CS_1305_SET		GPIO_1305->BSRR = SSD1305_CS_Pin
#define CS_1305_CLR		GPIO_1305->BRR = SSD1305_CS_Pin
#define CLK_MISO_HIGH	HAL_GPIO_ReadPin(GPIO_1305, DI_1305)


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Global Variables
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define PAGE(col) ((col) * 8)                  /* 假设每个字符列宽8像素,将逻辑列号转换为实际的像素坐标 */
#define contrast    0x8F                       /* 对比度，控制显示屏的亮度。通常取值范围 0x00 (最暗) 到 0xFF (最亮)，0x8F 是一个中等亮度 */ 
#define Brightness  0xBF                       /* 亮度，控制显示屏的亮度强度。0x00 是最暗，0xFF 是最亮，0xBF 通常是一个较亮的设置 */
#define MAX_COL     128                        /* 最大列数，显示屏的宽度为 128 像素 */
#define MAX_ROW     64                         /* 最大行数，显示屏的高度为 64 像素 */
#define horizontal  0                          /* 水平寻址模式：数据按列顺序从左到右存储，每次显示从左到右更新一列像素*/ 
#define vertical    1                          /* 垂直寻址模式：数据按行顺序从上到下存储，每次显示从上到下更新一行像素 */ 
#define page_mode   2                          /* 页模式：数据按 8 行（1 页）存储，每一页更新一个字节数据。通常用于减少更新数据量 */ 


void SSD1305_init(void);
void Write_Data(u8 data);
void Write_Cmd(u8 cmd);
void Full_on(void);
void Set_Addr(u8 page, u8 col);
void clearscreen(void);
void Show_pic(u8 page, u8 a, u8 b);
void dispHzChar(u8 x, u8 y, u16 gb_code, bool fb);
void dispHzChar12(u8 x, u8 y, u16 gb_code, bool fb);
u16 searchIndex12(const u8 da[2]);
void Disp_Char_6x12(u8 x, u8 y, u8 asc_code, bool fb);
void DispString12(u8 x, u8 y, const char* str, bool Fb);
void Disp_Char(u8 x, u8 y, u8 asc_code, bool fb);
void Disp_CharB(u8 x, u8 y, u8 asc_code, bool fb);
void DispString(u8 x, u8 y, const char* str, bool Fb);
void DispStringB(u8 x, u8 y, cchar* str, bool Fb);
void Disp_Word_U(u16 x, u16 y, u8 maxFb, u32 Da, u8 PosFb, u8 Point);
void Disp_Word_UB(u16 x, u16 y, u8 maxFb, u32 Da, u8 PosFb, u8 Point);
void Disp_Word_S(u8 x, u8 y, u8 maxFb, s32 Da, u8 PosFb, u8 Point);
void Disp_Word_H(u8 x, u8 y, u8 maxFb, u32 Da, bool Fb);
void Disp_Word_B(u8 y, u16 Da);
void line_sign(u8 x, u8 num, bool Fb);
void draw_hline(u8 x0, u8 x1, u8 y);
void draw_rect(u8 x, u8 y, u8 w, u8 h, bool fill);
void draw_vspan(u8 x, u8 y0, u8 y1);
void set_Sign(u8 x, u8 num);
void Disp_Time_Sec(u8 x, u8 y,u16 count);
void Disp_CharS(u8 x, u8 y, u8 asc_code, bool fb);
void Disp_Word_US(u16 x, u16 y, u8 maxFb, u32 Da, u8 PosFb, u8 Point);
void Disp_Word_SS(u8 x, u8 y, u8 maxFb, s32 Da, u8 PosFb, u8 Point);
void DispStringS(u8 x, u8 y, cchar* str, bool Fb);
void Disp_Time_SecS(u8 x, u8 y,u16 count);
void Disp_Word_UM(u16 x, u16 y, u8 maxFb, u32 Da, u8 PosFb, u8 Point);
void Show_Word_U(u8 x, u8 y, u32 Da, u8 maxFb, u8 Point, bool fb);
void Show_Word_S(u8 x, u8 y, s32 Da, u8 maxFb, u8 Point, bool fb);


// ------------------  汉字字模的数据结构定义 ------------------------ //
typedef struct              // 汉字字模数据结构
{
	u8 Index[2];            // 汉字内码索引
	u8 Msk[32];	            // 点阵码数据
} ST_GB16;


typedef struct              // 12x12汉字字模数据结构
{
    u8 Index[2];            // 汉字内码索引（GB2312）
    u8 Msk[24];             // 点阵码数据 (12x12需要24字节)
} ST_GB12;


typedef struct              // 6x12 ASCII字模数据结构
{
    u8 Msk[12];            // 点阵码数据 (6列x2页=12字节)
} ST_ASC_6x12;



typedef struct              // ASC字模数据结构
{
	u8 Msk[16];             // 点阵码数据
}ST_ASC_8x16;

typedef struct              // ASC字模数据结构
{
	u8 Msk[192];            // 点阵码数据
}ST_ASC_BIG;

typedef struct              // ASC字模数据结构
{
	u8 Msk[5];              // 点阵码数据
}ST_ASC_S;

extern const ST_ASC_8x16 asc[];
extern const ST_ASC_BIG asc_B[];
extern const ST_ASC_S asc_S[];
extern const ST_ASC_6x12 asc_M[];
extern const ST_GB16 GB_16[];
extern	uc16  NumOfGB16;
extern const ST_GB12 GB_12[];
extern	uc16  NumOfGB12;

#endif

