#ifndef __DRV_EC20_H__
#define __DRV_EC20_H__

typedef enum
{
    eIdle=0,
    eWaitRespon,
    
}eDeviceState;

typedef enum
{
    STATE_POWOFF=0,         //关闭电源
    STATE_POWSWITCH,        //通电
    STATE_POWON,            //通电
    STATE_CHECK_AT,
    STATE_CLOSE_ECHO,       //关闭发送回显
    STATE_CHECK_SIM_CARD,   //检查sim卡
    STATE_GET_QCCID,
    STATE_GET_CIMI,
    
    STATE_NET_REG,          //注册网络
    STATE_GET_CSQ,          //获取信号强度
    STATE_CHECK_NETTYPE,    //查询网络类型2g/3g/4g
    
    STATE_WAIT_CONNECT,     //等待连接网络
    STATE_CONFIG_CONTEXT,   //设置上下文
	STATE_DIACT_CONTEXT,	//关闭上下文
    STATE_ACT_CONTEXT,      //激活上下文
    STATE_GET_DNS,
    STATE_CONNECT_CONTEXT,  //连接
    STATE_WAIT_DATA,        //等待接收或发送数据
    STATE_SEND_READY,
    STATE_SENDING,
    STATE_SEND_BUSY, 
    STATE_BUSY,  
    STATE_TCPCLOSE,

    STATE_CONFIG_FTP,
    STATE_CONNECT_FTP,
    STATE_ENTER_FLODER,
    STATE_GET_FILELEN,
    STATE_GET_FILE,
    STATE_CLOSE_FTP,
}eEC20State;

void EC20PowON(void);
eEC20State EC20_SendData(unsigned char *tBuf,unsigned int tLen);

eEC20State EC20_GetState(void);

eEC20State EC20_GetFTPFile(char *tIpAddr,unsigned int tPort,char * tFolder,char *tFileName);
unsigned long GetFtpFileLen(void);
void EC20CloseTCP(void);

void Print_DTU_Version(void);

#endif
