#include "ssd1305.h"

//��SPI�����һ�ֽ�����
/**
 * @brief      ͨ�� GPIO ģ�� SPI ������д�� 1 �ֽڣ�MSB first��
 * @param      data    �����͵������ֽ�
 * @retval     None
 */
void spi_write(u8 data)
{
    u8 i;                                                            /* ѭ�������� */

    for (i = 0; i < 8; i++)
    {
       delay_us(1);                                                  /* ����ʱ����֤ʱ������ */
        CLK_1305_CLR;                                                /* ����ʱ�ӣ�׼����������λ */

        /* �������λ������ݣ�MSB first�� */
       delay_us(1);
        if (data & 0x80)    DI_1305_SET;
        else                DI_1305_CLR;

        data <<= 1;                                                  /* ���� 1 λ��׼����һλ */

       delay_us(1);                                                  /* ����ʱ��ȷ�������ȶ� */
        CLK_1305_SET;                                                /* ��������������λ */
    }
}

/**
 * @brief      ����һ�ֽ�������裨DC=0��
 * @param      cmd     �����ֽ�
 * @retval     None
 */
void Write_Cmd(u8 cmd)
{
   delay_us(1);                                                         /* ʱ������ */
    CS_1305_CLR;                                                     /* Ƭѡʹ�� */

   delay_us(1);
    DC_1305_CLR;                                                     /* ָʾ����Ϊ���� */

   delay_us(1);
    spi_write(cmd);                                                  /* �������� */

   delay_us(1);
    DC_1305_SET;                                                     /* �ָ�Ϊ����̬����ѡ�� */

   delay_us(1);
    CS_1305_SET;                                                     /* Ƭѡ�ͷ� */
}

/**
 * @brief      ����һ�ֽ����ݵ����裨DC=1��
 * @param      data    �����ֽ�
 * @retval     None
 */
void Write_Data(u8 data)
{
   delay_us(1);                                                         /* ʱ������ */
    CS_1305_CLR;                                                     /* Ƭѡʹ�� */

   delay_us(1);
    DC_1305_SET;                                                     /* ָʾ����Ϊ���� */

   delay_us(1);
    spi_write(data);                                                 /* �������� */

   delay_us(1);
    DC_1305_SET;                                                     /* ��������̬����ѡ�� */

   delay_us(1);
    CS_1305_SET;                                                     /* Ƭѡ�ͷ� */
}

/**
 * @brief      ������ʾ GRAM ��ַ��ҳ��ַ + �е�ַ��
 * @param      page    ҳ��ַ��ͨ�� 0~7��
 * @param      col     �е�ַ��0~127��
 * @retval     None
 */
void Set_Addr(u8 page, u8 col)
{
    Write_Cmd(0xb0 + page);                                          /* ����ҳ��ʼ��ַ��Page Start Address�� */
    Write_Cmd(col & 0x0F);                                           /* �����е� 4 λ��Lower Column Start Address�� */
    Write_Cmd(0x10 | (col >> 4));                                    /* �����и� 4 λ��Higher Column Start Address�� */
}

/*
void Full_on(void)
{
	u8  i=0, j=0;

	for (i = 0; i < MAX_ROW>>3; i++)
	{
		Set_Addr(i, 0);

		for (j = 0; j < MAX_COL; j++)
			Write_Data(0xff);
	}
	Delay(500);
}*/

/**
 * @brief      �������������Դ棨GRAM��д 0����ҳ�����У�
 * @param      None
 * @retval     None
 */
void clearscreen(void)
{
    u8 i = 0, j = 0;                                               /* ��/�м����� */

    for (i = 0; i < (MAX_ROW >> 3); i++)                           /* ��ҳ������ÿ 8 ������Ϊ 1 ҳ��page�� */
    {
        Set_Addr(i, 0);                                            /* ���õ�ǰҳ���д� 0 ��ʼ */

        for (j = 0; j < MAX_COL; j++)                              /* ������ҳ������ */
            Write_Data(0x00);                                      /* д 0 ������� */
    }
    delay_ms(10);                                                 /* �ʵ���ʱ���ȴ�Ӳ���ȶ�����λ��ƽ̨ʵ�ֶ����� */
}

/*
void Show_pic(u8 page, u8 a, u8 b)
{
	u8 i = 0, j = 0;

	for (i = 0; i < 2; i++)
	{
		Set_Addr(0xb0 + i + page, a, b);
		for (j = 0; j < 48; j++)
			Write_Data(UniV[i][j]);
	}
}*/

/*******************************************************************************/
/**
 * @brief      ������ (x, y) ��ʾһ�� 8x16 ASCII �ַ�����ҳ�߶ȣ�MSB��LSB��
 * @param      x         ����ʼλ�ã�0 ~ MAX_COL-1��
 * @param      y         ����ʼ���أ�0 ~ MAX_ROW������ 8 �ж���ʹ�ã���ҳ = y>>3����ҳ = (y>>3)+1
 * @param      asc_code  ASCII �루ͨ�� 0x20~0x7F�����ڲ����ȥ 0x20 ����ģ����
 * @param      fb        ��ɫ��ʾ��־��true ��ɫ��XOR 0xFF����false ����
 * @retval     None
 */
void Disp_Char(u8 x, u8 y, u8 asc_code, bool fb)
{
    u8  i = 0;                                                       /* ���ڼ�������ÿҳд 8 �У� */
    u8  mask;                                                        /* ��ɫ���룺0 �� 0xFF */
    const uc8 *pcode;                                                /* ָ����ģ���ݵ�ָ�� */

    if (x >= MAX_COL || y > MAX_ROW)    return;                      /* Խ�籣�� */

    asc_code -= 0x20;                                                /* �� ASCII ��ת��Ϊ��ģ���±� */
    pcode = asc[asc_code].Msk;                                       /* ȡ����Ӧ�ַ�����ģָ�� */

    if (fb)   mask = 0xFF;                                           /* ��ɫ�����ֽ�ȡ����XOR 0xFF�� */
    else      mask = 0x00;                                           /* ������ʾ */

    Set_Addr((y >> 3), x);                                           /* ���õ��ϰ벿��ҳ��ַ���� 8 �У� */
    for (i = 0; i < 8; i++)                                          /* ����д�� 8 �У��е�ַ���� */
        Write_Data((*pcode++) ^ mask);                               /* д��ҳ���� */

    Set_Addr((y >> 3) + 1, x);                                       /* �л����°벿��ҳ��ַ���� 8 �У� */
    for (i = 0; i < 8; i++)                                          /* ����д�� 8 �У��е�ַ���� */
        Write_Data((*pcode++) ^ mask);                               /* д��ҳ���� */
}


