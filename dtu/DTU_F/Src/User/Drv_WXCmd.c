#include <string.h>
#include "Drv_WXCmd.h"
#include "Drv_EC20.h"
#include "UserMain.h"
#include "FLASH_MX25.h"
typedef struct 
{
    unsigned char Buf[WX_CMD_BUF_LEN];
    unsigned int Len;
    unsigned int Timeout;
    unsigned char State;
}StuCmd;

StuCmd WXCmdRecv[2],TcpSendCmd,WXCmdSend;



void MakeCmd(unsigned char tChan,unsigned char tch)
{
    unsigned char DataChan;
    if(tChan==UART_CHAN_DEBUG)
    {
        DataChan=0;
    }
    else if(tChan==UART_CHAN_HOST)
    {
        DataChan=1;
    }
    
    WXCmdRecv [DataChan ].Timeout =10;
    if(WXCmdRecv [DataChan ].Len <WX_CMD_BUF_LEN)
    {
        WXCmdRecv [DataChan ].Buf [WXCmdRecv [DataChan ].Len ++]=tch;
    }
    else
    {
        if(DataChan ==1)
        {
            EC20_SendData(WXCmdRecv [DataChan].Buf ,WXCmdRecv [DataChan ].Len);
        }
        WXCmdRecv [DataChan ].State =0;
        WXCmdRecv [DataChan ].Len = 0; 
    }
}


char strIP[16],strPath[33],strFile[17];
unsigned int tPort;

//发送指令，加校验
void CmdSend(stu_cmd_str *T_CMD, unsigned char tChan)
{
    unsigned int i, j;
    unsigned char Tcrc;

    j = 0x100 * T_CMD->DataLenHi + T_CMD->DataLenLo + 10;
    Tcrc = 0;
    for(i = 0; i < j + 1; i++)
        Tcrc += *(((unsigned char*)T_CMD)+i);

    *(((unsigned char*)T_CMD)+j+1)=Tcrc;
    *(((unsigned char*)T_CMD)+j+2)=0x16;
    
    if(tChan==0)
    {
        DebugOutStr("Debug Ack:",10);
        DebugOutHexStr((unsigned char *)T_CMD,j+3);
    }
    else
    {
        DebugOutStr("SendToHost[c]:",14);  // 发送到RTU
        DebugOutHexStr((unsigned char *)T_CMD,j+3);
        UartSendStr(UART_CHAN_HOST,(unsigned char *)T_CMD,j+3);
    }
}

//受到数据给应答
void CmdSendACK(unsigned char tAckConCode,stu_cmd_str *tCmd,unsigned char tChan)
{
    tCmd->ConCode=tAckConCode;
    tCmd->DataLenLo=10;
    tCmd->DataLenHi=0;
    
    CmdSend(tCmd,tChan);
}


