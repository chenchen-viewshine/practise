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
	IEEE754_Std     = 0,  // 规范格式数值(普通数值)
	IEEE754_Non_Std = 1,  // 非规范格式数值
	IEEE754_Zero    = 2,  // 正负0
	IEEE754_Inf     = 3,  // 正负无穷大
	IEEE754_NaN     = 4   // 结果不是一个有效数值
}E_IEEE754_value_type;

typedef struct 
{
	 uint8_t  sign; // 符号位
	 int16_t  exp;  // 指数偏移(偏移量可能为负数)
	 int32_t  Int;  // 整数部分
	uint32_t  Dec;  // 小数部分
}Stu_IEEE754, *pStu_IEEE754;

uint8_t hex_to_double_IEEE754_in_Int(uint8_t *data, int32_t *pInt, uint32_t *pDec);
void ieee754_test(void);



#endif  // #ifndef __CC_IEEE754_H__