/**
 * @brief      ��ʾ 32x48 �����ַ�����6 ҳ �� 32 �У���֧�ַ�ɫ
 * @param      x         ����ʼλ�ã�0 ~ MAX_COL-1��
 * @param      y         ����ʼ���أ�0 ~ MAX_ROW��
 * @param      asc_code  ���ִ��룺0~9������ֵӳ�䵽 asc_B[10]��ռλ/���ţ�
 * @param      fb        ��ɫ��ʾ��true ��ɫ��false ����
 * @retval     None
 */
void Disp_CharB(u8 x, u8 y, u8 asc_code, bool fb)
{
    u8  i = 0, j;                                                   /* ��/�м����� */
    u8  mask;                                                       /* ��ɫ���룺0 �� 0xFF */
   const uc8 *pcode;                                                /* ָ�� 32x48 ��ģ���� */

    if (x >= MAX_COL || y > MAX_ROW)    return;                     /* Խ�籣�� */

    if (asc_code > 9)                                               /* �� 0~9 ʱ��ӳ��Ϊ���� 10 */
        asc_code = 10;

    pcode = asc_B[asc_code].Msk;                                    /* ȡ��ģ��ʼ��ַ */

    if (fb)    mask = 0xFF;                                         /* ��ɫ��XOR 0xFF */
    else       mask = 0x00;                                         /* ������ʾ */

    for (i = 0; i < 6; i++)                                         /* �� 6 ҳ��6��8=48 �У� */
    {
        Set_Addr((y >> 3) + i, x);                                  /* ���õ�ǰҳ��ַ���д� x ��ʼ�� */
        for (j = 0; j < 32; j++)                                    /* ÿҳд�� 32 �� */
            Write_Data((*pcode++) ^ mask);                          /* ���ֽ������ģ���� */
    }
}


/**
 * @brief      ��ʾһ�� 16x16 ���֣���ҳ�߶ȣ���֧�ַ�ɫ
 * @param      x         ����ʼλ�ã�0 ~ MAX_COL-1��
 * @param      y         ����ʼ���أ�0 ~ MAX_ROW��
 * @param      gb_code   �ַ�����/���루���� GB_16 ��
 * @param      fb        ��ɫ��־��true ��ɫ��XOR 0xFF����false ����
 * @retval     None
 */
void dispHzChar(u8 x, u8 y, u16 gb_code, bool fb)
{
    u8  i = 0;                                                     /* ���ڼ�������ÿҳд 16 �� */
    u8  mask;                                                      /* ��ɫ���룺0 �� 0xFF */
    const uc8 *pcode;                                              /* ָ�� 16x16 ������ģ */

    if (x >= MAX_COL || y > MAX_ROW)    return;                    /* Խ�籣�� */

    /* asc_code -= '0';  ����ԭע�ͣ���Ч���룬�������ڵ��ԣ� */                     /* ˵����ע�� */
    pcode = GB_16[gb_code].Msk;                                    /* ȡ��ģָ�� */

    if (fb)   mask = 0xFF;                                         /* ��ɫ�����ֽ�ȡ����XOR 0xFF�� */
    else      mask = 0x00;                                         /* ������ʾ */

    Set_Addr((y >> 3), x);                                         /* �ϰ���ҳ��ַ���� 8 �У� */
    for (i = 0; i < 16; i++)                                       /* ����д�� 16 �� */
        Write_Data((*pcode++) ^ mask);                             /* ����ϰ벿������ */

    Set_Addr((y >> 3) + 1, x);                                     /* �°���ҳ��ַ���� 8 �У� */
    for (i = 0; i < 16; i++)                                       /* ����д�� 16 �� */
        Write_Data((*pcode++) ^ mask);                             /* ����°벿������ */
}


/**
 * @brief      �� GB_16 ��ģ���а� 2 �ֽ�����������Ŀ
 * @param      da       ָ�� 2 ���ֽڵ����������ֽ��� da[0]�����ֽ��� da[1]��
 * @retval     �����ҵ����±꣨��Χ 1..NumOfGB16����δ�ҵ����� 0
 */
u16 searchIndex(uc8 da[2])
{
    u16 i;                                                                 /* �����±꣺1..NumOfGB16 */

    for (i = 1; i <= NumOfGB16; i++)
        if (da[0] == GB_16[i].Index[0])
            if (da[1] == GB_16[i].Index[1])
                return i;                                                  /* �����򷵻��±� */

    return 0;                                                              /* δ�ҵ� */
}

/**
 * @brief      �� (x, y) λ�ð����ŷ�ʽ��ʾ�ַ�����ASCII 8x16 + ���� 16x16��
 * @param      x        ����ʼλ�ã�0 ~ MAX_COL-1��
 * @param      y        ����ʼ���أ�0 ~ MAX_ROW��
 * @param      str      �� '\0' ��β���ַ�����ASCII(<0x80) �� GB16(>=0x80 ��˫�ֽ�) ����
 * @param      Fb       ��ɫ��־��true ��ɫ��false ����
 * @retval     None
 */
void DispString(u8 x, u8 y, cchar* str, bool Fb)
{
    /* u16 max_cnt = 0;  ����ͳ����д����ֽ����������øñ��� */                             /* ˵����ע�� */
    u16 hz_index = 0;                                                      /* ������ GB_16 ���е��±� */

    while (*str)
    {
        if (*str < 0x80)                                                   /* ASCII �ַ���8x16�� */
        {
            Disp_Char(x, y, *str, Fb);
            x += 8;
            str++;
            /* max_cnt++; */
        }
        else if ((*str >= 0x80) && (*(str + 1) >= 0x80))                   /* ���֣�ʹ��������λ�ֽ� */
        {
            hz_index = searchIndex((uc8*)str);                             /* ���� GB16 ��ģ���� */
            dispHzChar(x, y, hz_index, Fb);                                /* ��� 16x16 ���֣�δ�ҵ�ʱ hz_index=0�� */
            x += 16;
            str += 2;
            /* max_cnt += 2; */
        }
        else                                                               /* ����������ı��룬����һ���ֽ� */
        {
            x += 8;                                                        /* ��ռλ���ǰ����������ѭ�� */
            str++;
        }
    }
}

