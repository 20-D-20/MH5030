#include "ssd1305.h"

//从SPI口输出一字节数据
/**
 * @brief      通过 GPIO 模拟 SPI 向外设写入 1 字节（MSB first）
 * @param      data    待发送的数据字节
 * @retval     None
 */
void spi_write(u8 data)
{
    u8 i;                                                            /* 循环计数器 */

    for (i = 0; i < 8; i++)
    {
       delay_us(1);                                                  /* 短延时，保证时序余量 */
        CLK_1305_CLR;                                                /* 拉低时钟：准备设置数据位 */

        /* 根据最高位输出数据（MSB first） */
       delay_us(1);
        if (data & 0x80)    DI_1305_SET;
        else                DI_1305_CLR;

        data <<= 1;                                                  /* 左移 1 位，准备下一位 */

       delay_us(1);                                                  /* 短延时，确保数据稳定 */
        CLK_1305_SET;                                                /* 上升沿锁存数据位 */
    }
}

/**
 * @brief      发送一字节命令到外设（DC=0）
 * @param      cmd     命令字节
 * @retval     None
 */
void Write_Cmd(u8 cmd)
{
   delay_us(1);                                                         /* 时序余量 */
    CS_1305_CLR;                                                     /* 片选使能 */

   delay_us(1);
    DC_1305_CLR;                                                     /* 指示后续为命令 */

   delay_us(1);
    spi_write(cmd);                                                  /* 发送命令 */

   delay_us(1);
    DC_1305_SET;                                                     /* 恢复为数据态（可选） */

   delay_us(1);
    CS_1305_SET;                                                     /* 片选释放 */
}

/**
 * @brief      发送一字节数据到外设（DC=1）
 * @param      data    数据字节
 * @retval     None
 */
void Write_Data(u8 data)
{
   delay_us(1);                                                         /* 时序余量 */
    CS_1305_CLR;                                                     /* 片选使能 */

   delay_us(1);
    DC_1305_SET;                                                     /* 指示后续为数据 */

   delay_us(1);
    spi_write(data);                                                 /* 发送数据 */

   delay_us(1);
    DC_1305_SET;                                                     /* 保持数据态（可选） */

   delay_us(1);
    CS_1305_SET;                                                     /* 片选释放 */
}

/**
 * @brief      设置显示 GRAM 地址（页地址 + 列地址）
 * @param      page    页地址（通常 0~7）
 * @param      col     列地址（0~127）
 * @retval     None
 */
