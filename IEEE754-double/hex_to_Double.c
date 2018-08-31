带时间：
17 03 2A 14 07 03 09 44 59 40 7E E6 A1 E0 B2 30 15 40 7E E6 A1 57 6C 00 00 43 80 06 DB 43 80 06 DC 41 A0 00 00 42 CA 99 99 00 01 00 00 8F E7 

不带时间：
17 03 2D 40 7E E6 A1 E0 B2 30 15 40 7E E6 A1 57 6C 00 00 43 80 06 DB 43 80 06 DC 41 A0 00 00 42 CA 99 99 00 01 00 00 

17 03 2D 40 7E E6 A1 E0 B2 30 15 40 7E E6 A1 57 6C 00 00 43 80 06 DB 43 80 06 DC 41 A0 00 00 42 CA 99 99 00 01 00 00 60 08 


 * Note         : IEEE754浮点数存储格式为：SEEEEEEE EMMMMMMM MMMMMMMM MMMMMMMM = (-1)^S(整数) * 2^E(整数) * M(浮点数)











 SEEEEEEE EEEEMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMMSEEEEEEE EEEEMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM 
 *                                              = (-1)^S(1bit) * 2^E(11bit) * M(52bit)
40 F1 67 9A 85 A4 D2 15

40 	      F1	    67	      9A	    85	      A4	    D2	      15
0100 0000 1111 0001 0110 0111 1001 1010 1000 0101 1010 0100 1101 0010 0001 0101

S EEE EEEE EEEE MMMM MMMM MMMM MMMM MMMM MMMM MMMM MMMM MMMM MMMM MMMM MMMM MMMM
0 100 0000 1111 0001 0110 0111 1001 1010 1000 0101 1010 0100 1101 0010 0001 0101
S = 0
E = 0x040F
M = 0x01679A85A4D215
f = (-1)^S * 2^(E - 1023) * (1 + 0.M)


uint8_t  sign;
uint16_t e;
uint32_t exp = 1;
sign = data[0] >> 7;
e    = (((data[0] & 0x7F) << 4) | (data[1] >> 4)) - 1023;
while(e--)
{
	exp *= 2;  // e如果大于31、exp就会溢出uint32_t范围
}

uint8_t s;
uint8_t len = 7;
uint8_t *spM;
data[1] &= 0x0F;
pM = &data[1];

typedef float float32_t;
float32_t fTmpDec;
uint32_t tTmpInt = 0;
uint32_t tTmpDec = ; = 0;

// 整数部分
data[1] |= 0x40;  // 补上1.0
exp  += 4;        // 1.0所在的半字节
index = 0;
while(exp > 8)
{
	tTmpInt <<= 8;
	tTmpInt  += data[1 + index];
	exp -= 8;
	index++;
}
if(exp > 0)
{
	tTmpInt <<= 8;
	tTmpInt  += data[1 + index] >> (8 - exp);	
}

// 小数部分
data[1 + index] <<= exp;  // 去掉属于整数部分的bits
data[1 + index]  |= data[1 + index + 1] >> (8 - exp)；  // 凑足8bits
for(i = 8; i > 0; i--)
{
	if(data[1 + index] & (1 << (i - 1)))
	{
		fTmpDec += 1 / pow(2, 8 - i);
	}
}
tTmpDec = (uint32_t)fTmpDec;