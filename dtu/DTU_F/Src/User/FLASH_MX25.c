/*
													****************************
															fm25cl64
													****************************

˵��:	����FLSAH MX25L3205D ��������
����:	����Ӳ��spi������MSB��ʽ
�汾:		v0.0.1
��Ȩ:		��Ȩ���У�(c)2010-2020���㽭΢�������Ǳ�ɷ����޹�˾
�޸ļ�¼:	--------------------------------------------------------------------
			����:			����:		����:		�汾:
			2012-12-29		����		�޸�		v0.0.1

********************************************************************************/

#include "FLASH_MX25.h"
#include "stm32f1xx_hal.h"

unsigned char FLASH_Buffer[FLASH_SPI_PAGESIZE], Fac_ID, Dev_ID;

void IO_FLASH_CS_1(void)
{
    HAL_GPIO_WritePin(IO_FLASH_CS_GPIO,IO_FLASH_CS_PIN,GPIO_PIN_SET);
}

void IO_FLASH_CS_0(void)
{
    HAL_GPIO_WritePin(IO_FLASH_CS_GPIO,IO_FLASH_CS_PIN,GPIO_PIN_RESET);
}

void FlashSelect(void)
{
    IO_FLASH_CS_0();
}
void FlashDeSelect(void)
{
    IO_FLASH_CS_1();
}
/********************************************************************************
��    ��:		XXXX		2012-12-21		//
��    ��:		XXXX		2012-12-21
˵    ��:		FLASH ����һ���ֽ�
��    ��:		��
��    ��:		��
*/
extern SPI_HandleTypeDef hspi1;
unsigned char FLASH_RecvSendByte(unsigned char byte)
{
    unsigned char re;
//    /* Loop while DR register in not emplty */
//    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
//    /* Send byte through the SPI2 peripheral */
//    SPI_I2S_SendData(SPI1, byte);
//    /* Wait to receive a byte */
//    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
//    /* Return the byte read from the SPI bus */
//    return SPI_I2S_ReceiveData(SPI1);
    HAL_SPI_TransmitReceive(&hspi1,&byte,&re,1,0xffff);
    return re;
}

void FlashDelayMs(unsigned int tMs)
{
    unsigned int k;
    while(tMs--)
    {
        k=0xffff;
        while(k--)
            ;
    }
}

/********************************************************************************
��    ��:		XXXX		2012-12-21		//
��    ��:		XXXX		2012-12-21
˵    ��:		FLASH ��һ���ֽ�
��    ��:		��
��    ��:		��
*/
unsigned char FLASH_ReadByte(void)
{
    return (FLASH_RecvSendByte(FLASH_DUMMY_BYTE));
}

/********************************************************************************
��    ��:		XXXX		2012-12-21		//
��    ��:		XXXX		2012-12-21
˵    ��:		FLASH дһ���ֽ�
��    ��:		��
��    ��:		��
*/
void FLASH_WriteByte(unsigned char ch)
{
    FLASH_RecvSendByte(ch);
}
/********************************************************************************
��    ��:		XXXX		2012-12-21
��    ��:		XXXX		2012-12-21
˵    ��:		SPIдæ��־
��    ��:		��
��    ��:		��
*/
void Write_Busy(void)
{
    unsigned int timeout=10000;
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_RDSR);
    while((FLASH_ReadByte()&0x01) && (timeout))
    {
        FlashDelayMs(10);
        timeout--;
        if(timeout==0)
            return;
    }
    FlashDeSelect();
}

/********************************************************************************
��    ��:		XXXX		2012-12-21
��    ��:		XXXX		2012-12-21
˵    ��:		SPIʹ��д
��    ��:		��
��    ��:		��
*/
void Write_Enable(void)
{
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_WREN);
    FlashDeSelect();
}

/********************************************************************************
��    ��:		XXXX		2012-12-21
��    ��:		XXXX		2012-12-21
˵    ��:		SPIʧ��д
��    ��:		��
��    ��:		��
*/
void Write_Disable(void)
{
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_WRDI);
    FlashDeSelect();
    Write_Busy();
}

/********************************************************************************
��    ��:		XXXX		2012-12-21
��    ��:		XXXX		2012-12-21
˵    ��:		SPIд״̬�Ĵ���
��    ��:		��
��    ��:		��
*/
void Write_StatusRegister(unsigned char ch)
{
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_WRSR);
    FLASH_RecvSendByte(ch);
    FlashDeSelect();
    Write_Busy();
}
/********************************************************************************
��    ��:		XXXX		2012-12-21
��    ��:		XXXX		2012-12-21
˵    ��:		SPI��״̬�Ĵ���
��    ��:		��
��    ��:		��
*/
unsigned char Read_StatusRegister(void)
{
    unsigned char re;
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_RDSR);
    re = FLASH_ReadByte();
    FlashDeSelect();
    return (re);
}

void Flash_Init(void)
{
    Write_StatusRegister(0x02);
    Write_Enable();
}

/********************************************************************************
��    ��:		XXXX		2012-12-21
��    ��:		XXXX		2012-12-21
˵    ��:		FLAS����һ������
��    ��:		SectorAddr���������׵�ַ
��    ��:		��
*/
void FLASH_EraseSector(unsigned long SectorAddr)
{
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_SE);
    FLASH_RecvSendByte((SectorAddr & 0x0ffffff) >> 16);
    FLASH_RecvSendByte((SectorAddr & 0x0ffff) >> 8);
    FLASH_RecvSendByte(SectorAddr & 0x0ff);
    FlashDeSelect();
    Write_Busy();
}

