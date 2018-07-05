#ifndef __FLASH_MX25__
#define __FLASH_MX25__

#define FLASH_DUMMY_BYTE 0xff
#define FLASH_SPI_PAGESIZE  256

#define FLASH_OPCODE_WREN       0x06
#define FLASH_OPCODE_WRDI       0x04
#define FLASH_OPCODE_RDID       0x9F
#define FLASH_OPCODE_RDSR       0x05
#define FLASH_OPCODE_WRSR       0x01
#define FLASH_OPCODE_READ       0x03
#define FLASH_OPCODE_FREAD      0x0B
#define FLASH_OPCODE_PARALLER   0x55

#define FLASH_OPCODE_SE         0x20//0xD8
#define FLASH_OPCODE_CE         0x60//0XC7
#define FLASH_OPCODE_PP         0x02
#define FLASH_OPCODE_DP         0xB9
#define FLASH_OPCODE_EN4K       0xA5
#define FLASH_OPCODE_EX4k       0xB5
#define FLASH_OPCODE_RDP        0xAB
#define FLASH_OPCODE_RES        0xAB//??手册上这两个指令相同，未验证
#define FLASH_OPCODE_REMS       0x90


void FLASH_EraseChip(void);
void Flash_Init(void);
void FLASH_EraseAddSector(void);
void FLASH_WriteAddSector(unsigned int tAddr,char *tBuf,unsigned int tLen);
void FLASH_ReadAddSector(unsigned int tAddr,char *tBuf,unsigned int tLen);
unsigned char Flash_Write(unsigned long tAddr,char *tBuf,unsigned int tLen);
unsigned char Flash_Read(unsigned long tAddr,char *tBuf,unsigned int tLen);
void Flash_EraseSector(unsigned long tAddr);
void FLASH_Test(void);

void FlashDelayMs(unsigned int tMs);


//unsigned short p_EEPROM_Add[10];//EEPROM存数数据地址的指针


#endif


