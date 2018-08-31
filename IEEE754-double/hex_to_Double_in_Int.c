void hex_to_double_IEEE754_in_Int(uint8_t *data, int32_t *pInt, uint32_t *pDec)
{
	uint8_t  sign;
	uint16_t e;
	uint32_t exp = 1;

	uint8_t s;
	uint8_t len = 7;
	uint8_t *spM;

	typedef float float32_t;
	float32_t fTmpDec;
	uint32_t tTmpInt = 0;
	uint32_t tTmpDec = 0;

	sign = data[0] >> 7;
	e    = (((data[0] & 0x7F) << 4) | (data[1] >> 4)) - 1023;
	while(e--)
	{
		exp *= 2;  // e如果大于31、exp就会溢出uint32_t范围
	}

	data[1] &= 0x0F;
	pM = &data[1];

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

	*pInt = tTmpInt;
	*pDec = tTmpDec;
	// 符号位
	if(sign == 1)
	{
		*pInt = -*pInt;
	}
}
