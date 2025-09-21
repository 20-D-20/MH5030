#ifndef __SSD1305__H
#define __SSD1305__H

#include "main.h"
#include "delay.h"
//Ӳ�����Ŷ���
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
#define PAGE(col) ((col) * 8)                  /* ����ÿ���ַ��п�8����,���߼��к�ת��Ϊʵ�ʵ��������� */
#define contrast    0x8F                       /* �Աȶȣ�������ʾ�������ȡ�ͨ��ȡֵ��Χ 0x00 (�) �� 0xFF (����)��0x8F ��һ���е����� */ 
#define Brightness  0xBF                       /* ���ȣ�������ʾ��������ǿ�ȡ�0x00 �����0xFF ��������0xBF ͨ����һ������������ */
#define MAX_COL     128                        /* �����������ʾ���Ŀ��Ϊ 128 ���� */
#define MAX_ROW     64                         /* �����������ʾ���ĸ߶�Ϊ 64 ���� */
#define horizontal  0                          /* ˮƽѰַģʽ�����ݰ���˳������Ҵ洢��ÿ����ʾ�����Ҹ���һ������*/ 
#define vertical    1                          /* ��ֱѰַģʽ�����ݰ���˳����ϵ��´洢��ÿ����ʾ���ϵ��¸���һ������ */ 
#define page_mode   2                          /* ҳģʽ�����ݰ� 8 �У�1 ҳ���洢��ÿһҳ����һ���ֽ����ݡ�ͨ�����ڼ��ٸ��������� */ 


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


// ------------------  ������ģ�����ݽṹ���� ------------------------ //
typedef struct              // ������ģ���ݽṹ
{
	u8 Index[2];            // ������������
	u8 Msk[32];	            // ����������
} ST_GB16;


typedef struct              // 12x12������ģ���ݽṹ
{
    u8 Index[2];            // ��������������GB2312��
    u8 Msk[24];             // ���������� (12x12��Ҫ24�ֽ�)
} ST_GB12;


typedef struct              // 6x12 ASCII��ģ���ݽṹ
{
    u8 Msk[12];            // ���������� (6��x2ҳ=12�ֽ�)
} ST_ASC_6x12;



typedef struct              // ASC��ģ���ݽṹ
{
	u8 Msk[16];             // ����������
}ST_ASC_8x16;

typedef struct              // ASC��ģ���ݽṹ
{
	u8 Msk[192];            // ����������
}ST_ASC_BIG;

typedef struct              // ASC��ģ���ݽṹ
{
	u8 Msk[5];              // ����������
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