/**
 * @brief      ʹ�á����ַ�����32x48��6 ҳ �� 32 �У���ʾ�ַ���
 * @param      x        ����ʼλ�ã�0 ~ MAX_COL-1��
 * @param      y        ����ʼ���أ�0 ~ MAX_ROW��
 * @param      str      �ַ�����ÿ���ַ�����һ�� Disp_CharB�����鴫������ 0~9��
 * @param      Fb       ��ɫ��־��true ��ɫ��false ����
 * @retval     None
 */
void DispStringB(u8 x, u8 y, cchar* str, bool Fb)
{
    while (*str)
    {
        if (*str < 0x80)                                                  /* ASCII ��������ʾ�ַ� */
            Disp_CharB(x, y, *str, Fb);                                   /* ע�⣺Disp_CharB �� >9 ��ӳ��Ϊ���� 10 */

        x += 32;                                                          /* ���ַ����Ϊ 32 �� */
        str++;
    }
}

/**
 * @brief      ��ʾ��� maxFb λ���޷���������֧�̶ֹ�С�����뵥λ���ԣ�
 * @param      x        ����ʼ���꣨��λ�� PAGE �궨�壬ͨ��Ϊ��/���أ�
 * @param      y        ����ʼ��������
 * @param      maxFb    ����ʾλ������������С�����ֵ�����λ������ 10��С��ʵ��λ����ضϸ�λ����������ಹ 0��
 * @param      Da       ����ʾ�����ݣ��޷��� 32 λ��
 * @param      PosFb    ����λ��������0 ���㣬0 ��ʾ����ࣻ�����ڵ�ǰ���λʱ���ԣ�
 * @param      Point    С��λ����0 ��ʾ��С���㣻>0 ʱС����λ�ڴ������� maxFb-Point ������֮��
 * @retval     None
 */
void Disp_Word_U(u16 x, u16 y, u8 maxFb, u32 Da, u8 PosFb, u8 Point)
{
    u32 NUM = 1;                                                     /* ��ǰλ��Ӧ�� 10 ���ݣ����λ�� */
    u8  i;                                                           /* λ������ */

    if (maxFb > 10)  maxFb = 10;                                     /* ��ȫ���ƣ������ʾ 10 λ */

    for (i = 1; i < maxFb; i++)  NUM *= 10;                          /* ��ʼ�� NUM = 10^(maxFb-1) */

    if (Point)                                                       /* ����С��λ���Ȼ�С���㣨ռ 1 �п�ȣ� */
        Disp_Char(x + PAGE(maxFb - Point), y, '.', false);

    for (i = 0; i < maxFb; i++)                                      /* �����λ�����λ����������� */
    {
        if (i < (maxFb - Point))                                     /* С����֮ǰ�����������֣� */
            Disp_Char(x + PAGE(i), y, (u8)(Da / NUM % 10) + '0', i == PosFb);
        else                                                         /* С����֮����λ������ƫ�� 1 ��������С���� */
            Disp_Char(x + PAGE(i + 1), y, (u8)(Da / NUM % 10) + '0', i == PosFb);

        NUM /= 10;                                                   /* �ƶ�����һλ����һλ�� */
    }
}

/**
 * @brief      ��ʾ��� maxFb λ�Ĵ������������̶�С���㣩��֧�ַ����벹��
 * @param      x        ����ʼ���꣨����λռ 1 λ�����Ϊ����λ��
 * @param      y        ����ʼ��������
 * @param      maxFb    ��ʾ�ġ�����λ����������������С���㣩����� 10��������ಹ 0
 * @param      Da       ����ʾ�����ݣ��з��� 32 λ��
 * @param      PosFb    ����λ�ã�0 ��ʾ����λ��1..maxFb ��ʾ��Ӧ����λ
 * @param      Point    С��λ����0 ��ʾ��С���㣻>0 ʱС����λ�ڴ������� maxFb-Point λ����֮��
 * @retval     None
 */
void Disp_Word_S(u8 x, u8 y, u8 maxFb, s32 Da, u8 PosFb, u8 Point)
{
    u32 div = 1;                                                    /* ��ǰλ�ĳ�����10^(maxFb-1) */
    u32 mag;                                                        /* |Da| ���޷��ŷ�ֵ */
    u8  i;                                                          /* λ������ */

    if (x >= MAX_COL)    return;                                    /* Խ�籣��������� X */

    if (maxFb > 10)  maxFb = 10;                                    /* ��ȫ���ޣ���� 10 λ���� */

    /* �����ֵ��ע�⣺����ʾ��Χ��9 λ��ͨ�����漰 INT_MIN ��ȡ������� */
    mag = (Da < 0) ? (u32)(-Da) : (u32)Da;                          /* ȡ����ֵ����ȡ��λ���� */

    for (i = 1; i < maxFb; i++)  div *= 10;                         /* div = 10^(maxFb-1) */

    if (Point)                                                      /* ����С��λ�����ڶ�Ӧ�л���С���� */
        Disp_Char(x + PAGE(maxFb - Point + 1), y, '.', false);

    /* ����λ��PosFb==0 ʱ���ԣ�Da==0 ʱ��ʾ�ո񣨱��ֶ����� */
    if (Da)
        Disp_Char(x, y, (Da < 0) ? '-' : '+', (PosFb == 0));        /* ��ʾ +/- */
    else
        Disp_Char(x, y, ' ',                 (PosFb == 0));         /* 0 ʱ����ʾ���� */

    /* ��λ������֣������λ�����λ������಻��λ�� 0 */
    for (i = 0; i < maxFb; i++)
    {
        u8 d = (u8)((mag / div) % 10);                              /* ȡ��ǰλ���� */

        if (i < (maxFb - Point))                                    /* С����֮ǰ�����������֣� */
            Disp_Char(x + PAGE(i + 1), y, (u8)('0' + d), (i + 1 == PosFb));
        else                                                        /* С����֮����λ���������� 1 �� */
            Disp_Char(x + PAGE(i + 2), y, (u8)('0' + d), (i + 1 == PosFb));

        div /= 10;                                                  /* �Ƶ���һλ����һλ�� */
    }
}

