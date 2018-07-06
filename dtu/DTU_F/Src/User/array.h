/* 
 * File:   array.h
 * Author: Zzzzz
 *
 * Created on August 31, 2015, 9:02 AM
 */

#ifndef ARRAY_H
#define	ARRAY_H

#ifdef	__cplusplus
extern "C" {
#endif


typedef struct
{
    unsigned char   *Kid;
    unsigned int    pRead;
    unsigned int    pWrite;
    unsigned int    MaxKids;
    //unsigned int    KidsNums;
} T_Array, *pT_Array;

void ArrayInit(pT_Array tArray);
void ArrayFlush(pT_Array tArray);
unsigned char ArrayWrite(pT_Array tArray, unsigned char tChar);
unsigned char ArrayRead(pT_Array tArray, unsigned char *tChar);
unsigned char ArrayCheck(pT_Array tArray);

#ifdef	__cplusplus
}
#endif

#endif	/* ARRAY_H */