void Set_Addr(u8 page, u8 col)
{
    Write_Cmd(0xb0 + page);                                          /* 设置页起始地址（Page Start Address） */
    Write_Cmd(col & 0x0F);                                           /* 设置列低 4 位（Lower Column Start Address） */
    Write_Cmd(0x10 | (col >> 4));                                    /* 设置列高 4 位（Higher Column Start Address） */
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
 * @brief      清屏：将整个显存（GRAM）写 0（逐页、逐列）
 * @param      None
 * @retval     None
 */
void clearscreen(void)
{
    u8 i = 0, j = 0;                                               /* 行/列计数器 */

    for (i = 0; i < (MAX_ROW >> 3); i++)                           /* 逐页清屏：每 8 行像素为 1 页（page） */
    {
        Set_Addr(i, 0);                                            /* 设置当前页，列从 0 开始 */

        for (j = 0; j < MAX_COL; j++)                              /* 遍历该页所有列 */
            Write_Data(0x00);                                      /* 写 0 清空像素 */
    }
    delay_ms(10);                                                 /* 适当延时，等待硬件稳定（单位依平台实现而定） */
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
 * @brief      在坐标 (x, y) 显示一个 8x16 ASCII 字符（两页高度，MSB→LSB）
 * @param      x         列起始位置（0 ~ MAX_COL-1）
 * @param      y         行起始像素（0 ~ MAX_ROW），按 8 行对齐使用：上页 = y>>3，下页 = (y>>3)+1
 * @param      asc_code  ASCII 码（通常 0x20~0x7F），内部会减去 0x20 做字模索引
 * @param      fb        反色显示标志：true 反色（XOR 0xFF），false 正常
 * @retval     None
 */
void Disp_Char(u8 x, u8 y, u8 asc_code, bool fb)
{
    u8  i = 0;                                                       /* 行内计数器（每页写 8 列） */
    u8  mask;                                                        /* 反色掩码：0 或 0xFF */
    const uc8 *pcode;                                                /* 指向字模数据的指针 */

    if (x >= MAX_COL || y > MAX_ROW)    return;                      /* 越界保护 */

    asc_code -= 0x20;                                                /* 将 ASCII 码转换为字模表下标 */
    pcode = asc[asc_code].Msk;                                       /* 取出对应字符的字模指针 */

    if (fb)   mask = 0xFF;                                           /* 反色：逐字节取反（XOR 0xFF） */
    else      mask = 0x00;                                           /* 正常显示 */

    Set_Addr((y >> 3), x);                                           /* 设置到上半部分页地址（高 8 行） */
    for (i = 0; i < 8; i++)                                          /* 连续写入 8 列，列地址自增 */
        Write_Data((*pcode++) ^ mask);                               /* 写上页数据 */

    Set_Addr((y >> 3) + 1, x);                                       /* 切换到下半部分页地址（低 8 行） */
    for (i = 0; i < 8; i++)                                          /* 连续写入 8 列，列地址自增 */
        Write_Data((*pcode++) ^ mask);                               /* 写下页数据 */
}


/**
 * @brief      显示 32x48 “大字符”（6 页 × 32 列），支持反色
 * @param      x         列起始位置（0 ~ MAX_COL-1）
 * @param      y         行起始像素（0 ~ MAX_ROW）
 * @param      asc_code  数字代码：0~9；其他值映射到 asc_B[10]（占位/符号）
 * @param      fb        反色显示：true 反色，false 正常
 * @retval     None
 */
void Disp_CharB(u8 x, u8 y, u8 asc_code, bool fb)
{
    u8  i = 0, j;                                                   /* 行/列计数器 */
    u8  mask;                                                       /* 反色掩码：0 或 0xFF */
   const uc8 *pcode;                                                /* 指向 32x48 字模数据 */

    if (x >= MAX_COL || y > MAX_ROW)    return;                     /* 越界保护 */

    if (asc_code > 9)                                               /* 非 0~9 时，映射为索引 10 */
        asc_code = 10;

    pcode = asc_B[asc_code].Msk;                                    /* 取字模起始地址 */

    if (fb)    mask = 0xFF;                                         /* 反色：XOR 0xFF */
    else       mask = 0x00;                                         /* 正常显示 */

    for (i = 0; i < 6; i++)                                         /* 共 6 页（6×8=48 行） */
    {
        Set_Addr((y >> 3) + i, x);                                  /* 设置当前页地址（列从 x 开始） */
        for (j = 0; j < 32; j++)                                    /* 每页写入 32 列 */
            Write_Data((*pcode++) ^ mask);                          /* 逐字节输出字模数据 */
    }
}


/**
 * @brief      显示一个 16x16 汉字（两页高度），支持反色
 * @param      x         列起始位置（0 ~ MAX_COL-1）
 * @param      y         行起始像素（0 ~ MAX_ROW）
 * @param      gb_code   字符索引/编码（用于 GB_16 表）
 * @param      fb        反色标志：true 反色（XOR 0xFF），false 正常
 * @retval     None
 */
void dispHzChar(u8 x, u8 y, u16 gb_code, bool fb)
{
    u8  i = 0;                                                     /* 行内计数器：每页写 16 列 */
    u8  mask;                                                      /* 反色掩码：0 或 0xFF */
    const uc8 *pcode;                                              /* 指向 16x16 点阵字模 */

    if (x >= MAX_COL || y > MAX_ROW)    return;                    /* 越界保护 */

    /* asc_code -= '0';  保留原注释（无效代码，可能用于调试） */                     /* 说明性注释 */
    pcode = GB_16[gb_code].Msk;                                    /* 取字模指针 */

    if (fb)   mask = 0xFF;                                         /* 反色：按字节取反（XOR 0xFF） */
    else      mask = 0x00;                                         /* 正常显示 */

    Set_Addr((y >> 3), x);                                         /* 上半区页地址（高 8 行） */
    for (i = 0; i < 16; i++)                                       /* 连续写入 16 列 */
        Write_Data((*pcode++) ^ mask);                             /* 输出上半部分数据 */

    Set_Addr((y >> 3) + 1, x);                                     /* 下半区页地址（低 8 行） */
    for (i = 0; i < 16; i++)                                       /* 连续写入 16 列 */
        Write_Data((*pcode++) ^ mask);                             /* 输出下半部分数据 */
}


/**
 * @brief      在 GB_16 字模表中按 2 字节索引查找条目
 * @param      da       指向 2 个字节的索引（高字节在 da[0]，低字节在 da[1]）
 * @retval     返回找到的下标（范围 1..NumOfGB16），未找到返回 0
 */
u16 searchIndex(uc8 da[2])
{
    u16 i;                                                                 /* 遍历下标：1..NumOfGB16 */

    for (i = 1; i <= NumOfGB16; i++)
        if (da[0] == GB_16[i].Index[0])
            if (da[1] == GB_16[i].Index[1])
                return i;                                                  /* 命中则返回下标 */

    return 0;                                                              /* 未找到 */
}

/**
 * @brief      在 (x, y) 位置按混排方式显示字符串（ASCII 8x16 + 汉字 16x16）
 * @param      x        列起始位置（0 ~ MAX_COL-1）
 * @param      y        行起始像素（0 ~ MAX_ROW）
 * @param      str      以 '\0' 结尾的字符串；ASCII(<0x80) 与 GB16(>=0x80 的双字节) 混排
 * @param      Fb       反色标志：true 反色，false 正常
 * @retval     None
 */
void DispString(u8 x, u8 y, cchar* str, bool Fb)
{
    /* u16 max_cnt = 0;  若需统计已写入的字节数，可启用该变量 */                             /* 说明性注释 */
    u16 hz_index = 0;                                                      /* 汉字在 GB_16 表中的下标 */

    while (*str)
    {
        if (*str < 0x80)                                                   /* ASCII 字符（8x16） */
        {
            Disp_Char(x, y, *str, Fb);
            x += 8;
            str++;
            /* max_cnt++; */
        }
        else if ((*str >= 0x80) && (*(str + 1) >= 0x80))                   /* 汉字：使用两个高位字节 */
        {
            hz_index = searchIndex((uc8*)str);                             /* 查找 GB16 字模索引 */
            dispHzChar(x, y, hz_index, Fb);                                /* 输出 16x16 汉字（未找到时 hz_index=0） */
            x += 16;
            str += 2;
            /* max_cnt += 2; */
        }
        else                                                               /* 错误或不完整的编码，跳过一个字节 */
        {
            x += 8;                                                        /* 以占位宽度前进，避免死循环 */
            str++;
        }
    }
}

/**
 * @brief      使用“大字符”（32x48，6 页 × 32 列）显示字符串
 * @param      x        列起始位置（0 ~ MAX_COL-1）
 * @param      y        行起始像素（0 ~ MAX_ROW）
 * @param      str      字符串；每个字符调用一次 Disp_CharB（建议传入数字 0~9）
 * @param      Fb       反色标志：true 反色，false 正常
 * @retval     None
 */
void DispStringB(u8 x, u8 y, cchar* str, bool Fb)
{
    while (*str)
    {
        if (*str < 0x80)                                                  /* ASCII 视作可显示字符 */
            Disp_CharB(x, y, *str, Fb);                                   /* 注意：Disp_CharB 里 >9 会映射为索引 10 */

        x += 32;                                                          /* 大字符宽度为 32 列 */
        str++;
    }
}

/**
 * @brief      显示最多 maxFb 位的无符号整数（支持固定小数点与单位反显）
 * @param      x        列起始坐标（单位依 PAGE 宏定义，通常为列/像素）
 * @param      y        行起始像素坐标
 * @param      maxFb    总显示位数（含整数与小数部分的数字位；上限 10，小于实际位数会截断高位，大于则左侧补 0）
 * @param      Da       待显示的数据（无符号 32 位）
 * @param      PosFb    反显位置索引（0 起算，0 表示最左侧；当等于当前输出位时反显）
 * @param      Point    小数位数（0 表示无小数点；>0 时小数点位于从左数第 maxFb-Point 个数字之后）
 * @retval     None
 */
void Disp_Word_U(u16 x, u16 y, u8 maxFb, u32 Da, u8 PosFb, u8 Point)
{
    u32 NUM = 1;                                                     /* 当前位对应的 10 的幂（最高位起） */
    u8  i;                                                           /* 位计数器 */

    if (maxFb > 10)  maxFb = 10;                                     /* 安全限制：最多显示 10 位 */

    for (i = 1; i < maxFb; i++)  NUM *= 10;                          /* 初始化 NUM = 10^(maxFb-1) */

    if (Point)                                                       /* 若有小数位，先画小数点（占 1 列宽度） */
        Disp_Char(x + PAGE(maxFb - Point), y, '.', false);

    for (i = 0; i < maxFb; i++)                                      /* 从最高位到最低位依次输出数字 */
    {
        if (i < (maxFb - Point))                                     /* 小数点之前（含整数部分） */
            Disp_Char(x + PAGE(i), y, (u8)(Da / NUM % 10) + '0', i == PosFb);
        else                                                         /* 小数点之后：列位置向右偏移 1 列以跳过小数点 */
            Disp_Char(x + PAGE(i + 1), y, (u8)(Da / NUM % 10) + '0', i == PosFb);

        NUM /= 10;                                                   /* 移动到下一位（低一位） */
    }
}

/**
 * @brief      显示最多 maxFb 位的带符号整数（固定小数点），支持反显与补零
 * @param      x        列起始坐标（符号位占 1 位，其后为数字位）
 * @param      y        行起始像素坐标
 * @param      maxFb    显示的“数字位数”（不含符号与小数点），最大 10；不足左侧补 0
 * @param      Da       待显示的数据（有符号 32 位）
 * @param      PosFb    反显位置：0 表示符号位，1..maxFb 表示对应数字位
 * @param      Point    小数位数（0 表示无小数点；>0 时小数点位于从左数第 maxFb-Point 位数字之后）
 * @retval     None
 */
void Disp_Word_S(u8 x, u8 y, u8 maxFb, s32 Da, u8 PosFb, u8 Point)
{
    u32 div = 1;                                                    /* 当前位的除数：10^(maxFb-1) */
    u32 mag;                                                        /* |Da| 的无符号幅值 */
    u8  i;                                                          /* 位计数器 */

    if (x >= MAX_COL)    return;                                    /* 越界保护：仅检查 X */

    if (maxFb > 10)  maxFb = 10;                                    /* 安全上限：最多 10 位数字 */

    /* 计算幅值（注意：本显示范围≤9 位，通常不涉及 INT_MIN 的取反溢出） */
    mag = (Da < 0) ? (u32)(-Da) : (u32)Da;                          /* 取绝对值用于取各位数字 */

    for (i = 1; i < maxFb; i++)  div *= 10;                         /* div = 10^(maxFb-1) */

    if (Point)                                                      /* 若有小数位，先在对应列绘制小数点 */
        Disp_Char(x + PAGE(maxFb - Point + 1), y, '.', false);

    /* 符号位：PosFb==0 时反显；Da==0 时显示空格（保持对齐风格） */
    if (Da)
        Disp_Char(x, y, (Da < 0) ? '-' : '+', (PosFb == 0));        /* 显示 +/- */
    else
        Disp_Char(x, y, ' ',                 (PosFb == 0));         /* 0 时不显示符号 */

    /* 逐位输出数字（从最高位到最低位），左侧不足位补 0 */
    for (i = 0; i < maxFb; i++)
    {
        u8 d = (u8)((mag / div) % 10);                              /* 取当前位数字 */

        if (i < (maxFb - Point))                                    /* 小数点之前（含整数部分） */
            Disp_Char(x + PAGE(i + 1), y, (u8)('0' + d), (i + 1 == PosFb));
        else                                                        /* 小数点之后：列位置整体右移 1 列 */
            Disp_Char(x + PAGE(i + 2), y, (u8)('0' + d), (i + 1 == PosFb));

        div /= 10;                                                  /* 移到下一位（低一位） */
    }
}

/**
 * @brief      使用 32x48“大字符”显示最多 maxFb 位无符号数（支持小数位对齐，默认不画小数点）
 * @param      x        列起始坐标（像素）
 * @param      y        行起始像素坐标
 * @param      maxFb    需显示的数字位数（上限 10；不足左侧补 0）
 * @param      Da       待显示的数据（u32）
 * @param      PosFb    反显位置（0..maxFb-1），与数字位对应
 * @param      Point    小数位数（0 表示无小数；>0 时小数点位于从左数第 maxFb-Point 位之后）
 * @retval     None
 *
 * @note       本函数调用 Disp_CharB 输出 0~9；字库未定义 '.' 时，仅对位宽做右移处理，
 *             若需要显示大号小数点，可在字库 asc_B[10] 处放置点阵并改为输出索引 10。
 */
void Disp_Word_UB(u16 x, u16 y, u8 maxFb, u32 Da, u8 PosFb, u8 Point)
{
    u32 NUM = 1;                                                     /* 10 的幂：当前最高位除数 */
    u8  i;                                                           /* 位计数器 */

    if (maxFb == 0)  return;                                         /* 无需显示 */
    if (maxFb > 10)  maxFb = 10;                                     /* 安全上限：最多 10 位 */
    if (Point > maxFb) Point = maxFb;                                /* 规范化小数位数 */

    for (i = 1; i < maxFb; i++)  NUM *= 10;                          /* 初始化 NUM = 10^(maxFb-1) */

    /* 若字库支持大号小数点，可启用以下代码，在 (maxFb-Point) 与 (maxFb-Point+1) 位之间绘制 '.' */
    /* if (Point)                                                     */ /* 如果小数点位数非零显示小数点 */
    /*     Disp_CharB(x + (maxFb - Point) * 32, y, 10, false);        */ /* 假定 asc_B[10] 为 '.' 的点阵 */

    for (i = 0; i < maxFb; i++)                                      /* 高位→低位依次输出 */
    {
        u8 digit = (u8)((Da / NUM) % 10);                            /* 取当前位数字 */

        if (i < (maxFb - Point))                                     /* 小数点之前（含整数部分） */
            Disp_CharB(x + i * 32,      y, digit, (i == PosFb));
        else                                                         /* 小数点之后：整体右移 1 个字符位 */
            Disp_CharB(x + (i + 1) * 32, y, digit, (i == PosFb));

        NUM /= 10;                                                   /* 移到下一位 */
    }
}

/**
 * @brief      显示十六进制数字（高位在左），最大 8 位（FFFFFFFF）
 * @param      x        列起始坐标
 * @param      y        行起始像素坐标
 * @param      maxFb    需显示的位数（1~8），不足左侧补 0
 * @param      Da       待显示数据（u32）
 * @param      Fb       反显标志：true 反色，false 正常
 * @retval     None
 */
void Disp_Word_H(u8 x, u8 y, u8 maxFb, u32 Da, bool Fb)
{
    u8 i;                                                            /* 位计数器：从低位 nibble 开始 */
    u8 ch;                                                           /* 输出字符：'0'..'9','A'..'F' */

    if (maxFb == 0)  return;                                         /* 无需显示 */
    if (maxFb > 8)   maxFb = 8;                                      /* 上限保护：最多 8 个十六进制位 */

    for (i = 0; i < maxFb; i++)                                      /* 低位→高位取值，左侧为高位 */
    {
        ch = (u8)(Da & 0x0F);                                        /* 取最低 4 位（一个十六进制位） */
        ch = (ch > 9) ? (u8)('A' + ch - 10) : (u8)('0' + ch);        /* 转 ASCII 字符 */
        Disp_Char(x + PAGE(maxFb - i - 1), y, ch, Fb);               /* 从右往左填到固定宽度 */

        Da >>= 4;                                                    /* 右移 4 位，处理下一位 */
    }
}

/**
 * @brief      显示 16 位二进制数字（从高位到低位，'1'/'0' 以 8x16 字符显示）
 * @param      y        行起始像素坐标
 * @param      Da       16 位数据（按位输出）
 * @retval     None
 */
void Disp_Word_B(u8 y, u16 Da)
{
    u8  i, c;                                                        /* 位/字符临时变量 */
    u16 Num = 0x8000;                                                /* 掩码，从最高位开始 */

    for (i = 0; i < 16; i++)                                         /* 依次输出 16 位（高→低） */
    {
        if (Da & Num)    c = '1';
        else             c = '0';
        Num >>= 1;                                                   /* 右移到下一位 */
        Disp_Char(PAGE(i), y, c, false);                             /* 在第 i 位位置显示字符 */
    }
}


/**
 * @brief      以“mm:ss”格式显示秒计时
 * @param      x        列起始坐标
 * @param      y        行起始像素坐标
 * @retval     None
 *
 * @note       依赖 Show_Word_U(x, y, value, maxFb, posFb, point) 的实现，
 *             约定：maxFb 为位数、posFb 为反显位、point 为小数位数（本函数中为 0）。
 */
void Disp_Time_Sec(u8 x, u8 y,u16 count)
{
    Show_Word_U(x,           y, count / 60, 3, 0, false);           /* 分：3 位，00~999 */
    Disp_Char   (x + PAGE(3), y, ':',        false);                /* 分秒分隔符 ':' */
    Show_Word_U(x + PAGE(4), y, count % 60, 2, 0, false);           /* 秒：2 位，00~59 */
}

/**
 * @brief      显示 5x8 小字符（单页高度），支持 0~9 和部分符号
 * @param      x         列起始坐标
 * @param      y         行起始像素坐标
 * @param      asc_code  源字符：'0'~'9' 或 . - + : % 空格（其余字符按表索引使用）
 * @param      fb        反色标志：true 反色，false 正常
 * @retval     None
 */
void Disp_CharS(u8 x, u8 y, u8 asc_code, bool fb)
{
    u8  i = 0, mask;                                                /* 列计数/反色掩码 */
    const uc8 *pcode;                                               /* 字模指针（5 列） */

    if (x >= MAX_COL || y > MAX_ROW)    return;                     /* 越界保护 */

    if (asc_code >= '0' && asc_code <= '9')
        asc_code -= '0';                                            /* 映射到 0..9 */
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
        default: /* 其他字符：按原值索引，需确保字库覆盖 */          break;
        }
    }

    pcode = asc_S[asc_code].Msk;                                    /* 获取字模起始地址 */

    if (fb)   mask = 0xFF;                                          /* 反色：XOR 0xFF */
    else      mask = 0x00;                                          /* 正常显示 */

    Set_Addr((y >> 3), x);                                          /* 单页字体：仅写一页（8 行） */
    for (i = 0; i < 5; i++)                                         /* 5 列像素 */
        Write_Data((*pcode++) ^ mask);
}


