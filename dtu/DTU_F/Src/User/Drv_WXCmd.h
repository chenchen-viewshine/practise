#ifndef __DRV_WXCMD_H__
#define __DRV_WXCMD_H__

/* ================================================================================
多大的buffer合适
1、对于后付费流量计、一帧的数据量 = 36 + 57 * 流量嫉钠数量
2、对于预付费控制器、一帧的数据量	= 36 + 67 * 控制器的数量

==> 目前、销售部的需求是：最多4台设备，其中可能包含1台预付费控制器(微星IC卡控制器)
    所以、最多接4台后付费的流量计、1台预付费控制器
		因此、最大的buffer = 4台流量计的数据量 = 36 + 57 * 4 = 264 (字节)
================================================================================ */
#define WX_CMD_BUF_LEN    (256+32)

#define CMD_OPCODE_GSMMODEL         0xC352  //参数设置
#define CMD_OPCODE_GETFTP           0xC353  //取ftp文件
#define CMD_OPCODE_FTPTRAN          0xAAF0  //传输FTP文件

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

#define CON_BIT_REP     0x10    //路由
#define CON_BIT_SEQ     0x20    //多帧
#define CON_BIT_ERR     0x40    //错误
#define CON_BIT_DIR     0x80    //方向
#define CON_BIT_WRITE   0x04    //写
#define CON_BIT_READ    0x01    //读

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



#define CMDCODE_SETIP   0xf001  //设置IP地址，端口，apn等
#define CMDCODE_SETFTP  0xf010  //设置ftp地址，端口，文件名称，


void SendCmdHostMan(unsigned char *tch,unsigned int tLen);

#endif



