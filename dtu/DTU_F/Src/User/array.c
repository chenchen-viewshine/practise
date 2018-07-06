#include "array.h"

unsigned char ArrayCheck(pT_Array tArray)
{
    if(tArray->pRead == tArray->pWrite)
        return 0;
    else
        return 1;
}

unsigned char ArrayRead(pT_Array tArray, unsigned char *tChar)
{
    if(tArray->pRead == tArray->pWrite)
    {
        return 0;
    }
    *tChar = tArray->Kid[tArray->pRead];
    tArray->pRead += 1;
    if(tArray->pRead > tArray->MaxKids)
        tArray->pRead = 0;
    return 1;
}

unsigned char ArrayWrite(pT_Array tArray, unsigned char tChar)
{
    if(((tArray->pWrite + 1) % tArray->MaxKids) ==
            (tArray->pRead))
        return 0;
    tArray->Kid[tArray->pWrite] = tChar;
    tArray->pWrite += 1;
    if(tArray->pWrite > tArray->MaxKids)
        tArray->pWrite = 0;
    return 1;
}

void ArrayInit(pT_Array tArray)
{
    tArray->pRead = 0;
    tArray->pWrite = 0;
    tArray->MaxKids = 0;
    tArray->Kid = 0;
}

void ArrayFlush(pT_Array tArray)
{
    tArray->pRead = 0;
    tArray->pWrite = 0;
}

