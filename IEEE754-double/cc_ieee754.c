#include"r_cg_userdefine.h"
#include"gprs_manage.h"
#include"cc_ieee754.h"


// 有符号数int32_t *pInt限制了整数部分超过0xFFFFFFFF/2的时候就会溢出(比如2547521564会变成：-1747445732)
uint8_t hex_to_double_IEEE754_in_Int(uint8_t *data, int32_t *pInt, uint32_t *pDec)
{
    uint8_t  offset, i;
    uint8_t  empty = 0;// 0：尾数为0；1：尾数不为0
    uint8_t  buf[7];   // 保存后面7个字节(尾数部分)
     int16_t tExp;
    uint16_t Dec12;
    Stu_IEEE754 ieee;
    ieee.Int = 0;
    ieee.Dec = 0;

    // 符号位和指数偏移
    ieee.sign  = ((data[0] & 0x80) == 0x80);
    ieee.exp   = data[0] & 0x7F;
    ieee.exp <<= 4;
    ieee.exp  |= data[1] >> 4;

    // 缓存尾数(52bit)
    for(i = 0; i < 7; i++)
    {
        buf[i] = data[1 + i];
        if(buf[i] > 0x00) { empty++; }
    }
    if(((buf[0] & 0xF0) != 0x00) && ((buf[0] & 0x0F) == 0x00)) { empty--; }
    buf[0] &= 0x0F;  // 去掉多余的指数部分

    // 特殊值处理
    switch(ieee.exp)
    {
        case 0x0000:
            if(empty == 0)  // ±0.0
            {
                *pInt = 0;
                *pDec = 0;
                return IEEE754_Zero;
            }
            else          // 非规范数值
            {
                *pInt = 0;
                *pDec = 0;
                return IEEE754_Non_Std;
            }
        case 0x07FF:
            if(empty == 0)  // ±∞
            {
                *pInt = 0;
                *pDec = 0;
                return IEEE754_Inf;
            }
            else          // NaN(结果非法：not a number)
            {
                *pInt = 0;
                *pDec = 0;
                return IEEE754_NaN;
            }
        default: break;
    }

    // 普通数值
    // 小数点偏移
    ieee.exp -= 1023;
    if(ieee.exp >= 0) // 绝对值 >= 1
    {
        // 整数部分
        ieee.Int = 0x00000001;  // 补上默认的1.0
        offset = 4;
        tExp = ieee.exp;
        while(tExp > 0)
        {
            ieee.Int <<= 1;
            if((buf[offset / 8]) & (1 << (7 - (offset % 8))))
                ieee.Int |= 0x01;
            tExp--;
            offset++;
            if(offset > 51) { break; }
        }
        // 小数部分使用前12bit计算
        Dec12 = 0;
        tExp  = 12;
        while(tExp > 0)
        {
            Dec12 <<= 1;
            if((buf[offset / 8]) & (1 << (7 - (offset % 8))))
                Dec12 |= 0x01;
            tExp--;
            offset++;
            if(offset > 51) { break; }
        }
    }
    else // 绝对值 < 1
    {
        // 整数部分
        ieee.Int = 0;
        offset = 0;
        tExp = -ieee.exp;
        // 小数部分使用前12bit计算
        Dec12   = buf[0];
        Dec12 <<= 8;
        Dec12  |= buf[1];
        Dec12  |= 1 << 12;  // 补上默认的1.0
        while(tExp > 0)
        {
            Dec12 >>= 1;
            tExp--;
            offset++;
            if(offset > 11)// 前12bit全部为0、那么就可认为小数部分为0
            {
                *pInt = 0;
                *pDec = 0;
                return IEEE754_Std;
            }
        }
    }
    // 计算小数部分
    for(i = 12; i > 0; i--)
    {
        if(Dec12 & (1 << (i - 1)))
        {
            ieee.Dec += 10000 >> (13 - i);  // 小数部分前12bit最大为4095、所以使用10000就够了
        }
    }
    ieee.Dec /= 100;  // 两位小数
    // 符号位
    if(ieee.sign == 1)
    {
        if(ieee.Int == 0) { ieee.Int = 0x80000000; }  // 具体怎么表示-0.f、根据系统要求修改
        else              { ieee.Int = -ieee.Int;  }
    }

    *pInt = ieee.Int;
    *pDec = ieee.Dec;
    return IEEE754_Std;
}