/**
 * @brief      使用 5x8 小字符显示带符号整数（固定小数点、左侧补 0、可定位反显）
 * @param      x        列起始坐标（符号占 1 个小字符宽度）
 * @param      y        行起始像素坐标
 * @param      maxFb    显示的“数字位数”（不含符号与小数点），最大 10；不足左侧补 0
 * @param      Da       待显示数据（s32）
 * @param      PosFb    反显位置：0 表示符号位，1..maxFb 表示相应数字位
 * @param      Point    小数位数（0 表示无小数点；>0 时小数点位于从左数第 maxFb-Point 位数字之后）
 * @retval     None
 */
void Disp_Word_SS(u8 x, u8 y, u8 maxFb, s32 Da, u8 PosFb, u8 Point)
{
    u32 div = 1;                                                    /* 当前位除数：10^(maxFb-1) */
    u32 mag;                                                        /* |Da| 的无符号幅值 */
    u8  i;                                                          /* 位计数器 */

    if (x >= MAX_COL)  return;                                      /* 越界保护（X） */
    if (maxFb > 10)    maxFb = 10;                                  /* 上限保护 */
    if (Point > maxFb) Point = maxFb;                               /* 规范化小数位数 */

    /* 取幅值用于逐位取数（注意：若可能出现 INT_MIN，请在外层约束范围） */
    mag = (Da < 0) ? (u32)(-Da) : (u32)Da;                          /* 绝对值 */

    for (i = 1; i < maxFb; i++)  div *= 10;                         /* div = 10^(maxFb-1) */

    if (Point)                                                      /* 小数点：在数字区与其后之间插入 '.' */
        Disp_CharS(x + 5 * (maxFb - Point + 1), y, '.', false);

    /* 符号位：Da≠0 时显示 +/-；Da==0 显示空格保持对齐 */
    if (Da)
        Disp_CharS(x, y, (Da < 0) ? '-' : '+', (PosFb == 0));
    else
        Disp_CharS(x, y, ' ',                 (PosFb == 0));

    /* 输出数字位：从最高位到最低位，左侧不足位补 0 */
    for (i = 0; i < maxFb; i++)
    {
        u8 d = (u8)((mag / div) % 10);                              /* 当前位数字 */

        if (i < (maxFb - Point))                                    /* 小数点之前（含整数部分） */
            Disp_CharS(x + 5 * (i + 1), y, (u8)('0' + d), (i + 1 == PosFb));
        else                                                        /* 小数点之后：整体右移 1 个小字符位 */
            Disp_CharS(x + 5 * (i + 2), y, (u8)('0' + d), (i + 1 == PosFb));

        div /= 10;                                                  /* 下一位 */
    }
}