stu_cmd_str *Cmd;
unsigned char FileSendMark,FileSendAck,FileSendRty;
unsigned long FileSendLen,FileSendTimeout,FileSendProcess;
unsigned char APNStr[30] = "CMNET",IPStr[6]={115,236,33,164, 0x1f, 0x62};
unsigned char UserName[30] = "";
unsigned char PassWord[30] = "";
// 解析RTU发来的指令
unsigned char CheckWXCmd(unsigned char tChan)
{
    unsigned int CmdCode;

    if(WXCmdRecv[tChan].Len < 21)
        return 1;
    Cmd = (stu_cmd_str*)WXCmdRecv[tChan].Buf;
    if((Cmd->Head1_68 == 0x68) && (Cmd->Head2_68 == 0x68))
    {
        CmdCode = Cmd->CmdCodeHi * 0x100 + Cmd->CmdCodeLo;
        if(CmdCode == CMD_OPCODE_GSMMODEL)
        {
            if(Cmd->databuf[0] == 0xff)
            {
                EC20CloseTCP();
            }
            else
            {
                memcpy(APNStr,   &Cmd->databuf[0],  30);
                memcpy(UserName, &Cmd->databuf[30], 30);
                memcpy(PassWord, &Cmd->databuf[60], 30);
                memcpy(IPStr,    &Cmd->databuf[90], 6);
            }
        }
        else if(CmdCode == CMD_OPCODE_GETFTP)
        {
            sprintf(strIP, "%d.%d.%d.%d", Cmd->databuf[0], Cmd->databuf[1], Cmd->databuf[2], Cmd->databuf[3]);
            tPort = 0x100 * Cmd->databuf[4] + Cmd->databuf[5];
            sprintf(strPath, "%s", Cmd->databuf + 6 );
            sprintf(strFile, "%s", Cmd->databuf + 38);
            
            if(EC20_GetFTPFile(strIP, tPort, strPath, strFile) == STATE_BUSY)
            {
                CmdSendACK(CON_BIT_DIR | CON_BIT_ERR | CON_BIT_ACK, Cmd, tChan);
            }
            else
            {
                CmdSendACK(CON_BIT_DIR | CON_BIT_ACK, Cmd, tChan);
                FileSendMark = 0;                
            }
        }
        else if(CmdCode == CMD_OPCODE_FTPTRAN)
        {
            if(FileSendLen - FileSendProcess > 128)
            {
                FileSendProcess += 128;
                FileSendAck = 1;
            }
            else
            {
                FileSendProcess = FileSendLen;
                FileSendMark=0;
            }
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 1;
    }
    return 0;
}


void SendCmdHostMan(unsigned char *tch,unsigned int tLen)
{
    unsigned int i,cmdlen;
    stu_cmd_str *tCmd;
    tCmd=(stu_cmd_str *)WXCmdSend.Buf;
    tCmd->Head1_68=0x68;
    memset(tCmd->Target,0xFF,6);
    tCmd->Head2_68=0x68;
    tCmd->ConCode=0x05;
    tCmd->CmdCodeLo=CMD_OPCODE_GSMMODEL % 0x100;
    tCmd->CmdCodeHi=CMD_OPCODE_GSMMODEL / 0x100;
	  memset(tCmd->Time,0xFF,6);
    cmdlen=10+tLen;
    
    tCmd->DataLenLo=cmdlen%0x100;
    tCmd->DataLenHi=cmdlen/0x100;
    
    for(i=0;i<tLen;i++)
    {
        tCmd->databuf[i]=*(tch+i);
    }    
    CmdSend(tCmd,1);    
}

//发送ftp文件
unsigned char FtpDataBuf[256];
void FtpFileSend(void)
{
//    unsigned long i=0;
    stu_cmd_str *tCmd;
    tCmd=(stu_cmd_str *)WXCmdSend.Buf;
    switch(FileSendMark)
    {
        case 0://未获取文件或文件接收中
            break;
        case 1://文件接收完毕
            //获取文件长度
            FileSendLen=GetFtpFileLen();
            FileSendRty=0;
            FileSendMark =2;
            FileSendProcess=0;
            //打印接收的文件数据
//            while(i<FileSendLen)
//            {
//                if(FileSendLen-i>128)
//                {
//                    Flash_Read(i,FtpDataBuf,128);
//                    i+=128;
//                    DebugOutStr(FtpDataBuf,128);
//                }
//                else
//                {
//                    Flash_Read(i,FtpDataBuf,FileSendLen-i);
//                    DebugOutStr(FtpDataBuf,FileSendLen-i);
//                    i=FileSendLen;
//                }
//            }
        
            break;
        case 2://向主机发送文件中
            FileSendRty++;
            if(FileSendRty>5)
            {
                FileSendMark=0;
            }
            else
            {
                //读取文件
                tCmd->Head1_68=0x68;
                memset(tCmd->Target,0xaa,6);
                tCmd->Head2_68=0x68;
                tCmd->ConCode=0x01;
                tCmd->CmdCodeLo=CMD_OPCODE_FTPTRAN % 0x100;
                tCmd->CmdCodeHi=CMD_OPCODE_FTPTRAN / 0x100;
                
                tCmd->databuf[0]=(FileSendProcess/128+1)%0x100;
                tCmd->databuf[1]=(FileSendProcess/128+1)/0x100;
                tCmd->databuf[2]=(FileSendLen/128)%0x100;
                tCmd->databuf[3]=(FileSendLen/128)/0x100;
                
                tCmd->SetType=0x02;
                    
                if(FileSendLen-FileSendProcess>128)
                {
                    Flash_Read(FileSendProcess,(char *)tCmd->databuf+4,128);
                    tCmd->DataLenLo=(128+14)%0x100;
                    tCmd->DataLenHi=(128+14)/0x100;
                }
                else
                {
                    Flash_Read(FileSendProcess,(char *)tCmd->databuf+4,FileSendLen-FileSendProcess);
                    tCmd->DataLenLo=(FileSendLen-FileSendProcess+14)%0x100;
                    tCmd->DataLenHi=(0+10)/0x100;
                }
                
                CmdSend(tCmd,1);
                FileSendAck=0;
                FileSendMark=3;
                FileSendTimeout=5000;
            }
            break;
        case 3://等待应答
            if(FileSendTimeout>0)
                FileSendTimeout--;
            else
                FileSendMark=2;
            
            if(FileSendAck==1)
            {
                FileSendRty=0;
                FileSendMark=2;
                
            }
            break;
        default:
            break;
    }
}

//主机主循环
unsigned char MeState=0;
unsigned int MeTimeout=0;
void HostMainTask(void)
{
    unsigned char i;
//    unsigned char tBuf;
    //检测是否收到数据
    for(i=0;i<2;i++)
    {
        if(WXCmdRecv [i].Timeout>0)
        {
            WXCmdRecv [i].Timeout--;
        }
        else
        {
            if(WXCmdRecv[i].Len >0)
            {
                if(i==1)
                {
                    DebugOutStr("Recv_Host:",10);  // 收到RTU的数据
                }
                else
                {
                    DebugOutStr("Recv_Debug:",11);
                }
                DebugOutHexStr(WXCmdRecv[i].Buf ,WXCmdRecv[i].Len);
                if(CheckWXCmd(i)!=0)
                {
                    if(i==1)
                    {
                        //透传数据
                        EC20_SendData(WXCmdRecv[i] .Buf ,WXCmdRecv[i].Len);
                    }
                }
                WXCmdRecv [i].Len=0;
            }     
        }
    }
    if(MeTimeout)
    {
        MeTimeout --;
    }
    else
    {
        switch(MeState)
        {
            case 0:
                MeTimeout =3000;
                MeState=1;
                break;
            case 1:
                MeState=2;
                break;
            case 2:
                break;
            default:
                MeState=0;
                break;
        }
    }
    
    
    
    FtpFileSend();
}