#if 1
// 测试
int32_t  value_Int = 0;
uint32_t value_Dec = 0;
typedef struct 
{
    uint8_t *name;
    uint8_t  buf[8];
}Stu_Double_test, *pStu_Double_test;

Stu_Double_test Df[] = 
{
    // 特殊值
    { "1.1125369292536007E-308 | 0x0008000000000000", { 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }, // _Non_Std
    { "-1.668805393880401E-308 | 0x800C000000000000", { 0x80, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }, // _Non_Std
    { "-0 | 0x8000000000000000",                      { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }, // -0
    { "+0 | 0x0000000000000000",                      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }, // +0
    { "+∞ | 0x7FF0000000000000",                      { 0x7F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }, // +∞
    { "-∞ | 0xFFF0000000000000",                      { 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }, // -∞
    { "+NaN | 0xFFF8000000000000",                    { 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }, // _NaN
    { "-NaN | 0xFFFC000000000000",                    { 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }, // _NaN
    // 普通数值.小数
    { "1.5 | 0x3FF8000000000000",                 { 0x3F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },  // 小数点不移动的情况
    { "0.12345678901234 | 0x3FBF9ADD3746F4C6",    { 0x3F, 0xBF, 0x9A, 0xDD, 0x37, 0x46, 0xF4, 0xC6 } },  // 14位小数
    { "0.1234567890123  | 0x3FBF9ADD3746E984",    { 0x3F, 0xBF, 0x9A, 0xDD, 0x37, 0x46, 0xE9, 0x84 } },  // 13位小数
    { "0.123456789012   | 0x3FBF9ADD37469512",    { 0x3F, 0xBF, 0x9A, 0xDD, 0x37, 0x46, 0x95, 0x12 } },  // 12位小数
    { "0.12345678901    | 0x3FBF9ADD3744621F",    { 0x3F, 0xBF, 0x9A, 0xDD, 0x37, 0x44, 0x62, 0x1F } },  // 11位小数
    // 普通数值.整数
    { "2547521564.25416 | 0x41E2FB0303882214",    { 0x41, 0xE2, 0xFB, 0x03, 0x03, 0x88, 0x22, 0x14 } },  // 10位整数
    { "547521564.1526   | 0x41C051420E138866",    { 0x41, 0xC0, 0x51, 0x42, 0x0E, 0x13, 0x88, 0x66 } },  //  9位整数
    { "14752564.56245   | 0x416C236691FF9724",    { 0x41, 0x6C, 0x23, 0x66, 0x91, 0xFF, 0x97, 0x24 } },  //  8位整数
    { "6752564.954      | 0x4159C24D3D0E5604",    { 0x41, 0x59, 0xC2, 0x4D, 0x3D, 0x0E, 0x56, 0x04 } },  //  7位整数
    { "505296.254954    | 0x411ED7410512A950",    { 0x41, 0x1E, 0xD7, 0x41, 0x05, 0x12, 0xA9, 0x50 } },  //  6位整数
    { "71289.6576278883290 | 0x40F1679A85A4D215", { 0x40, 0xF1, 0x67, 0x9A, 0x85, 0xA4, 0xD2, 0x15 } },  //  5位整数
    { "5962.02145614    | 0x40B74A057E264B99",    { 0x40, 0xB7, 0x4A, 0x05, 0x7E, 0x26, 0x4B, 0x99 } },  //  4位整数
    { "494.4145         | 0x407EE6A1CAC08312",    { 0x40, 0x7E, 0xE6, 0xA1, 0xCA, 0xC0, 0x83, 0x12 } },  //  3位整数
    { "56.2101254232    | 0x404C1AE563CE59E0",    { 0x40, 0x4C, 0x1A, 0xE5, 0x63, 0xCE, 0x59, 0xE0 } },  //  2位整数
    { "5.0              | 0x4014000000000000",    { 0x40, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },  //  1位整数
    { "0.56214          | 0x3FE1FD0D0678C005",    { 0x3F, 0xE1, 0xFD, 0x0D, 0x06, 0x78, 0xC0, 0x05 } },  //  +0.f
    { "-0.46214           | 0xBFDD93B3A68B19A4",  { 0xBF, 0xDD, 0x93, 0xB3, 0xA6, 0x8B, 0x19, 0xA4 } },  //  -0.f
    { "-9.2152015         | 0xC0226E2EE41919AC",  { 0xC0, 0x22, 0x6E, 0x2E, 0xE4, 0x19, 0x19, 0xAC } },  //  1位整数
    { "-95.026542152015   | 0xC057C1B2DDDAB7AC",  { 0xC0, 0x57, 0xC1, 0xB2, 0xDD, 0xDA, 0xB7, 0xAC } },  //  2位整数
    { "-156.456           | 0xC0638E978D4FDF3B",  { 0xC0, 0x63, 0x8E, 0x97, 0x8D, 0x4F, 0xDF, 0x3B } },  //  3位整数
    { "-2564.2645         | 0xC0A408876C8B4396",  { 0xC0, 0xA4, 0x08, 0x87, 0x6C, 0x8B, 0x43, 0x96 } },  //  4位整数
    { "-95216.2456221     | 0xC0F73F03EE11706B",  { 0xC0, 0xF7, 0x3F, 0x03, 0xEE, 0x11, 0x70, 0x6B } },  //  5位整数
    { "-657425.59015024526 | 0xC12410232E282C46", { 0xC1, 0x24, 0x10, 0x23, 0x2E, 0x28, 0x2C, 0x46 } },  //  6位整数
    { "-4647526.4245      | 0xC151BA999B2B020C",  { 0xC1, 0x51, 0xBA, 0x99, 0x9B, 0x2B, 0x02, 0x0C } },  //  7位整数
    { "-56475215.2654     | 0xC18AEDF27A1F8A09",  { 0xC1, 0x8A, 0xED, 0xF2, 0x7A, 0x1F, 0x8A, 0x09 } },  //  8位整数
    { "-256475215.2654    | 0xC1AE93009E87E282",  { 0xC1, 0xAE, 0x93, 0x00, 0x9E, 0x87, 0xE2, 0x82 } },  //  9位整数
    { "-1569654456.45621  | 0xC1D763C1AE1D328B",  { 0xC1, 0xD7, 0x63, 0xC1, 0xAE, 0x1D, 0x32, 0x8B } },  // 10位整数
    { "-29469659451.12547 | 0xC21B721D44EC807B",  { 0xC2, 0x1B, 0x72, 0x1D, 0x44, 0xEC, 0x80, 0x7B } },  // 11位整数
};

void ieee754_test(void)
{
    uint8_t i;
    uint8_t type;

    // 列举测试
    for(i = 0; i < _countof(Df); i++)
    {
        print("\r\n[%d] %s:\r\n", i, Df[i].name);
        if((i == 19) || (i == 26))
        {
            value_Int++;
        }
        type = hex_to_double_IEEE754_in_Int(Df[i].buf, &value_Int, &value_Dec);
        print(" v = %ld.%02d", value_Int, value_Dec);
        switch(type)
        {
            case IEEE754_Std:
                print("\r\n");
                break;
            case IEEE754_Non_Std:
                print(" -->|_Non_Std\r\n");
                break;
            case IEEE754_Zero:
                print(" -->| 0\r\n");
                break;
            case IEEE754_Inf:
                print(" -->| ∞\r\n");
                break;
            case IEEE754_NaN:
                print(" -->|_NaN\r\n");
                break;
        }
    }
    // 编写测试代码、覆盖double的11位正整数到11位负整数(步进=0.01)
}
#endif
