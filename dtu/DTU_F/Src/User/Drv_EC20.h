#ifndef __DRV_EC20_H__
#define __DRV_EC20_H__

typedef enum
{
    eIdle=0,
    eWaitRespon,
    
}eDeviceState;

typedef enum
{
    STATE_POWOFF=0,         //�رյ�Դ
    STATE_POWSWITCH,        //ͨ��
    STATE_POWON,            //ͨ��
    STATE_CHECK_AT,
    STATE_CLOSE_ECHO,       //�رշ��ͻ���
    STATE_CHECK_SIM_CARD,   //���sim��
    STATE_GET_QCCID,
    STATE_GET_CIMI,
    
    STATE_NET_REG,          //ע������
    STATE_GET_CSQ,          //��ȡ�ź�ǿ��
    STATE_CHECK_NETTYPE,    //��ѯ��������2g/3g/4g
    
    STATE_WAIT_CONNECT,     //�ȴ���������
    STATE_CONFIG_CONTEXT,   //����������
	STATE_DIACT_CONTEXT,	//�ر�������
    STATE_ACT_CONTEXT,      //����������
    STATE_GET_DNS,
    STATE_CONNECT_CONTEXT,  //����
    STATE_WAIT_DATA,        //�ȴ����ջ�������
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