/**
 * @brief      使用 5x8 小字符显示无符号整数（固定小数点、左侧补 0、可定位反显）
 * @param      x        列起始坐标
 * @param      y        行起始像素坐标
 * @param      maxFb    显示的数字位数（最大 10）；不足左侧补 0
 * @param      Da       待显示数据（u32）
 * @param      PosFb    反显位置（0..maxFb-1），与数字位对应
 * @param      Point    小数位数（0 表示无小数点；>0 时小数点位于从左数第 maxFb-Point 位数字之后）
 * @retval     None
 */
void Disp_Word_US(u16 x, u16 y, u8 maxFb, u32 Da, u8 PosFb, u8 Point)
{
    u32 div = 1;                                                    /* 当前位除数：10^(maxFb-1) */
    u8  i;                                                          /* 位计数器 */

    if (maxFb == 0)  return;                                        /* 无需显示 */
    if (maxFb > 10)  maxFb = 10;                                    /* 上限保护 */
    if (Point > maxFb) Point = maxFb;                               /* 规范化小数位数 */

    for (i = 1; i < maxFb; i++)  div *= 10;                         /* 初始化除数 */

    if (Point)                                                      /* 小数点：位于数字区与其后之间 */
        Disp_CharS(x + 5 * (maxFb - Point), y, '.', false);

    for (i = 0; i < maxFb; i++)                                     /* 逐位输出（高→低） */
    {
        u8 d = (u8)((Da / div) % 10);                               /* 当前位数字 */

        if (i < (maxFb - Point))                                    /* 小数点之前（含整数部分） */
            Disp_CharS(x + i * 5,       y, (u8)('0' + d), (i == PosFb));
        else                                                        /* 小数点之后：整体右移 1 个小字符位 */
            Disp_CharS(x + 5 * (i + 1), y, (u8)('0' + d), (i == PosFb));

        div /= 10;                                                  /* 下一位 */
    }
}


