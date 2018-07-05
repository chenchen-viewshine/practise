#include "Drv_EC20.h"
#include "UserMain.h"
#include "Drv_WXCmd.h"
#include <stdio.h>
#include <string.h>
#include "FLASH_MX25.h"


const unsigned char DTU_Version[3] = {0x01, 0x00, 0x05};  // DTU����汾 = V01.00.05

extern unsigned char APNStr[30],IPStr[6];
extern unsigned char UserName[30];
extern unsigned char PassWord[30];

unsigned char EC20NetWorkType = 0;//0:�ƶ���1�����ţ�2����ͨ

static char EC20CmdSendBuf[128];//,EC20CmdWaitRespond[128],EC20CmdWaitRespondLen;
static unsigned char EC20CmdRespondOK,CheckRespondStep;
// static eDeviceState DeviceState;
static struct 
{
    unsigned char Buf[256];
    unsigned int Len;
    unsigned int Timeout;
    unsigned char State;
}EC20Cmd;

unsigned int EC20Rssi;
//ftp��ز���
char FTP_IpAddr[50],FTP_FileName[50],FTP_FileFloder[50],FTP_Buf[1024];
unsigned int FTP_Port,FTP_BufLen;
unsigned long FTPFileLen,FTPFileNeedGetLen,FtpFileGetLen,FtpFileGetStart;

unsigned int EC20RecvLen,EC20RecvedDataLen;
unsigned char EC20RecvBuf[1500];

unsigned int EC20SendLen;
unsigned char *EC20SendBuf;

eEC20State EC20State,tEC20State,PreFtpState;
static unsigned int CmdWaitTime=0,CmdRtyTimes=0;
eEC20State EC20_GetState(void)
{
    return EC20State;
}

void Print_DTU_Version(void)
{
    uint8_t tDebugStr[25];
    uint8_t len = sizeof(tDebugStr);
    sprintf((char *)tDebugStr,"DTU = V%d.%d.%d\r\n",DTU_Version[0],DTU_Version[1],DTU_Version[2]);
    if(len > strlen((const char *)tDebugStr))
        len = strlen((const char *)tDebugStr);
    DebugOutStr(tDebugStr, len);
}