/**
 * @brief      ʹ�� 32x48�����ַ�����ʾ��� maxFb λ�޷�������֧��С��λ���룬Ĭ�ϲ���С���㣩
 * @param      x        ����ʼ���꣨���أ�
 * @param      y        ����ʼ��������
 * @param      maxFb    ����ʾ������λ�������� 10��������ಹ 0��
 * @param      Da       ����ʾ�����ݣ�u32��
 * @param      PosFb    ����λ�ã�0..maxFb-1����������λ��Ӧ
 * @param      Point    С��λ����0 ��ʾ��С����>0 ʱС����λ�ڴ������� maxFb-Point λ֮��
 * @retval     None
 *
 * @note       ���������� Disp_CharB ��� 0~9���ֿ�δ���� '.' ʱ������λ�������ƴ���
 *             ����Ҫ��ʾ���С���㣬�����ֿ� asc_B[10] �����õ��󲢸�Ϊ������� 10��
 */
void Disp_Word_UB(u16 x, u16 y, u8 maxFb, u32 Da, u8 PosFb, u8 Point)
{
    u32 NUM = 1;                                                     /* 10 ���ݣ���ǰ���λ���� */
    u8  i;                                                           /* λ������ */

    if (maxFb == 0)  return;                                         /* ������ʾ */
    if (maxFb > 10)  maxFb = 10;                                     /* ��ȫ���ޣ���� 10 λ */
    if (Point > maxFb) Point = maxFb;                                /* �淶��С��λ�� */

    for (i = 1; i < maxFb; i++)  NUM *= 10;                          /* ��ʼ�� NUM = 10^(maxFb-1) */

    /* ���ֿ�֧�ִ��С���㣬���������´��룬�� (maxFb-Point) �� (maxFb-Point+1) λ֮����� '.' */
    /* if (Point)                                                     */ /* ���С����λ��������ʾС���� */
    /*     Disp_CharB(x + (maxFb - Point) * 32, y, 10, false);        */ /* �ٶ� asc_B[10] Ϊ '.' �ĵ��� */

    for (i = 0; i < maxFb; i++)                                      /* ��λ����λ������� */
    {
        u8 digit = (u8)((Da / NUM) % 10);                            /* ȡ��ǰλ���� */

        if (i < (maxFb - Point))                                     /* С����֮ǰ�����������֣� */
            Disp_CharB(x + i * 32,      y, digit, (i == PosFb));
        else                                                         /* С����֮���������� 1 ���ַ�λ */
            Disp_CharB(x + (i + 1) * 32, y, digit, (i == PosFb));

        NUM /= 10;                                                   /* �Ƶ���һλ */
    }
}

/**
 * @brief      ��ʾʮ���������֣���λ���󣩣���� 8 λ��FFFFFFFF��
 * @param      x        ����ʼ����
 * @param      y        ����ʼ��������
 * @param      maxFb    ����ʾ��λ����1~8����������ಹ 0
 * @param      Da       ����ʾ���ݣ�u32��
 * @param      Fb       ���Ա�־��true ��ɫ��false ����
 * @retval     None
 */
void Disp_Word_H(u8 x, u8 y, u8 maxFb, u32 Da, bool Fb)
{
    u8 i;                                                            /* λ���������ӵ�λ nibble ��ʼ */
    u8 ch;                                                           /* ����ַ���'0'..'9','A'..'F' */

    if (maxFb == 0)  return;                                         /* ������ʾ */
    if (maxFb > 8)   maxFb = 8;                                      /* ���ޱ�������� 8 ��ʮ������λ */

    for (i = 0; i < maxFb; i++)                                      /* ��λ����λȡֵ�����Ϊ��λ */
    {
        ch = (u8)(Da & 0x0F);                                        /* ȡ��� 4 λ��һ��ʮ������λ�� */
        ch = (ch > 9) ? (u8)('A' + ch - 10) : (u8)('0' + ch);        /* ת ASCII �ַ� */
        Disp_Char(x + PAGE(maxFb - i - 1), y, ch, Fb);               /* ����������̶���� */

        Da >>= 4;                                                    /* ���� 4 λ��������һλ */
    }
}

/**
 * @brief      ��ʾ 16 λ���������֣��Ӹ�λ����λ��'1'/'0' �� 8x16 �ַ���ʾ��
 * @param      y        ����ʼ��������
 * @param      Da       16 λ���ݣ���λ�����
 * @retval     None
 */
void Disp_Word_B(u8 y, u16 Da)
{
    u8  i, c;                                                        /* λ/�ַ���ʱ���� */
    u16 Num = 0x8000;                                                /* ���룬�����λ��ʼ */

    for (i = 0; i < 16; i++)                                         /* ������� 16 λ���ߡ��ͣ� */
    {
        if (Da & Num)    c = '1';
        else             c = '0';
        Num >>= 1;                                                   /* ���Ƶ���һλ */
        Disp_Char(PAGE(i), y, c, false);                             /* �ڵ� i λλ����ʾ�ַ� */
    }
}


/**
 * @brief      �ԡ�mm:ss����ʽ��ʾ���ʱ
 * @param      x        ����ʼ����
 * @param      y        ����ʼ��������
 * @retval     None
 *
 * @note       ���� Show_Word_U(x, y, value, maxFb, posFb, point) ��ʵ�֣�
 *             Լ����maxFb Ϊλ����posFb Ϊ����λ��point ΪС��λ������������Ϊ 0����
 */
void Disp_Time_Sec(u8 x, u8 y,u16 count)
{
    Show_Word_U(x,           y, count / 60, 3, 0, false);           /* �֣�3 λ��00~999 */
    Disp_Char   (x + PAGE(3), y, ':',        false);                /* ����ָ��� ':' */
    Show_Word_U(x + PAGE(4), y, count % 60, 2, 0, false);           /* �룺2 λ��00~59 */
}