/**
 * @brief      使用 5x8 小字符显示字符串（ASCII 部分）
 * @param      x        列起始坐标
 * @param      y        行起始像素坐标
 * @param      str      以 '\0' 结尾的字符串（仅当 *str < 0x80 时输出）
 * @param      Fb       反色标志：true 反色，false 正常
 * @retval     None
 */
void DispStringS(u8 x, u8 y, cchar* str, bool Fb)
{
    while (*str)
    {
        if (*str < 0x80)                                            /* ASCII 字符 */
            Disp_CharS(x, y, *str, Fb);

        x += 5;                                                     /* 小字符等宽 5 列 */
        str++;
    }
}

/**
 * @brief      以“mm:ss”格式显示秒计时（小字符 5x8 版本）
 * @param      x        列起始坐标
 * @param      y        行起始像素坐标
 * @retval     None
 */
void Disp_Time_SecS(u8 x, u8 y,u16 count)
{
    Disp_Word_US(x,        y, 3, count / 60, 3, 0);                 /* 分：3 位，00~999 */
    Disp_CharS  (x + 15,   y, ':', false);                          /* 3*5 列后输出 ':' */
    Disp_Word_US(x + 20,   y, 2, count % 60, 2, 0);                 /* 秒：2 位，00~59 */
}

