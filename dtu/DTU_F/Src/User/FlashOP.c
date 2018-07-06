
#include "FlashOP.h"
#include "stm32f10x.h"
#include <string.h>

#define FLASH_PAGE_SIZE    ((uint16_t)0x400)

void Flash_EraseSector(unsigned long tLong)
{
    FLASH_Unlock();

    /* Define the number of page to be erased */    /* Clear All pending flags */
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	
    
    FLASH_ErasePage(BANK1_WRITE_START_ADDR + tLong);
    
    FLASH_Lock();
}


void Flash_Write(unsigned long tAddr,char *tBuf,unsigned int tLen)
{
    uint32_t tData;
    FLASH_Unlock();

    while(tLen)
    {
        if(tLen>4)
        {
            tLen-=4;
            memcpy(&tData,tBuf,4);
        }
        else
        {
            memcpy(&tData,tBuf,tLen);
            tLen=0;
        }
        FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	
        FLASH_ProgramWord(BANK1_WRITE_START_ADDR+tAddr,tData);
        tAddr+=4;
        tBuf+=4;
    }

    FLASH_Lock();
}

void Flash_Read(unsigned long tAddr,char *tBuf,unsigned int tLen)
{
    memcpy(tBuf,(uint8_t*)(BANK1_WRITE_START_ADDR+tAddr),tLen);
}