/**
 * @brief      ��ʾ 5x8 С�ַ�����ҳ�߶ȣ���֧�� 0~9 �Ͳ��ַ���
 * @param      x         ����ʼ����
 * @param      y         ����ʼ��������
 * @param      asc_code  Դ�ַ���'0'~'9' �� . - + : % �ո������ַ���������ʹ�ã�
 * @param      fb        ��ɫ��־��true ��ɫ��false ����
 * @retval     None
 */
void Disp_CharS(u8 x, u8 y, u8 asc_code, bool fb)
{
    u8  i = 0, mask;                                                /* �м���/��ɫ���� */
    const uc8 *pcode;                                               /* ��ģָ�루5 �У� */

    if (x >= MAX_COL || y > MAX_ROW)    return;                     /* Խ�籣�� */

    if (asc_code >= '0' && asc_code <= '9')
        asc_code -= '0';                                            /* ӳ�䵽 0..9 */
    else
    {
        switch (asc_code)
        {
        case '.': asc_code = 10; break;
        case '-': asc_code = 11; break;
        case '+': asc_code = 12; break;
        case ':': asc_code = 13; break;
        case '%': asc_code = 14; break;
        case ' ': asc_code = 15; break;
        default: /* �����ַ�����ԭֵ��������ȷ���ֿ⸲�� */          break;
        }
    }

    pcode = asc_S[asc_code].Msk;                                    /* ��ȡ��ģ��ʼ��ַ */

    if (fb)   mask = 0xFF;                                          /* ��ɫ��XOR 0xFF */
    else      mask = 0x00;                                          /* ������ʾ */

    Set_Addr((y >> 3), x);                                          /* ��ҳ���壺��дһҳ��8 �У� */
    for (i = 0; i < 5; i++)                                         /* 5 ������ */
        Write_Data((*pcode++) ^ mask);
}


/**
 * @brief      ʹ�� 5x8 С�ַ���ʾ�������������̶�С���㡢��ಹ 0���ɶ�λ���ԣ�
 * @param      x        ����ʼ���꣨����ռ 1 ��С�ַ���ȣ�
 * @param      y        ����ʼ��������
 * @param      maxFb    ��ʾ�ġ�����λ����������������С���㣩����� 10��������ಹ 0
 * @param      Da       ����ʾ���ݣ�s32��
 * @param      PosFb    ����λ�ã�0 ��ʾ����λ��1..maxFb ��ʾ��Ӧ����λ
 * @param      Point    С��λ����0 ��ʾ��С���㣻>0 ʱС����λ�ڴ������� maxFb-Point λ����֮��
 * @retval     None
 */
void Disp_Word_SS(u8 x, u8 y, u8 maxFb, s32 Da, u8 PosFb, u8 Point)
{
    u32 div = 1;                                                    /* ��ǰλ������10^(maxFb-1) */
    u32 mag;                                                        /* |Da| ���޷��ŷ�ֵ */
    u8  i;                                                          /* λ������ */

    if (x >= MAX_COL)  return;                                      /* Խ�籣����X�� */
    if (maxFb > 10)    maxFb = 10;                                  /* ���ޱ��� */
    if (Point > maxFb) Point = maxFb;                               /* �淶��С��λ�� */

    /* ȡ��ֵ������λȡ����ע�⣺�����ܳ��� INT_MIN���������Լ����Χ�� */
    mag = (Da < 0) ? (u32)(-Da) : (u32)Da;                          /* ����ֵ */

    for (i = 1; i < maxFb; i++)  div *= 10;                         /* div = 10^(maxFb-1) */

    if (Point)                                                      /* С���㣺�������������֮����� '.' */
        Disp_CharS(x + 5 * (maxFb - Point + 1), y, '.', false);

    /* ����λ��Da��0 ʱ��ʾ +/-��Da==0 ��ʾ�ո񱣳ֶ��� */
    if (Da)
        Disp_CharS(x, y, (Da < 0) ? '-' : '+', (PosFb == 0));
    else
        Disp_CharS(x, y, ' ',                 (PosFb == 0));

    /* �������λ�������λ�����λ����಻��λ�� 0 */
    for (i = 0; i < maxFb; i++)
    {
        u8 d = (u8)((mag / div) % 10);                              /* ��ǰλ���� */

        if (i < (maxFb - Point))                                    /* С����֮ǰ�����������֣� */
            Disp_CharS(x + 5 * (i + 1), y, (u8)('0' + d), (i + 1 == PosFb));
        else                                                        /* С����֮���������� 1 ��С�ַ�λ */
            Disp_CharS(x + 5 * (i + 2), y, (u8)('0' + d), (i + 1 == PosFb));

        div /= 10;                                                  /* ��һλ */
    }
}

/**
 * @brief      ʹ�� 5x8 С�ַ���ʾ�޷����������̶�С���㡢��ಹ 0���ɶ�λ���ԣ�
 * @param      x        ����ʼ����
 * @param      y        ����ʼ��������
 * @param      maxFb    ��ʾ������λ������� 10����������ಹ 0
 * @param      Da       ����ʾ���ݣ�u32��
 * @param      PosFb    ����λ�ã�0..maxFb-1����������λ��Ӧ
 * @param      Point    С��λ����0 ��ʾ��С���㣻>0 ʱС����λ�ڴ������� maxFb-Point λ����֮��
 * @retval     None
 */
void Disp_Word_US(u16 x, u16 y, u8 maxFb, u32 Da, u8 PosFb, u8 Point)
{
    u32 div = 1;                                                    /* ��ǰλ������10^(maxFb-1) */
    u8  i;                                                          /* λ������ */

    if (maxFb == 0)  return;                                        /* ������ʾ */
    if (maxFb > 10)  maxFb = 10;                                    /* ���ޱ��� */
    if (Point > maxFb) Point = maxFb;                               /* �淶��С��λ�� */

    for (i = 1; i < maxFb; i++)  div *= 10;                         /* ��ʼ������ */

    if (Point)                                                      /* С���㣺λ�������������֮�� */
        Disp_CharS(x + 5 * (maxFb - Point), y, '.', false);

    for (i = 0; i < maxFb; i++)                                     /* ��λ������ߡ��ͣ� */
    {
        u8 d = (u8)((Da / div) % 10);                               /* ��ǰλ���� */

        if (i < (maxFb - Point))                                    /* С����֮ǰ�����������֣� */
            Disp_CharS(x + i * 5,       y, (u8)('0' + d), (i == PosFb));
        else                                                        /* С����֮���������� 1 ��С�ַ�λ */
            Disp_CharS(x + 5 * (i + 1), y, (u8)('0' + d), (i == PosFb));

        div /= 10;                                                  /* ��һλ */
    }
}