/**
 * @brief      显示无符号整数（8x16 字符，固定小数点；左侧补 0）
 * @param      x        列起始坐标
 * @param      y        行起始像素坐标
 * @param      Da       待显示数据（u32）
 * @param      maxFb    显示的数字位数（最大 10）；不足左侧补 0
 * @param      Point    小数位数（0 表示无小数点；>0 时小数点位于从左数第 maxFb-Point 位数字之后）
 * @param      fb       反色标志：true 反色，false 正常（作用于所有数字与小数点）
 * @retval     None
 */
void Show_Word_U(u8 x, u8 y, u32 Da, u8 maxFb, u8 Point, bool fb)
{
    u32 NUM = 1;                                                    /* 10 的幂：当前最高位除数 */
    u8  i;                                                          /* 位计数器 */

    if (maxFb == 0)  return;                                        /* 无需显示 */
    if (maxFb > 10) maxFb = 10;                                     /* 上限保护 */
    if (Point > maxFb) Point = maxFb;                               /* 规范化小数位数 */

    for (i = 1; i < maxFb; i++)                                     /* NUM = 10^(maxFb-1) */
        NUM *= 10;

    if (Point)                                                       /* 若有小数位，先绘制小数点 */
        Disp_Char(x + PAGE(maxFb - Point), y, '.', fb);

    for (i = 0; i < maxFb; i++)                                      /* 高位→低位依次输出 */
    {
        u8 d = (u8)((Da / NUM) % 10);                               /* 当前位数字 */

        if (i < (maxFb - Point))                                    /* 小数点之前（含整数部分） */
            Disp_Char(x + PAGE(i),     y, (u8)('0' + d), fb);
        else                                                        /* 小数点之后：位置整体右移 1 位 */
            Disp_Char(x + PAGE(i + 1), y, (u8)('0' + d), fb);

        NUM /= 10;                                                  /* 下一位 */
    }
}

/**
 * @brief      使用 8x16 字符显示带符号整数（固定小数点、左侧补 0）
 * @param      x        列起始坐标（符号占 1 位，其后为数字位）
 * @param      y        行起始像素坐标
 * @param      Da       待显示数据（s32）
 * @param      maxFb    数字位数（不含符号与小数点），最大 10；不足左侧补 0
 * @param      Point    小数位数（0 表示无小数点；>0 时小数点位于从左数第 maxFb-Point 位数字之后）
 * @param      fb       反色标志：true 反色，false 正常
 * @retval     None
 */
