#ifndef __CC_IEEE754_H__
#define __CC_IEEE754_H__

#include "r_cg_macrodriver.h"

typedef float float32_t;
#ifndef bool
    typedef uint8_t bool;
#endif
#ifndef _countof
    #define _countof(s) (sizeof(s) / sizeof(s[0]))
#endif

typedef enum 
{
	IEEE754_Std     = 0,  // �淶��ʽ��ֵ(��ͨ��ֵ)
	IEEE754_Non_Std = 1,  // �ǹ淶��ʽ��ֵ
	IEEE754_Zero    = 2,  // ����0
	IEEE754_Inf     = 3,  // ���������
	IEEE754_NaN     = 4   // �������һ����Ч��ֵ
}E_IEEE754_value_type;

typedef struct 
{
	 uint8_t  sign; // ����λ
	 int16_t  exp;  // ָ��ƫ��(ƫ��������Ϊ����)
	 int32_t  Int;  // ��������
	uint32_t  Dec;  // С������
}Stu_IEEE754, *pStu_IEEE754;

uint8_t hex_to_double_IEEE754_in_Int(uint8_t *data, int32_t *pInt, uint32_t *pDec);
void ieee754_test(void);



#endif  // #ifndef __CC_IEEE754_H__
