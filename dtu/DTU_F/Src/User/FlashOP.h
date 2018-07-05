#ifndef __FLASH_H__
#define __FLASH_H__

#define BANK1_WRITE_START_ADDR  ((uint32_t)0x08004000)
#define BANK1_WRITE_END_ADDR    ((uint32_t)0x08010000)

void Flash_EraseSector(unsigned long tLong);
void Flash_Write(unsigned long tAddr,char *tBuf,unsigned int tLen);
void Flash_Read(unsigned long tAddr,char *tBuf,unsigned int tLen);


#endif