void Show_Word_S(u8 x, u8 y, s32 Da, u8 maxFb, u8 Point, bool fb)
{
    u32 div = 1;                                                    /* 当前位除数：10^(maxFb-1) */
    u32 mag;                                                        /* |Da| 的无符号幅值 */
    u8  i;                                                          /* 位计数器 */

    if (x >= MAX_COL)                return;                        /* 越界保护（X） */
    if (maxFb == 0)                  return;                        /* 无需显示 */
    if (maxFb > 10)                  maxFb = 10;                    /* 上限保护 */
    if (Point > maxFb)               Point = maxFb;                 /* 规范化小数位数 */

    mag = (Da < 0) ? (u32)(-Da) : (u32)Da;                          /* 取幅值用于逐位取数 */

    for (i = 1; i < maxFb; i++)  div *= 10;                         /* div = 10^(maxFb-1) */

    if (Point)                                                      /* 若有小数位，先绘制小数点（跳过 1 个符号位） */
        Disp_Char(x + PAGE(maxFb - Point + 1), y, '.', fb);

    if (Da)  Disp_Char(x, y, (Da < 0) ? '-' : '+', fb);             /* 符号位（Da=0 时显示空格保持对齐） */
    else     Disp_Char(x, y, ' ',                     fb);

    for (i = 0; i < maxFb; i++)                                     /* 从最高位到最低位依次输出数字 */
    {
        u8 d = (u8)((mag / div) % 10);                              /* 取当前位数字 */

        if (i < (maxFb - Point))                                    /* 小数点之前（含整数部分） */
            Disp_Char(x + PAGE(i + 1), y, (u8)('0' + d), fb);
        else                                                        /* 小数点之后：整体右移 1 位跳过 '.' */
            Disp_Char(x + PAGE(i + 2), y, (u8)('0' + d), fb);

        div /= 10;                                                  /* 下一位 */
    }
}

/**
 * @brief      在列 x 处从底部向上绘制高度为 num 的实心竖条；支持全反色
 * @param      x        列坐标（0 ~ MAX_COL-1）
 * @param      num      竖条高度（像素，0 ~ MAX_ROW）
 * @param      Fb       反色标志：true 反色，false 正常
 * @retval     None
 *
 * @note       显存按页（8 行像素）组织：i 为页索引（0 顶部 → 7 底部）。
 *             先将 num 反算为自顶端的“空白高度”，随后按页写入 0x00/0xFF 及边界页掩码。
 */
void line_sign(u8 x, u8 num, bool Fb)
{
#if 0
    /* 旧实现：单页写入，适用于 64 行面板（保留以便对比） */
    num++;
    if (num >= MAX_ROW)
        num = MAX_ROW - 1;

    Set_Addr(((64 - num) >> 3), x);
    Write_Data(0xFF << ((64 - num) & 0x07));
#else
    u8  i, tMask, FbMask = 0;                                        /* 临时掩码 / 反色掩码 */
    if (Fb)  FbMask = 0xFF;                                          /* 反色：最终与写入值 XOR */

    if (num > MAX_ROW)  num = MAX_ROW;                               /* 夹紧高度到屏幕范围 */

    num = MAX_ROW - num;                                             /* 转换为自顶端的空白像素数 */

    for (i = 0; i < 8; i++)                                          /* 遍历 8 个页（8×8=64 行） */
    {
        Set_Addr(i, x);                                              /* 设置页地址与列地址 */

        if (i < (num >> 3))                                          /* 该页完全在空白区之上：全 0 */
            tMask = 0x00;
        else if (i == (num >> 3))                                    /* 边界页：上部空白、下部填充 */
            tMask = (u8)(0xFF << (num & 0x07));
        else                                                         /* 该页在填充区内：全 1 */
            tMask = 0xFF;

        tMask ^= FbMask;                                             /* 根据 Fb 取反（反色） */
        Write_Data(tMask);                                           /* 写入当前页数据 */
    }
#endif
}

/**
 * @brief      在列 x 处绘制整页（8 像素）分辨率的竖向条，从底部向上填充
 * @param      x        列坐标（0 ~ MAX_COL-1）
 * @param      num      需填充的页数（0 ~ 8），1 页 = 8 像素，自底向上累计
 * @retval     None
 */
void set_Sign(u8 x, u8 num)
{
    u8 i;                                                            /* 页索引：0 顶部 → 7 底部 */

    if (x >= MAX_COL)  return;                                       /* 越界保护（X） */
    if (num > 8)       num = 8;                                      /* 夹紧范围：最多 8 页 */

    for (i = 0; i < 8; i++)
    {
        Set_Addr(i, x);                                              /* 设置第 i 页与列 x */

        if (i > (u8)(7 - num))                                       /* 底部 num 页填充 0xFF */
            Write_Data(0xFF);
        else
            Write_Data(0x00);
    }
}

/**
 * @brief      在指定页和列写入1字节数据
 * @param[in]  page: 页地址
 * @param[in]  col:  列地址  
 * @param[in]  mask: 数据掩码（bit=1点亮对应像素）
 * @retval     None
 * @note       覆盖写模式（非叠加）
 */
static inline void write_col_byte(u8 page, u8 col, u8 mask)
{
    Set_Addr(page, col);
    Write_Data(mask);
}

/**
 * @brief      在第 y 行像素处从 x0 到 x1 画一条 1 像素厚的水平线
 * @param      x0   起始列（包含）
 * @param      x1   结束列（包含）
 * @param      y    行号（像素坐标）
 * @param      Fb   颜色/反色标志；false=正常掩码，true=反色掩码
 * @retval     无
 *
 * 说明：
 * - 显存按页（8 像素高）组织：先计算页号与页内位号，然后构造单行位掩码写入每列。
 * - 若面板位序相反，可将掩码由 (1u << bit) 改为 (0x80u >> bit)。
 * - 为避免无效访问，函数会对 x0/x1 做交换并裁剪至显示边界。
 */