/**
 * @brief      ʹ�� 5x8 С�ַ���ʾ�ַ�����ASCII ���֣�
 * @param      x        ����ʼ����
 * @param      y        ����ʼ��������
 * @param      str      �� '\0' ��β���ַ��������� *str < 0x80 ʱ�����
 * @param      Fb       ��ɫ��־��true ��ɫ��false ����
 * @retval     None
 */
void DispStringS(u8 x, u8 y, cchar* str, bool Fb)
{
    while (*str)
    {
        if (*str < 0x80)                                            /* ASCII �ַ� */
            Disp_CharS(x, y, *str, Fb);

        x += 5;                                                     /* С�ַ��ȿ� 5 �� */
        str++;
    }
}

/**
 * @brief      �ԡ�mm:ss����ʽ��ʾ���ʱ��С�ַ� 5x8 �汾��
 * @param      x        ����ʼ����
 * @param      y        ����ʼ��������
 * @retval     None
 */
void Disp_Time_SecS(u8 x, u8 y,u16 count)
{
    Disp_Word_US(x,        y, 3, count / 60, 3, 0);                 /* �֣�3 λ��00~999 */
    Disp_CharS  (x + 15,   y, ':', false);                          /* 3*5 �к���� ':' */
    Disp_Word_US(x + 20,   y, 2, count % 60, 2, 0);                 /* �룺2 λ��00~59 */
}

/**
 * @brief      ��ʾ�޷���������8x16 �ַ����̶�С���㣻��ಹ 0��
 * @param      x        ����ʼ����
 * @param      y        ����ʼ��������
 * @param      Da       ����ʾ���ݣ�u32��
 * @param      maxFb    ��ʾ������λ������� 10����������ಹ 0
 * @param      Point    С��λ����0 ��ʾ��С���㣻>0 ʱС����λ�ڴ������� maxFb-Point λ����֮��
 * @param      fb       ��ɫ��־��true ��ɫ��false ����������������������С���㣩
 * @retval     None
 */
void Show_Word_U(u8 x, u8 y, u32 Da, u8 maxFb, u8 Point, bool fb)
{
    u32 NUM = 1;                                                    /* 10 ���ݣ���ǰ���λ���� */
    u8  i;                                                          /* λ������ */

    if (maxFb == 0)  return;                                        /* ������ʾ */
    if (maxFb > 10) maxFb = 10;                                     /* ���ޱ��� */
    if (Point > maxFb) Point = maxFb;                               /* �淶��С��λ�� */

    for (i = 1; i < maxFb; i++)                                     /* NUM = 10^(maxFb-1) */
        NUM *= 10;

    if (Point)                                                       /* ����С��λ���Ȼ���С���� */
        Disp_Char(x + PAGE(maxFb - Point), y, '.', fb);

    for (i = 0; i < maxFb; i++)                                      /* ��λ����λ������� */
    {
        u8 d = (u8)((Da / NUM) % 10);                               /* ��ǰλ���� */

        if (i < (maxFb - Point))                                    /* С����֮ǰ�����������֣� */
            Disp_Char(x + PAGE(i),     y, (u8)('0' + d), fb);
        else                                                        /* С����֮��λ���������� 1 λ */
            Disp_Char(x + PAGE(i + 1), y, (u8)('0' + d), fb);

        NUM /= 10;                                                  /* ��һλ */
    }
}

/**
 * @brief      ʹ�� 8x16 �ַ���ʾ�������������̶�С���㡢��ಹ 0��
 * @param      x        ����ʼ���꣨����ռ 1 λ�����Ϊ����λ��
 * @param      y        ����ʼ��������
 * @param      Da       ����ʾ���ݣ�s32��
 * @param      maxFb    ����λ��������������С���㣩����� 10��������ಹ 0
 * @param      Point    С��λ����0 ��ʾ��С���㣻>0 ʱС����λ�ڴ������� maxFb-Point λ����֮��
 * @param      fb       ��ɫ��־��true ��ɫ��false ����
 * @retval     None
 */
void Show_Word_S(u8 x, u8 y, s32 Da, u8 maxFb, u8 Point, bool fb)
{
    u32 div = 1;                                                    /* ��ǰλ������10^(maxFb-1) */
    u32 mag;                                                        /* |Da| ���޷��ŷ�ֵ */
    u8  i;                                                          /* λ������ */

    if (x >= MAX_COL)                return;                        /* Խ�籣����X�� */
    if (maxFb == 0)                  return;                        /* ������ʾ */
    if (maxFb > 10)                  maxFb = 10;                    /* ���ޱ��� */
    if (Point > maxFb)               Point = maxFb;                 /* �淶��С��λ�� */

    mag = (Da < 0) ? (u32)(-Da) : (u32)Da;                          /* ȡ��ֵ������λȡ�� */

    for (i = 1; i < maxFb; i++)  div *= 10;                         /* div = 10^(maxFb-1) */

    if (Point)                                                      /* ����С��λ���Ȼ���С���㣨���� 1 ������λ�� */
        Disp_Char(x + PAGE(maxFb - Point + 1), y, '.', fb);

    if (Da)  Disp_Char(x, y, (Da < 0) ? '-' : '+', fb);             /* ����λ��Da=0 ʱ��ʾ�ո񱣳ֶ��룩 */
    else     Disp_Char(x, y, ' ',                     fb);

    for (i = 0; i < maxFb; i++)                                     /* �����λ�����λ����������� */
    {
        u8 d = (u8)((mag / div) % 10);                              /* ȡ��ǰλ���� */

        if (i < (maxFb - Point))                                    /* С����֮ǰ�����������֣� */
            Disp_Char(x + PAGE(i + 1), y, (u8)('0' + d), fb);
        else                                                        /* С����֮���������� 1 λ���� '.' */
            Disp_Char(x + PAGE(i + 2), y, (u8)('0' + d), fb);

        div /= 10;                                                  /* ��һλ */
    }
}

