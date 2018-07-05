#ifndef __DRV_WXCMD_H__
#define __DRV_WXCMD_H__

/* ================================================================================
����buffer����
1�����ں󸶷������ơ�һ֡�������� = 36 + 57 * ������������
2������Ԥ���ѿ�������һ֡��������	= 36 + 67 * ������������

==> Ŀǰ�����۲��������ǣ����4̨�豸�����п��ܰ���1̨Ԥ���ѿ�����(΢��IC��������)
    ���ԡ�����4̨�󸶷ѵ������ơ�1̨Ԥ���ѿ�����
		��ˡ�����buffer = 4̨�����Ƶ������� = 36 + 57 * 4 = 264 (�ֽ�)
================================================================================ */
#define WX_CMD_BUF_LEN    (256+32)

#define CMD_OPCODE_GSMMODEL         0xC352  //��������
#define CMD_OPCODE_GETFTP           0xC353  //ȡftp�ļ�
#define CMD_OPCODE_FTPTRAN          0xAAF0  //����FTP�ļ�

#define FLASH1_ERR 						0X01
#define CSQ_ERR							0X02
#define CREG_ERR						0X04
#define NET_ERR							0X08
#define POWER_ERR						0X10
#define NET_OK							0X20	
#define DTU_OK							0X40
#define CSQ_OK							0X80

#define QCCID_GOT                       0x0A
#define CIMI_GOT                        0x0C

#define CON_BIT_REP     0x10    //·��
#define CON_BIT_SEQ     0x20    //��֡
#define CON_BIT_ERR     0x40    //����
#define CON_BIT_DIR     0x80    //����
#define CON_BIT_WRITE   0x04    //д
#define CON_BIT_READ    0x01    //��

#define CON_BIT_ACK     0x0a


typedef struct
{
    unsigned char Head1_68;
    unsigned char Target[6];
    unsigned char Head2_68;
    unsigned char ConCode;
    unsigned char DataLenLo;
    unsigned char DataLenHi;
    unsigned char Time[6];
    unsigned char SetType;
    unsigned char CmdCodeLo;
    unsigned char CmdCodeHi;
    unsigned char Seq;
    unsigned char databuf[];
} stu_cmd_str;



#define CMDCODE_SETIP   0xf001  //����IP��ַ���˿ڣ�apn��
#define CMDCODE_SETFTP  0xf010  //����ftp��ַ���˿ڣ��ļ����ƣ�


void SendCmdHostMan(unsigned char *tch,unsigned int tLen);

#endif