void EC20SendAT(void)
{
    sprintf(EC20CmdSendBuf,"AT\r\n");
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATE0(void)
{
    sprintf(EC20CmdSendBuf,"ATE0\r\n");
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATCPIN(void)
{
    sprintf(EC20CmdSendBuf,"AT+CPIN?\r\n");
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATQCCID(void)
{
    sprintf(EC20CmdSendBuf,"AT+QCCID\r\n");
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATCIMI(void)
{
    sprintf(EC20CmdSendBuf,"AT+CIMI\r\n");
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATCREG(void)
{
    sprintf(EC20CmdSendBuf,"AT+CREG?\r\n");
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATCGREG(void)
{
    sprintf(EC20CmdSendBuf,"AT+CGREG?\r\n");
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATCEREG(void)
{
    sprintf(EC20CmdSendBuf,"AT+CEREG?\r\n");
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATCSQ(void)
{
    sprintf(EC20CmdSendBuf,"AT+CSQ\r\n");
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}

void EC20SendATQCFGCDMA(void)
{
		sprintf((char*)EC20CmdSendBuf,"AT+QCFG=\"CDMARUIM\",1%c",0x0D);//���ø������ר�������ã�����CDMA��NVȡ�������룩
		UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}

void EC20SendATQCTPWDCFG(void)
{
		sprintf((char*)EC20CmdSendBuf,"AT+QCTPWDCFG=\"%s\",\"%s\"%c",(char*)UserName,(char*)PassWord,0x0D);//CDMA������ҵ���û���������
		UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATQICSGP(void)
{
    //sprintf(EC20CmdSendBuf,"AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n","CMNET");
    sprintf(EC20CmdSendBuf,"AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\",1\r\n",(char*)APNStr, (char*)UserName, (char*)PassWord);
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATQIACT(void)
{
    sprintf(EC20CmdSendBuf,"AT+QIACT=1\r\n");
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATQIDNSCFG(void)
{
    sprintf(EC20CmdSendBuf,"AT+QIDNSCFG=1\r\n");
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATQIDEACT(void)
{
    sprintf(EC20CmdSendBuf,"AT+QIDEACT=1\r\n");
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATQIOPEN(unsigned char *tStr,unsigned int tPort)
{    
    //sprintf(EC20CmdSendBuf,"AT+QIOPEN=1,0,\"TCP\",\"%d.%d.%d.%d\",%d,0,0\r\n",115,236,33,164,21);
    sprintf(EC20CmdSendBuf,"AT+QIOPEN=1,0,\"TCP\",\"%d.%d.%d.%d\",%d,0,0\r\n",IPStr[0],IPStr[1],IPStr[2],IPStr[3],0x100*IPStr[4]+IPStr[5]);
    //sprintf(EC20CmdSendBuf,"AT+QIOPEN=1,0,\"TCP\",\"%s\",%d,0,0\r\n",tStr,tPort);
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATQICLOSE(void)
{
    sprintf(EC20CmdSendBuf,"AT+QILCOSE=1\r\n");
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}

void EC20SendATQIREAD(void)
{
    sprintf(EC20CmdSendBuf,"AT+QIRD=0\r\n");
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}

void EC20SendATQISEND(unsigned int tLen)
{
    sprintf(EC20CmdSendBuf,"AT+QISEND=0,%d\r\n",tLen);
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}

void EC20SendATQFTPCFG(unsigned char tNum)
{
    switch(tNum)
    {
        case 0:
            sprintf(EC20CmdSendBuf,"AT+QFTPCFG=\"filetype\",1\r\n");
            break;
        case 1:
            sprintf(EC20CmdSendBuf,"AT+QFTPCFG=\"transmode\",0\r\n");
            break;
    }
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATQFTPOPEN(char *tStr,unsigned int tPort)
{
    sprintf(EC20CmdSendBuf,"AT+QFTPOPEN=\"%s\",%d\r\n",tStr,tPort);
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATQFTPCLOSE(void)
{
    sprintf(EC20CmdSendBuf,"AT+QFTPCLOSE\r\n");
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATQFTPCWD(char *tPath)
{
    sprintf(EC20CmdSendBuf,"AT+QFTPCWD=\"%s\"\r\n",tPath);
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATQFTPGET(char *tFile,unsigned long tStart,unsigned long tLen)
{
    sprintf(EC20CmdSendBuf,"AT+QFTPGET=\"%s\",\"COM:\",%ld,%ld\r\n",tFile,tStart ,tLen );
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}
void EC20SendATQFTPLEN(char *tFile)
{
    sprintf(EC20CmdSendBuf,"AT+QFTPSIZE=\"%s\"\r\n",tFile);
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}


void EC20SendATCTZR(void)
{
    sprintf(EC20CmdSendBuf,"AT+CTZR=2\r\n");
    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
}

unsigned int FindCharInStr(unsigned char *tBuf,unsigned int tMaxLen,unsigned char tMatch)
{
    unsigned int i;
    for(i=0;i<tMaxLen;i++)
    {
        if(*(tBuf+i)==tMatch)
        {
            return i;
        }
    }
    return i;
}

unsigned int GetStrPara(unsigned char *tBuf,unsigned int tMaxLen,unsigned char tchnum)
{
    unsigned int i,re=0;
    unsigned char k=0,fc=0;
    for(i=0;i<tMaxLen;i++)
    {
        if(*tBuf>=0x30 && *tBuf<=0x39)
            fc=1;
        
        if(fc!=0)
        {
            if(*tBuf>=0x30 && *tBuf<=0x39)
            {
                re*=10;
                re+=(*tBuf & 0x0f);
            }
            else
            {
                k++;
                if(k==tchnum)
                {
                    return re;
                }
                else
                {
                    re=0;
                }
            }
        }
        tBuf++;
    }
    return re;
}

void CheckOtherRespond(void)
{
    if(memcmp(EC20CmdSendBuf,"AT+CSQ",6)==0)
    {
        if(memcmp(EC20Cmd.Buf,"+CSQ:",5)==0)
        {
            EC20Rssi=GetStrPara(EC20Cmd.Buf+5,EC20Cmd.Len,1);
        }                    
    }
}

static unsigned char CmdWaitOK;
extern unsigned char FileSendMark;
// ���GPRSģ���ATָ��ķ���
void CheckRespond(void)
{
    if(EC20Cmd.State==9)
    {           
        if(CmdWaitOK)
        {
            if(memcmp(EC20Cmd.Buf,"OK",2)==0)
            {
                CmdWaitOK=0;
                return;
            }
        }
        // GPRSģ�鷴��˵Ready�ˡ���ֱ�ӿ�ʼATָ���
        // ��Ҫ�����ϵ�ʱ����Ҫ�ȴ���10�롢GPRS�Ϳ���Ready��
        if(memcmp(EC20Cmd.Buf,"RDY",2)==0)  // GPRS����'RDY'(ֻ�Ƚ�2���ַ���)
        {
            EC20State=STATE_CHECK_AT;       // GPRSģ��Ready֮�󡢾Ϳ��Կ�ʼATָ�����
            EC20CmdRespondOK=1;             // GPRS������
            CmdWaitTime=0;                  // ����Ҫ�ٵȴ��ˡ�ֱ�ӿ�ʼ����ATָ��
        }
        // ���̿��ƣ�����GPRS��ATָ��ķ�����Ϣ
        switch(EC20State)
        {
            case STATE_POWON:
            case STATE_CHECK_AT: 
                if(memcmp(EC20Cmd.Buf,"OK",2)==0)   // 'AT'�ķ���='OK'
                {
                    EC20State=STATE_CLOSE_ECHO;     // ������һ��'ATE0'����
                    EC20CmdRespondOK=1;
                    CmdWaitTime=0;
                }
                break;
            case STATE_CLOSE_ECHO:
                if(memcmp(EC20Cmd.Buf,"OK",2)==0)
                {
                    EC20State=STATE_CHECK_SIM_CARD;
                    EC20CmdRespondOK=1;
                    CmdWaitTime=0;
                }
                break;
            case STATE_CHECK_SIM_CARD:   // ���sim��
                if(CheckRespondStep==0)  // SIM������Ҫ2��
                {
                    if(memcmp(EC20Cmd.Buf,"+CPIN: READY",12)==0) // ���SIM������ҪPIN��
                        CheckRespondStep=1;
                    if(memcmp(EC20Cmd.Buf,"+CME ERROR: 10",14)==0)
                    {
                        EC20CmdRespondOK=1;
                        CmdWaitTime=2000; // 2s���ط�ָ��
                    }
                }
                else if(CheckRespondStep==1)
                {
                    if(memcmp(EC20Cmd.Buf,"OK",2)==0)
                    {
                        EC20State=STATE_GET_QCCID;
                        EC20CmdRespondOK=1;
                        CmdWaitTime=0;
                    }
                }
                break;
            case STATE_GET_QCCID:
                if(memcmp(EC20Cmd.Buf,"+QCCID:",7)==0)
                {
                    unsigned char tSendData[21];
                    EC20State=STATE_GET_CIMI;
                    EC20CmdRespondOK=1;
                    CmdWaitTime=0;
                    tSendData[0]=QCCID_GOT;
                    memcpy(tSendData+1,EC20Cmd.Buf+8,20);
                    SendCmdHostMan(tSendData,21);
                }
                break;
            case STATE_GET_CIMI:
                if(memcmp(EC20Cmd.Buf,"46",2)==0)//
                {
                    unsigned char tSendData[16];
					// �й��ƶ�
                    if(memcmp(EC20Cmd.Buf,"46000", 5)==0) EC20NetWorkType = 0;	else
                    if(memcmp(EC20Cmd.Buf,"46002", 5)==0) EC20NetWorkType = 0;	else
                    if(memcmp(EC20Cmd.Buf,"46004", 5)==0) EC20NetWorkType = 0;	else
                    if(memcmp(EC20Cmd.Buf,"46007", 5)==0) EC20NetWorkType = 0;	else
                    // �й���ͨ
                    if(memcmp(EC20Cmd.Buf,"46001", 5)==0) EC20NetWorkType = 1;	else
                    if(memcmp(EC20Cmd.Buf,"46006", 5)==0) EC20NetWorkType = 1;	else
                    if(memcmp(EC20Cmd.Buf,"46009", 5)==0) EC20NetWorkType = 1;	else
                    // �й�����
                    if(memcmp(EC20Cmd.Buf,"46003", 5)==0) EC20NetWorkType = 2;	else
                    if(memcmp(EC20Cmd.Buf,"46005", 5)==0) EC20NetWorkType = 2;	else
                    if(memcmp(EC20Cmd.Buf,"46011", 5)==0) EC20NetWorkType = 2;
									
//					EC20State=STATE_NET_REG;
                    EC20State=STATE_CHECK_NETTYPE;
                    EC20CmdRespondOK=1;
                    CmdWaitTime=1000;
                    tSendData[0]=CIMI_GOT;
                    memcpy(tSendData+1,EC20Cmd.Buf,15);
                    SendCmdHostMan(tSendData,16);
                }
                break;
            case STATE_NET_REG:          //ע������
                if(CheckRespondStep==0)
                {
                    if(memcmp(EC20Cmd.Buf,"+CREG:",6)==0)
                    {
                        if(GetStrPara(EC20Cmd.Buf+7,EC20Cmd.Len,2)==1 ||
                           GetStrPara(EC20Cmd.Buf+7,EC20Cmd.Len,2)==5 )
                        {
                            CheckRespondStep=1;
                        }
                        else
                        {
                            EC20CmdRespondOK=1;
                            CmdWaitTime=5000;                                
                        }
                        memset(EC20Cmd.Buf,0xff,10);
                    }
                }
                else if(CheckRespondStep==1)
                {
                    if(memcmp(EC20Cmd.Buf,"OK",2)==0)
                    {
                        EC20State=STATE_GET_CSQ;
                        EC20CmdRespondOK=1;
                        CmdWaitTime=0;
                    }
                }
                break;
            case STATE_CHECK_NETTYPE:    //��ѯ��������2g/3g/4g
                if(CheckRespondStep==0)
                {
                    if(memcmp(EC20Cmd.Buf,"+CEREG:",7)==0)
                    {
                        if(GetStrPara(EC20Cmd.Buf+7,EC20Cmd.Len,2)==1)  // 1��������ע��Ϊ�������硢5��������ע��Ϊ�������硢2��δע�ᡢ3��ע�ᱻ�ܾ���4��δ֪����
                        {
                            CheckRespondStep=1;
                        }
                        else
                        {
                            EC20CmdRespondOK=1;
                            CmdWaitTime=2000;                                
                        }
                    }
                }
                else if(CheckRespondStep==1)
                {
                    if(memcmp(EC20Cmd.Buf,"OK",2)==0)
                    {
                        EC20State=STATE_GET_CSQ;//STATE_WAIT_CONNECT;
                        EC20CmdRespondOK=1;
                        CmdWaitTime=0;
                    }
                }
                break;
            case STATE_GET_CSQ:
                if(CheckRespondStep==0)
                {
                    if(memcmp(EC20Cmd.Buf,"+CSQ:",5)==0)
                    {
                        EC20Rssi=GetStrPara(EC20Cmd.Buf+6,EC20Cmd.Len,1);
                        if(EC20Rssi<99 && EC20Rssi>9)
                        {
                            unsigned char tSendData[2];
                            EC20CmdRespondOK=1;
                            tSendData[0]=CSQ_OK;
                            tSendData[1]=EC20Rssi;                            
                            SendCmdHostMan(tSendData,2);
//                            sprintf(EC20CmdSendBuf,"--׼����ɣ��ȴ�����---\r\n");
//                            UartSendStr(UART_CHAN_DEBUG,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));                            
                        }
                        CheckRespondStep=1;
                    }                    
                }
                else if(CheckRespondStep==1)
                {
                    if(memcmp(EC20Cmd.Buf,"OK",2)==0)
                    {
                        if(EC20CmdRespondOK==1)
                        {
                            EC20State=STATE_CONFIG_CONTEXT;
                            //EC20SendATCTZR();
                            CmdWaitTime=1000;
                            //CmdWaitOK =1;
                        }
                        EC20CmdRespondOK=1;
                    }
                }
                break;
            case STATE_CONFIG_CONTEXT:   //����������
                if(memcmp(EC20Cmd.Buf,"OK",2)==0)
                {
                    EC20State=STATE_DIACT_CONTEXT;
                    EC20CmdRespondOK=1;
                    CmdWaitTime=100;
                    
//                    sprintf(EC20CmdSendBuf,"AT+CTZR?\r\n");
//                    UartSendStr(UART_CHAN_EC20,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
//                    CmdWaitTime=2000;
//                    CmdWaitOK =1;
                    
                }
                break;
			case STATE_DIACT_CONTEXT:
                if(memcmp(EC20Cmd.Buf,"OK",2)==0)
                {
                    EC20State=STATE_ACT_CONTEXT;
                    EC20CmdRespondOK=1;
                    CmdWaitTime=100;
                }
				break;
            case STATE_ACT_CONTEXT:      //����������
                if(memcmp(EC20Cmd.Buf,"OK",2)==0)
                {
                    EC20State=STATE_GET_DNS;//STATE_WAIT_CONNECT;
                    EC20CmdRespondOK=1;
                    CmdWaitTime=1000;
                }
                break;
            case STATE_GET_DNS:
                if(memcmp(EC20Cmd.Buf,"+QIDNSCFG:",10)==0)
                {     
                    unsigned char k;
                    unsigned char tSendData[16];
                    EC20State=STATE_CONNECT_CONTEXT;
                    EC20CmdRespondOK=1;
                    CmdWaitTime=0;
                    tSendData[0]=0x0d;
                    tSendData[1]=GetStrPara(EC20Cmd.Buf+14,16,1);
                    tSendData[2]=GetStrPara(EC20Cmd.Buf+14,16,2);
                    tSendData[3]=GetStrPara(EC20Cmd.Buf+14,16,3);
                    tSendData[4]=GetStrPara(EC20Cmd.Buf+14,16,4);
                    k=FindCharInStr(EC20Cmd.Buf+14,20,',')+1;
                    tSendData[5]=GetStrPara(EC20Cmd.Buf+14+k,16,1);
                    tSendData[6]=GetStrPara(EC20Cmd.Buf+14+k,16,2);
                    tSendData[7]=GetStrPara(EC20Cmd.Buf+14+k,16,3);
                    tSendData[8]=GetStrPara(EC20Cmd.Buf+14+k,16,4);                    
                    SendCmdHostMan(tSendData,9);
                }
                break;
            case STATE_CONNECT_CONTEXT:  //����
                if(memcmp(EC20Cmd.Buf,"+QIOPEN:",8)==0)
                {
                    if(GetStrPara(EC20Cmd.Buf+8,EC20Cmd.Len,2)==0)
                    {
                        EC20State=STATE_WAIT_DATA;
                        EC20CmdRespondOK=1;
                        CmdWaitTime=1000;

                        unsigned char tSendData[1];
                        EC20CmdRespondOK=1;
                        tSendData[0]=NET_OK;
                        SendCmdHostMan(tSendData,1);
                    }
                    else
                    {
                        // ����ʧ�ܣ�����GPRSģ��
                        EC20State=STATE_POWOFF;
                        EC20CmdRespondOK=1;
                        //EC20SendATQIDEACT();
                        CmdWaitTime=3000;
                        CmdWaitOK=1;
                    }
                }
                else if(memcmp(EC20Cmd.Buf,"ERROR",5)==0)
                {
                    CmdWaitTime=100;
                }                    
                break;
            case  STATE_SEND_READY:
                break;
            case  STATE_SENDING:
                break;
            case  STATE_SEND_BUSY:
                if(memcmp(EC20Cmd.Buf,"SEND OK",7)==0)
                {
                    EC20State = STATE_WAIT_DATA;
                }
                break;
            case STATE_WAIT_DATA:
                if(memcmp(EC20Cmd.Buf,"+QIURC:",7)==0)
                {
                    EC20SendATQIREAD();
                    CmdWaitTime=1000;
                }  
                else if(memcmp(EC20Cmd.Buf,"+QIRD:",6)==0)
                {
                    EC20RecvedDataLen=GetStrPara (EC20Cmd.Buf+6,EC20Cmd.Len-6,1);
                    EC20RecvLen=0;
                }
                break;
            case STATE_CONFIG_FTP:
                if(memcmp(EC20Cmd.Buf,"OK",2)==0)
                {
                    if(CheckRespondStep==0)
                    {
                        EC20SendATQFTPCFG(1);
                        CheckRespondStep=1;
                    }
                    else
                    {
                        EC20State = STATE_CONNECT_FTP;
                    }
                    EC20CmdRespondOK=1;
                    CmdWaitTime=100;
                }
                break;
            case STATE_CONNECT_FTP:
                if(memcmp(EC20Cmd.Buf,"+QFTPOPEN:",10)==0)
                {
                    EC20CmdRespondOK=1;
                   if(GetStrPara(EC20Cmd.Buf+10,EC20Cmd.Len,1)==0)
                    {
                        EC20State=STATE_ENTER_FLODER;
                        CmdWaitTime=1000;
                    }
                }
                break;
            case STATE_ENTER_FLODER:
                if(memcmp(EC20Cmd.Buf,"+QFTPCWD:",9)==0)
                {
                    EC20CmdRespondOK=1;
                    if(GetStrPara(EC20Cmd.Buf+9,EC20Cmd.Len,2)==0)
                    {
                        EC20State=STATE_GET_FILELEN;
                        CmdWaitTime=1000;
                    }
                }
                break;
            case STATE_GET_FILELEN:
                if(memcmp(EC20Cmd.Buf,"+QFTPSIZE:",10)==0)
                {
                    if(GetStrPara(EC20Cmd.Buf+10,EC20Cmd.Len,1)==0)
                    {
                        FTPFileLen=GetStrPara(EC20Cmd.Buf+10,EC20Cmd.Len,2);
                        EC20State=STATE_GET_FILE;
                        EC20CmdRespondOK=1;
                        CmdWaitTime=1000;
                        ////////////////////////////////////////////
                        //EC20State = STATE_CLOSE_FTP;
                    }
                }
                break;
            case STATE_GET_FILE:
                if(memcmp(EC20Cmd.Buf,"CONNECT",7)==0)
                {                    
                    FTPFileNeedGetLen=FtpFileGetLen;
                    FTP_BufLen=0;
                }
                else if(memcmp(EC20Cmd.Buf,"+QFTPGET:",9)==0)
                {
                    
                    FtpFileGetStart+= FtpFileGetLen;
                    if(FtpFileGetStart>=FTPFileLen)
                    {
                        EC20State = STATE_CLOSE_FTP;
                    }                        
                    CmdWaitTime=100;
                    EC20CmdRespondOK=1;
                }
                break;
            case STATE_CLOSE_FTP:
                if(memcmp(EC20Cmd.Buf,"+QFTPCLOSE:",11)==0)
                {
                    EC20CmdRespondOK=1;
                    CmdWaitTime=5000;
                    EC20State = PreFtpState;
                    FileSendMark=1;//�ļ�������ϣ��Ͽ�ftp����
                }
                break;
            default:
                CheckOtherRespond();
                break;
        }
        EC20Cmd.State=0;
        EC20Cmd.Len =0;            
    }
}

void EC20_PWRKEY_Ctrl(unsigned char state)
{
    if(state == 0)
    {
        // GSM_PWR=1 ==> PWRKEY=0
        HAL_GPIO_WritePin(IO_GSM_PWR_GPIO, IO_GSM_PWR_PIN, GPIO_PIN_SET);
    }
    else
    {
        // GSM_PWR=0 ==> PWRKEY=1
        HAL_GPIO_WritePin(IO_GSM_PWR_GPIO, IO_GSM_PWR_PIN, GPIO_PIN_RESET);
    }
}

// ��GPRSģ�鷢��ATָ��
// ģ�������ѭ��
// 1ms����1��
unsigned long ResetCount=0;
void EC20MainTask(void)
{
    // ��ʱ���ƣ�180sδ�����ɹ�(�����ɹ���ͻ����STATE_WAIT_DATA����)��������GPRS
    if(EC20State != STATE_WAIT_DATA)
    {
        if(ResetCount > 0)
        {
            ResetCount--;
        }
        else
        {
            EC20State=STATE_POWOFF;
            ResetCount=180000;
        }
    }
    else
    {
        // �������ڵȴ�����(RTU)���������ڷ��͡���һֱ�ȴ�
        ResetCount=180000;
    }
    // �ȴ�GPRS����ATָ��
    if(CmdWaitTime)
    {
        CmdWaitTime--;
        //CheckRespond();
    }
    else// GPRS�Ѿ�������ATָ���ʱδ����
    {
        // GPRS��ʱδ����������������10ʧ�ܣ�������GPRSģ��
        if(EC20CmdRespondOK==0 && CmdRtyTimes==0)
        {
            EC20State=STATE_POWOFF;
        }
        // ��ǰ������������10ʧ�ܣ�������GPRSģ��
        if(tEC20State==EC20State)
        {
            if(CmdRtyTimes)
            {
                CmdRtyTimes --;
            }
            else
            {
                // �ȴ����Ӻ͵ȴ�����������ָ�����
                if(EC20State!=STATE_WAIT_CONNECT && EC20State!=STATE_WAIT_DATA)
                    EC20State=STATE_POWOFF;
                CmdRtyTimes=10;
            }
        }
        else
        {   // �µ�����
            tEC20State=EC20State;
            CmdRtyTimes=10;
            if(tEC20State==STATE_NET_REG)
            {
                CmdRtyTimes=60;
            }
        }
        // ���̿��ƣ�����ATָ��
        switch(EC20State)
        {
            case STATE_POWOFF:
                EC20_PWRKEY_Ctrl(1);
                CmdWaitTime=100;
                EC20CmdRespondOK=1;
                EC20State=STATE_POWSWITCH;
                sprintf(EC20CmdSendBuf,"ģ�鸴λ\r\n");                
                UartSendStr(UART_CHAN_DEBUG,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
                unsigned char tBuf[4];
                tBuf[0] = DTU_OK;
                tBuf[1] = DTU_Version[0];
                tBuf[2] = DTU_Version[1];
                tBuf[3] = DTU_Version[2];
                SendCmdHostMan(tBuf, sizeof(tBuf));
                break;
            case STATE_POWSWITCH:
                EC20_PWRKEY_Ctrl(0);
                CmdWaitTime=300;
                EC20CmdRespondOK=1;
                EC20State=STATE_POWON;
                break;
            case STATE_POWON:
                EC20_PWRKEY_Ctrl(1);
                CmdWaitTime=10000;
                EC20CmdRespondOK=0;
                EC20State=STATE_CHECK_AT;
                break;
            case STATE_CHECK_AT:    // ����'AT'����ʼATָ���
                EC20SendAT();
                CmdWaitTime=1500;
                EC20CmdRespondOK=0;
                break;
            case STATE_CLOSE_ECHO:
                EC20SendATE0();
                CmdWaitTime=500;
                EC20CmdRespondOK=0;
                break;
            case STATE_CHECK_SIM_CARD:   // ���sim��
                EC20SendATCPIN();
                CmdWaitTime=2000;
                CheckRespondStep=0;
                EC20CmdRespondOK=0;
                break;
            case STATE_GET_QCCID:   // ��ȡSIM����
                EC20SendATQCCID();
                CmdWaitTime=2000;
                CheckRespondStep=0;
                EC20CmdRespondOK=0;
                break;
            case STATE_GET_CIMI:    // ��ȡCIMI��
                EC20SendATCIMI();
                CmdWaitTime=2000;
                CheckRespondStep=0;
                EC20CmdRespondOK=0;
                break;
            case STATE_NET_REG:     //ע������
                EC20SendATCREG();
                CmdWaitTime=3000;
                CheckRespondStep=0;
                EC20CmdRespondOK=0;
                break;
            case STATE_CHECK_NETTYPE://��ѯ��������2g/3g/4g
                EC20SendATCEREG();
                CmdWaitTime=2000;
                CheckRespondStep=0;
                EC20CmdRespondOK=0;
                break;  
            case STATE_GET_CSQ:     //��ȡ�ź�ǿ��
                EC20SendATCSQ();
                CmdWaitTime=2000;
                CheckRespondStep=0;
                EC20CmdRespondOK=0;
                break;
            case STATE_WAIT_CONNECT://�ȴ���������
                break;
            case STATE_CONFIG_CONTEXT://����������
                EC20SendATQICSGP();
                CmdWaitTime=2000;
                CheckRespondStep=0;
                EC20CmdRespondOK=0;
                break;
			case STATE_DIACT_CONTEXT:
                EC20SendATQIDEACT();
                CmdWaitTime=3000;
                CheckRespondStep=0;
                EC20CmdRespondOK=0;
				break;
            case STATE_ACT_CONTEXT: //����������
                EC20SendATQIACT();
                CmdWaitTime=10000;
                CheckRespondStep=0;
                EC20CmdRespondOK=0;
                break;
            case STATE_GET_DNS: // ����DNS
                EC20SendATQIDNSCFG();
                CmdWaitTime=2000;
                CheckRespondStep=0;
                EC20CmdRespondOK=0;
                break;
            case STATE_CONNECT_CONTEXT: //����
                EC20SendATQIOPEN("192.168.1.1",1234);
                CmdWaitTime=15000;
                CheckRespondStep=0;
                EC20CmdRespondOK=0;
                break;
            case STATE_SEND_READY: // ѯ��GPRSģ�顢�Ƿ�׼���÷���������
                EC20SendATQISEND(EC20SendLen);
                CmdWaitTime=1000;            
                break;
            case STATE_SENDING:
                UartSendStr(UART_CHAN_EC20,EC20SendBuf ,EC20SendLen );
                EC20State = STATE_SEND_BUSY;
                CmdWaitTime=1000;            
                break;
            case STATE_SEND_BUSY:
                break;
            case STATE_CONFIG_FTP:
                EC20SendATQFTPCFG(0);
                CmdWaitTime=5000;      
                CheckRespondStep=0;
                EC20CmdRespondOK=0;
                break;
            case STATE_CONNECT_FTP:
                EC20SendATQFTPOPEN(FTP_IpAddr,FTP_Port);
                CmdWaitTime=5000;            
                EC20CmdRespondOK=0;
                break;
            case STATE_ENTER_FLODER:
                if(strlen(FTP_FileFloder)>0)
                {
                    EC20SendATQFTPCWD(FTP_FileFloder);
                }
                else
                {
                    EC20SendATQFTPCWD("/");
                }
                CmdWaitTime=5000;            
                EC20CmdRespondOK=0;                    
                break;
            case STATE_GET_FILELEN:
                EC20SendATQFTPLEN(FTP_FileName );
                CmdWaitTime=5000;            
                EC20CmdRespondOK=0;                            
                break;
            case STATE_GET_FILE:
                if(FTPFileLen-FtpFileGetStart>1024)
                    FtpFileGetLen=1024;
                else
                    FtpFileGetLen=FTPFileLen-FtpFileGetStart;
                EC20SendATQFTPGET(FTP_FileName ,FtpFileGetStart ,FtpFileGetLen);
                CmdWaitTime=5000;            
                EC20CmdRespondOK=0;                            
                break;
            case STATE_CLOSE_FTP:
                EC20SendATQFTPCLOSE();
                CmdWaitTime=5000;            
                EC20CmdRespondOK=0;
                break;
            case STATE_WAIT_DATA:
                break;
            default:
                EC20State=STATE_POWOFF;
                break;
        }
    }
}



void FTPWriteFile(unsigned char tch)
{
    FTP_Buf [FTP_BufLen ++]=tch;
    if(FTPFileNeedGetLen==0)
    {
        //EC20State = STATE_CLOSE_FTP;
        //������ɺ�ʼ�ط�����
        //sprintf(EC20CmdSendBuf,"\r\n���ս���:%ld/%ld\r\n",FtpFileGetStart,FTPFileLen);
        //UartSendStr(UART_CHAN_DEBUG,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
        
        Flash_Write(FtpFileGetStart, FTP_Buf, FtpFileGetLen);
        CmdRtyTimes=10;
        sprintf(EC20CmdSendBuf,"\r\n�������� %ld/%ld\r\n",FtpFileGetStart,FTPFileLen);
        UartSendStr(UART_CHAN_DEBUG,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
    }
}

void EC20MakeCmd(unsigned char tch)
{
    if(FTPFileNeedGetLen>0)
    {
        FTPFileNeedGetLen--;
        FTPWriteFile(tch);
        return ;
    }
    
    //�ȴ�����������ʾ��
    if(EC20State==STATE_SEND_READY)
    {
        if(tch=='>')
            EC20State=STATE_SENDING;
        return;
    }
    
    if(EC20State==STATE_SENDING && EC20SendLen >0)
    {
        return;
    }
    
    //����������
    if(EC20RecvedDataLen>0)
    {
        if(EC20RecvedDataLen>EC20RecvLen)
        {
            EC20RecvBuf[EC20RecvLen]=tch;
            EC20RecvLen++;
        }
        else
        {            
            EC20SendAT();
            UartSendStr(UART_CHAN_DEBUG,"SendToHost[D]:",14);  // ���͵�RTU
            // UartSendStr(UART_CHAN_DEBUG,EC20RecvBuf,EC20RecvLen);
            DebugOutHexStr_fast(EC20RecvBuf,EC20RecvLen);
            UartSendStr(UART_CHAN_HOST,EC20RecvBuf,EC20RecvLen);
            EC20RecvedDataLen=0;
        }
        return;
    }
    if(EC20Cmd.State !=9)
    {
        if(EC20Cmd.Len >=4096)
        {
            EC20Cmd.Len =0;
            EC20Cmd.State =0;
        }
        EC20Cmd.Buf[EC20Cmd.Len++]=tch;
    }
    else
    {
        CheckRespond();
    }
    switch(EC20Cmd.State)
    {
        case 0:
            if(tch==0x0d)
                EC20Cmd.State=1;
            break;
        case 1:
            if(tch==0x0a)
            {
                EC20Cmd.State=9;
                if(EC20Cmd.Len>2)
                {
                    CheckRespond();
                }
                else
                {
                    EC20Cmd.State=0;
                    EC20Cmd.Len =0; 
                }
            }
            else if(tch!=0x0d)
                EC20Cmd.State=0;
            break;
        case 2:
            break;
        case 9:
            break;
        default:
            EC20Cmd.State =0;
            break;
    }
}

void EC20PowON(void)
{
    EC20State=STATE_POWOFF;
}

eEC20State EC20_SendData(unsigned char *tBuf,unsigned int tLen)
{
    if(EC20State==STATE_WAIT_DATA)
    {
        EC20SendBuf=tBuf;
        EC20SendLen=tLen;
        EC20State = STATE_SEND_READY;
        return STATE_SENDING;
    }
    else
        return STATE_BUSY;
}



eEC20State EC20_GetFTPFile(char *tIpAddr,unsigned int tPort,char * tFolder,char *tFileName)
{
    // unsigned long i;
    if(EC20State!=STATE_WAIT_DATA && EC20State!=STATE_WAIT_CONNECT)
    {
        return STATE_BUSY;
    }
    sprintf(EC20CmdSendBuf,"����FLASH\r\n");
    UartSendStr(UART_CHAN_DEBUG,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
    FLASH_EraseChip();
//    for(i=0;i<256;i++)
//    {
//        Flash_EraseSector(1024l*4*i);
//    }
    sprintf(EC20CmdSendBuf,"���������ļ�\r\n");
    UartSendStr(UART_CHAN_DEBUG,(uint8_t *)EC20CmdSendBuf,strlen((const char *)EC20CmdSendBuf));
    PreFtpState=EC20State;
    strcpy(FTP_IpAddr,tIpAddr );
    strcpy(FTP_FileName ,tFileName );
    strcpy(FTP_FileFloder,tFolder);
    FTP_Port=tPort;
    EC20State=STATE_CONFIG_FTP;
    FtpFileGetLen=0;
    FtpFileGetStart=0;
    return EC20State;
}

unsigned long GetFtpFileLen(void)
{
    return FTPFileLen;
}


void EC20CloseTCP(void)
{
    EC20SendATQICLOSE();
    EC20State=STATE_TCPCLOSE;
}