/**
 * @brief      ���� x ���ӵײ����ϻ��Ƹ߶�Ϊ num ��ʵ��������֧��ȫ��ɫ
 * @param      x        �����꣨0 ~ MAX_COL-1��
 * @param      num      �����߶ȣ����أ�0 ~ MAX_ROW��
 * @param      Fb       ��ɫ��־��true ��ɫ��false ����
 * @retval     None
 *
 * @note       �Դ水ҳ��8 �����أ���֯��i Ϊҳ������0 ���� �� 7 �ײ�����
 *             �Ƚ� num ����Ϊ�Զ��˵ġ��հ׸߶ȡ������ҳд�� 0x00/0xFF ���߽�ҳ���롣
 */
void line_sign(u8 x, u8 num, bool Fb)
{
#if 0
    /* ��ʵ�֣���ҳд�룬������ 64 ����壨�����Ա�Աȣ� */
    num++;
    if (num >= MAX_ROW)
        num = MAX_ROW - 1;

    Set_Addr(((64 - num) >> 3), x);
    Write_Data(0xFF << ((64 - num) & 0x07));
#else
    u8  i, tMask, FbMask = 0;                                        /* ��ʱ���� / ��ɫ���� */
    if (Fb)  FbMask = 0xFF;                                          /* ��ɫ��������д��ֵ XOR */

    if (num > MAX_ROW)  num = MAX_ROW;                               /* �н��߶ȵ���Ļ��Χ */

    num = MAX_ROW - num;                                             /* ת��Ϊ�Զ��˵Ŀհ������� */

    for (i = 0; i < 8; i++)                                          /* ���� 8 ��ҳ��8��8=64 �У� */
    {
        Set_Addr(i, x);                                              /* ����ҳ��ַ���е�ַ */

        if (i < (num >> 3))                                          /* ��ҳ��ȫ�ڿհ���֮�ϣ�ȫ 0 */
            tMask = 0x00;
        else if (i == (num >> 3))                                    /* �߽�ҳ���ϲ��հס��²���� */
            tMask = (u8)(0xFF << (num & 0x07));
        else                                                         /* ��ҳ��������ڣ�ȫ 1 */
            tMask = 0xFF;

        tMask ^= FbMask;                                             /* ���� Fb ȡ������ɫ�� */
        Write_Data(tMask);                                           /* д�뵱ǰҳ���� */
    }
#endif
}

/**
 * @brief      ���� x ��������ҳ��8 ���أ��ֱ��ʵ����������ӵײ��������
 * @param      x        �����꣨0 ~ MAX_COL-1��
 * @param      num      ������ҳ����0 ~ 8����1 ҳ = 8 ���أ��Ե������ۼ�
 * @retval     None
 */
void set_Sign(u8 x, u8 num)
{
    u8 i;                                                            /* ҳ������0 ���� �� 7 �ײ� */

    if (x >= MAX_COL)  return;                                       /* Խ�籣����X�� */
    if (num > 8)       num = 8;                                      /* �н���Χ����� 8 ҳ */

    for (i = 0; i < 8; i++)
    {
        Set_Addr(i, x);                                              /* ���õ� i ҳ���� x */

        if (i > (u8)(7 - num))                                       /* �ײ� num ҳ��� 0xFF */
            Write_Data(0xFF);
        else
            Write_Data(0x00);
    }
}

/**
 * @brief      ��ָ��ҳ����д��1�ֽ�����
 * @param[in]  page: ҳ��ַ
 * @param[in]  col:  �е�ַ  
 * @param[in]  mask: �������루bit=1������Ӧ���أ�
 * @retval     None
 * @note       ����дģʽ���ǵ��ӣ�
 */
static inline void write_col_byte(u8 page, u8 col, u8 mask)
{
    Set_Addr(page, col);
    Write_Data(mask);
}

/**
 * @brief      �ڵ� y �����ش��� x0 �� x1 ��һ�� 1 ���غ��ˮƽ��
 * @param      x0   ��ʼ�У�������
 * @param      x1   �����У�������
 * @param      y    �кţ��������꣩
 * @param      Fb   ��ɫ/��ɫ��־��false=�������룬true=��ɫ����
 * @retval     ��
 *
 * ˵����
 * - �Դ水ҳ��8 ���ظߣ���֯���ȼ���ҳ����ҳ��λ�ţ�Ȼ���쵥��λ����д��ÿ�С�
 * - �����λ���෴���ɽ������� (1u << bit) ��Ϊ (0x80u >> bit)��
 * - Ϊ������Ч���ʣ�������� x0/x1 ���������ü�����ʾ�߽硣
 */
void draw_hline(u8 x0, u8 x1, u8 y)
{
    if (x0 > x1) { u8 t = x0; x0 = x1; x1 = t; }                               /* ��֤ x0 <= x1 */
    if (y >= MAX_ROW) return;                                                  /* ��Խ��ֱ�ӷ��� */
    if (x0 >= MAX_COL) return;                                                 /* ���Խ��ֱ�ӷ��� */
    if (x1 >= MAX_COL) x1 = (u8)(MAX_COL - 1);                                 /* �ü�β���� */

    u8 page = (u8)(y >> 3);                                                    /* ����ҳ */
    u8 bit  = (u8)(y & 0x07);                                                  /* ҳ�ڵڼ�λ��0~7�� */
    u8 mask = (u8)(1u << bit);                                                 /* ��λ���෴��Ϊ (0x80u >> bit) */
    
    Set_Addr(page, x0);                                                        /* �趨��ʼд��ַ */
    for (u8 x = x0; x <= x1; x++)                                              /* ����д�� */
    {
        Write_Data(mask);                                                      /* д����λ���� */
    }
}

/**
 * @brief      ��ָ���л����������ض�
 * @param[in]  x:  ������
 * @param[in]  y0: ��ʼ������
 * @param[in]  y1: ����������
 * @retval     None
 * @note       �Զ������ҳ��֣�y0/y1������˳��
 */