/********************************************************************************
��    ��:		XXXX		2012-12-21
��    ��:		XXXX		2012-12-21
˵    ��:		FLAS����CHIP,����Ƭ
��    ��:		��
��    ��:		��
*/
void FLASH_EraseChip(void)
{
    Flash_Init();
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_CE);
    FlashDeSelect();
    Write_Busy();
}

/********************************************************************************
��    ��:		XXXX		2012-12-21
��    ��:		XXXX		2012-12-21
˵    ��:		��ȡ����ID
��    ��:		��
��    ��:		��
*/
void FLASH_ReadID(void)
{
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_REMS);
    FLASH_RecvSendByte(0x00);
    FLASH_RecvSendByte(0x00);
    FLASH_RecvSendByte(0x00);
    Fac_ID = FLASH_ReadByte();		 //BFH: SST
    Dev_ID = FLASH_ReadByte();	     //41H:SST25VF016B
    FlashDeSelect();
}

void FLASH_EraseAddSector(void)
{
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_EN4K);
    FlashDeSelect();
    Write_Enable();
    //Flash_Init();
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_SE);
    FLASH_RecvSendByte(0x00);
    FLASH_RecvSendByte(0x00);
    FLASH_RecvSendByte(0x00);
    FlashDeSelect();
    Write_Busy();
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_EX4k);
    FlashDeSelect();
}

//д4K��
void FLASH_WriteAddSector(unsigned int tAddr,char *tBuf,unsigned int tLen)
{
    unsigned int i;
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_EN4K);
    FlashDeSelect();
    
    //Flash_Init();
    Write_Enable();
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_PP);
    FLASH_RecvSendByte(0x00);
    FLASH_RecvSendByte((tAddr & 0x0ffff) >> 8);
    FLASH_RecvSendByte(tAddr & 0x0ff);
    for(i = 0; i < tLen; i++)
    {
        FLASH_RecvSendByte(*(tBuf+i));
    }
    FlashDeSelect();
    Write_Busy();
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_EX4k);
    FlashDeSelect();
}

//��4K��
void FLASH_ReadAddSector(unsigned int tAddr,char *tBuf,unsigned int tLen)
{
    unsigned int i;
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_EN4K);
    FlashDeSelect();
    
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_READ);
    FLASH_RecvSendByte(0x00);
    FLASH_RecvSendByte((tAddr & 0x0ffff) >> 8);
    FLASH_RecvSendByte(tAddr & 0x0ff);
    for(i = 0; i < tLen; i++)
    {
        *(tBuf+i)=FLASH_RecvSendByte(FLASH_DUMMY_BYTE);
    }
    FlashDeSelect();
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_EX4k);
    FlashDeSelect();
}



void Flash_PageWrite(unsigned long tAddr,char *tBuf,unsigned int tLen)
{
    unsigned int i;
    Flash_Init();
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_PP);
    FLASH_RecvSendByte((tAddr & 0x0ffffff) >> 16);
    FLASH_RecvSendByte((tAddr & 0x0ffff) >> 8);
    FLASH_RecvSendByte(tAddr & 0x0ff);
    for(i = 0; i < tLen; i++)
    {
        FLASH_RecvSendByte(*(tBuf+i));
    }
    FlashDeSelect();
    Write_Busy();
}

/**
 *  дFlash
 *  
 */
unsigned char Flash_Write(unsigned long tAddr,char *tBuf,unsigned int tLen)
{
    unsigned int PageLen;
    PageLen=tAddr & 0x0ff;
    if(PageLen+tLen>0x100)
    {
        PageLen=0x100-PageLen;
        Flash_PageWrite(tAddr,tBuf,PageLen);
        Flash_Write(tAddr+PageLen,tBuf+PageLen,tLen-PageLen);
    }
    else
    {
        Flash_PageWrite(tAddr,tBuf,tLen);
    }
    return 1;
}

/**
 * ��Flash
 *
 */
unsigned char Flash_Read(unsigned long tAddr,char *tBuf,unsigned int tLen)
{
    unsigned int i;
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_READ);
    FLASH_RecvSendByte((tAddr & 0x0ffffff) >> 16);
    FLASH_RecvSendByte((tAddr & 0x0ffff) >> 8);
    FLASH_RecvSendByte(tAddr & 0x0ff);
    for(i = 0; i < tLen; i++)
    {
        *(tBuf+i)=FLASH_RecvSendByte(FLASH_DUMMY_BYTE);
    }
    FlashDeSelect();
    Write_Busy();    
    return 0;
}

void Flash_EraseSector(unsigned long tAddr)
{
    Flash_Init();
    FlashSelect();
    FLASH_RecvSendByte(FLASH_OPCODE_SE);
    FLASH_RecvSendByte((tAddr & 0x0ffffff) >> 16);
    FLASH_RecvSendByte((tAddr & 0x0ffff) >> 8);
    FLASH_RecvSendByte(tAddr & 0x0ff);
    FlashDeSelect();
    Write_Busy();
}

char buf[6];
void FLASH_Test(void)
{
    
    Flash_Init();
    FLASH_ReadID();
    FLASH_WriteAddSector(0,"123456",6);
    FLASH_ReadAddSector(0,buf,6);
}