void draw_hline(u8 x0, u8 x1, u8 y)
{
    if (x0 > x1) { u8 t = x0; x0 = x1; x1 = t; }                               /* 保证 x0 <= x1 */
    if (y >= MAX_ROW) return;                                                  /* 行越界直接返回 */
    if (x0 >= MAX_COL) return;                                                 /* 起点越界直接返回 */
    if (x1 >= MAX_COL) x1 = (u8)(MAX_COL - 1);                                 /* 裁剪尾端列 */

    u8 page = (u8)(y >> 3);                                                    /* 所在页 */
    u8 bit  = (u8)(y & 0x07);                                                  /* 页内第几位（0~7） */
    u8 mask = (u8)(1u << bit);                                                 /* 若位序相反改为 (0x80u >> bit) */
    
    Set_Addr(page, x0);                                                        /* 设定起始写地址 */
    for (u8 x = x0; x <= x1; x++)                                              /* 逐列写入 */
    {
        Write_Data(mask);                                                      /* 写单行位掩码 */
    }
}

/**
 * @brief      在指定列绘制竖向像素段
 * @param[in]  x:  列坐标
 * @param[in]  y0: 起始行坐标
 * @param[in]  y1: 结束行坐标
 * @retval     None
 * @note       自动处理跨页拆分，y0/y1可任意顺序
 */
void draw_vspan(u8 x, u8 y0, u8 y1)
{
    /* 确保 y0 <= y1 */                                                             /* 交换坐标确保顺序 */
    if (y0 > y1)
    {
        u8 t = y0;
        y0 = y1;
        y1 = t;
    }
    
    /* 边界检查 */                                                                  /* 超出屏幕范围则返回 */
    if (x >= MAX_COL || y0 >= MAX_ROW)
    {
        return;
    }
    
    /* 裁剪y1到屏幕范围内 */                                                         /* 防止越界 */
    if (y1 >= MAX_ROW)
    {
        y1 = MAX_ROW - 1;
    }

    /* 计算页地址和位偏移 */                                                         /* 每页8行 */
    u8 p0 = y0 >> 3;
    u8 p1 = y1 >> 3;
    u8 b0 = y0 & 7;
    u8 b1 = y1 & 7;

    if (p0 == p1)
    {
        /* 同页处理：生成位掩码 */                                                   /* 设置b0到b1之间的位 */
        u8 mask = (u8)((0xFFu << b0) & (0xFFu >> (7 - b1)));
        write_col_byte(p0, x, mask);
    }
    else
    {
        /* 首页处理：从b0到页底 */                                                   /* 设置起始位到页末 */
        u8 first_mask = (u8)(0xFFu << b0);
        write_col_byte(p0, x, first_mask);

        /* 中间整页处理：全填充 */                                                   /* 完整页全部点亮 */
        for (u8 p = p0 + 1; p < p1; ++p)
        {
            write_col_byte(p, x, 0xFF);
        }

        /* 末页处理：从页顶到b1 */                                                   /* 设置页起始到结束位 */
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

    /* Calculate bottom-right corner (inclusive) */                                 /* 计算右下角坐标（包含） */
    u8 x1 = x + w - 1;
    u8 y1 = y + h - 1;

    /* Clip to screen boundaries */                                                /* 裁剪到屏幕边界 */
    if (x >= MAX_COL || y >= MAX_ROW) return;
    if (x1 >= MAX_COL) x1 = MAX_COL - 1;
    if (y1 >= MAX_ROW) y1 = MAX_ROW - 1;

    if (fill)
    {
        /* Filled rectangle: draw vertical spans for each column */                 /* 实心矩形：每列绘制竖向跨度 */
        for (u8 cx = x; cx <= x1; ++cx)
        {
            draw_vspan(cx, y, y1);
        }
    }
    else
    {
        /* Outline rectangle: two horizontal lines + two vertical lines */          /* 空心矩形：两条水平线 + 两条垂直线 */
        draw_hline(x, x1, y);                                                      /* Top edge 顶边 */
        draw_hline(x, x1, y1);                                                     /* Bottom edge 底边 */

        if (h >= 3)                                                                /* Avoid overlapping with top/bottom */
        {
            draw_vspan(x, y + 1, y1 - 1);                                          /* Left edge 左边 */
            if (w >= 2) draw_vspan(x1, y + 1, y1 - 1);                             /* Right edge 右边 */
        }
        else if (w >= 2)
        {
            /* For small height (1 or 2 pixels), horizontal edges are sufficient */ /* 高度很小时，水平边已足够 */
            draw_vspan(x, y, y1);                                                  /* Left edge 左边 */
            draw_vspan(x1, y, y1);                                                 /* Right edge 右边 */
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

	Write_Cmd(0x81);//Set contrast Control for BANK0//对比度
	Write_Cmd(contrast);

	Write_Cmd(0x82);//Set Brightness for Area Color Banks
	Write_Cmd(Brightness);

	Write_Cmd(0x40);//Set Display Start Line from 0x40 to 0x7F
	Write_Cmd(0xa1);//Set Segment Re-map =>0xa0:column addr. 0 is mapped to SEG0//左右
	Write_Cmd(0xa4);//Entire Display On =>0xa4:resume to RAM content display
	Write_Cmd(0xa6);//Set Normal/Inverse Display =>0xa6:Normal,0xa7:inverse//正显反显

	Write_Cmd(0xa8);//Set Multiplex Ratio
	Write_Cmd(0x3f);//63

	Write_Cmd(0xad);//Master Configuration
	Write_Cmd(0x8e);

	Write_Cmd(0xc8);//Set COM Output Scan Direction//上下
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