void draw_vspan(u8 x, u8 y0, u8 y1)
{
    /* ȷ�� y0 <= y1 */                                                             /* ��������ȷ��˳�� */
    if (y0 > y1)
    {
        u8 t = y0;
        y0 = y1;
        y1 = t;
    }
    
    /* �߽��� */                                                                  /* ������Ļ��Χ�򷵻� */
    if (x >= MAX_COL || y0 >= MAX_ROW)
    {
        return;
    }
    
    /* �ü�y1����Ļ��Χ�� */                                                         /* ��ֹԽ�� */
    if (y1 >= MAX_ROW)
    {
        y1 = MAX_ROW - 1;
    }

    /* ����ҳ��ַ��λƫ�� */                                                         /* ÿҳ8�� */
    u8 p0 = y0 >> 3;
    u8 p1 = y1 >> 3;
    u8 b0 = y0 & 7;
    u8 b1 = y1 & 7;

    if (p0 == p1)
    {
        /* ͬҳ��������λ���� */                                                   /* ����b0��b1֮���λ */
        u8 mask = (u8)((0xFFu << b0) & (0xFFu >> (7 - b1)));
        write_col_byte(p0, x, mask);
    }
    else
    {
        /* ��ҳ������b0��ҳ�� */                                                   /* ������ʼλ��ҳĩ */
        u8 first_mask = (u8)(0xFFu << b0);
        write_col_byte(p0, x, first_mask);

        /* �м���ҳ����ȫ��� */                                                   /* ����ҳȫ������ */
        for (u8 p = p0 + 1; p < p1; ++p)
        {
            write_col_byte(p, x, 0xFF);
        }

        /* ĩҳ������ҳ����b1 */                                                   /* ����ҳ��ʼ������λ */
        u8 last_mask = (u8)(0xFFu >> (7 - b1));
        write_col_byte(p1, x, last_mask);
    }
}

/**
 * @brief      Draw rectangle on display
 * @param[in]  x: Top-left x coordinate
 * @param[in]  y: Top-left y coordinate
 * @param[in]  w: Width of rectangle
 * @param[in]  h: Height of rectangle
 * @param[in]  fill: Fill mode (true = filled, false = outline)
 * @retval     None
 */
void draw_rect(u8 x, u8 y, u8 w, u8 h, bool fill)
{
    if (w == 0 || h == 0) return;

    /* Calculate bottom-right corner (inclusive) */                                 /* �������½����꣨������ */
    u8 x1 = x + w - 1;
    u8 y1 = y + h - 1;

    /* Clip to screen boundaries */                                                /* �ü�����Ļ�߽� */
    if (x >= MAX_COL || y >= MAX_ROW) return;
    if (x1 >= MAX_COL) x1 = MAX_COL - 1;
    if (y1 >= MAX_ROW) y1 = MAX_ROW - 1;

    if (fill)
    {
        /* Filled rectangle: draw vertical spans for each column */                 /* ʵ�ľ��Σ�ÿ�л��������� */
        for (u8 cx = x; cx <= x1; ++cx)
        {
            draw_vspan(cx, y, y1);
        }
    }
    else
    {
        /* Outline rectangle: two horizontal lines + two vertical lines */          /* ���ľ��Σ�����ˮƽ�� + ������ֱ�� */
        draw_hline(x, x1, y);                                                      /* Top edge ���� */
        draw_hline(x, x1, y1);                                                     /* Bottom edge �ױ� */

        if (h >= 3)                                                                /* Avoid overlapping with top/bottom */
        {
            draw_vspan(x, y + 1, y1 - 1);                                          /* Left edge ��� */
            if (w >= 2) draw_vspan(x1, y + 1, y1 - 1);                             /* Right edge �ұ� */
        }
        else if (w >= 2)
        {
            /* For small height (1 or 2 pixels), horizontal edges are sufficient */ /* �߶Ⱥ�Сʱ��ˮƽ�����㹻 */
            draw_vspan(x, y, y1);                                                  /* Left edge ��� */
            draw_vspan(x1, y, y1);                                                 /* Right edge �ұ� */
        }
    }
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Initialization
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SSD1305_init()
{
	RES_1305_CLR;
	delay_ms(100);
	RES_1305_SET;
	delay_ms(100);

	Write_Cmd(0xae);//Set Display ON/OFF =>0xae:Display off
	Write_Cmd(0x20);//Set Memory Addressing Mode
	Write_Cmd(horizontal);

	Write_Cmd(0x21);//Set Column Address Aange
	Write_Cmd(0);
	Write_Cmd(MAX_COL-1);

	Set_Addr(0, 0);

	Write_Cmd(0x81);//Set contrast Control for BANK0//�Աȶ�
	Write_Cmd(contrast);

	Write_Cmd(0x82);//Set Brightness for Area Color Banks
	Write_Cmd(Brightness);

	Write_Cmd(0x40);//Set Display Start Line from 0x40 to 0x7F
	Write_Cmd(0xa1);//Set Segment Re-map =>0xa0:column addr. 0 is mapped to SEG0//����
	Write_Cmd(0xa4);//Entire Display On =>0xa4:resume to RAM content display
	Write_Cmd(0xa6);//Set Normal/Inverse Display =>0xa6:Normal,0xa7:inverse//���Է���

	Write_Cmd(0xa8);//Set Multiplex Ratio
	Write_Cmd(0x3f);//63

	Write_Cmd(0xad);//Master Configuration
	Write_Cmd(0x8e);

	Write_Cmd(0xc8);//Set COM Output Scan Direction//����
					//0xc0:scan from COM0 to COM63, 0xc8:scan from COM63 to COM0

	Write_Cmd(0xd3);//Set Display Offset
	Write_Cmd(0x00);

	Write_Cmd(0xd5);//Set Display Clock Divid Ratio/Oscillator Frequency
	Write_Cmd(0xf0);

	Write_Cmd(0xd8);//Set Area Color Mode ON/OFF & Low Power Display Mode
	Write_Cmd(0x05);//0x05:monochrome mode and low power display mode

	Write_Cmd(0xd9);//Set Pre-charge Period
	Write_Cmd(0xf1);

	Write_Cmd(0xda);//Set COM Pins Hardware Configuration
	Write_Cmd(0x12);

	Write_Cmd(0xdb);//Set Vcomh Deselect Level
	Write_Cmd(0x3C);// 00h~ 0.43 x Vcc
					// 34h~ 0.77 x Vcc (RESET)
					// 3Ch~ 0.83 x Vcc

	Write_Cmd(0x2e);//deactivate scroll

	Write_Cmd(0xaf);//Set Display ON/OFF =>0xaf:Display on
}


