#include "stm32f1xx_hal.h"
#include "array.h"
#include "UserMain.h"

#define UartChanDebug   0
#define UartChanEC20    1
#define UartChanHost    2

#define UART_DEBUG_BUFLEN       32
#define UART_DEBUGTX_BUFLEN     256
#define UART_EC20_BUFLEN        128
#define UART_HOST_BUFLEN        32


T_Array ArrayUartRX[3];
T_Array ArrayUartTX[1];
uint8_t UartRXBufDebug[UART_DEBUG_BUFLEN];      //调试端口接收缓存
uint8_t UartTXBufDebug[UART_DEBUGTX_BUFLEN];    //调试端口发送缓存
uint8_t UartRXBufEC20[UART_EC20_BUFLEN];        //ec20模块接收缓存
uint8_t UartRXBufHost[UART_HOST_BUFLEN];        //主机通信接收缓存

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

void DebugOutStr(uint8_t *tBuf,uint16_t tLen)
{
//    while(tLen--)
//    {
//        ArrayWrite(&ArrayUartTX[UartChanDebug],*buf);
//        buf++;
//    }
//    __HAL_UART_ENABLE_IT(&huart2, UART_IT_TXE);
    
    while(tLen--)
    {
        huart2.Instance->DR=*tBuf;
        tBuf++;
        while(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TXE) == RESET)
        ;
    }
    
}

void DebugOutHexStr(uint8_t *buf,uint16_t tLen)
{
    uint8_t tDebugStr[5];
    while(tLen--)
    {
        sprintf((char *)tDebugStr,"%02X ",*buf);
        DebugOutStr(tDebugStr,3);
        buf++;
    }
    
    sprintf((char *)tDebugStr,"\r\n");
    DebugOutStr(tDebugStr,2);
}

// 仅用来发送hex数据
void DebugOutHexStr_fast(uint8_t *buf,uint16_t tLen)
{
    uint8_t temp;
    uint8_t tDebugStr[5];
    while(tLen--)
    {
        // sprintf((char *)tDebugStr,"%02X ",*buf);
        temp = buf[0] >> 4;
        if(temp < 10) { tDebugStr[0] = temp + '0'; }
        else          { tDebugStr[0] = temp + 'A' - 10; }
        temp = buf[0] & 0x0F;
        if(temp < 10) { tDebugStr[1] = temp + '0'; }
        else          { tDebugStr[1] = temp + 'A' - 10; }
        tDebugStr[2] = ' ';
        DebugOutStr(tDebugStr,3);
        buf++;
    }
    DebugOutStr("\r\n",2);
}

void UartSendStr(uint8_t chan,uint8_t *tBuf,uint16_t tLen)
{
    if(chan==UART_CHAN_DEBUG || chan==UART_CHAN_EC20)
    {
        DebugOutStr(tBuf,tLen);
    }
    
    if(chan==UART_CHAN_HOST)
    {
        while(tLen--)
        {
            huart1.Instance->DR=*tBuf;
            tBuf++;
            while(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE) == RESET)
            ;
        }
    }    
    else if(chan==UART_CHAN_EC20)
    {
        while(tLen--)
        {
            huart3.Instance->DR=*tBuf;
            tBuf++;
            while(__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TXE) == RESET)
            ;
        }
    }
}


//串口接收中断回调函数
void CallBackUartRecv(unsigned char tChan,unsigned char tch)
{
    if(tChan==UART_CHAN_HOST)
    {
        ArrayWrite(&ArrayUartRX[UartChanHost],tch);
        //DebugOutStr(&tch,1);
    }    
    else if(tChan==UART_CHAN_EC20)
    {
        ArrayWrite(&ArrayUartRX[UartChanEC20],tch);
        DebugOutStr(&tch,1);
    }
    else if(tChan==UART_CHAN_DEBUG)
    {
        ArrayWrite(&ArrayUartRX[UartChanDebug],tch);
    }
}


//串口发送中断回调函数
void CallBackUartSend(unsigned tChan)
{
    uint8_t tch;
    if(tChan==UART_CHAN_DEBUG)
    {
        if(ArrayRead(&ArrayUartTX[UartChanDebug],&tch))
            huart2.Instance->DR=tch;//  HAL_UART_Transmit(&huart1,&tch,1,100);
        else
            __HAL_UART_DISABLE_IT(&huart2, UART_IT_TXE);        
    }
    else if(tChan==UART_CHAN_EC20)
    {
    }
    else if(tChan==UART_CHAN_HOST)
    {
    }
}


void OpenUartRXInt(void)
{
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
}


void UserInit(void)
{
    //初始化队列
    ArrayInit(&ArrayUartRX[UartChanDebug]);
    ArrayInit(&ArrayUartRX[UartChanEC20]);
    ArrayInit(&ArrayUartRX[UartChanHost]);
    ArrayUartRX[UartChanDebug].MaxKids=UART_DEBUG_BUFLEN;
    ArrayUartRX[UartChanDebug].Kid=UartRXBufDebug;
    ArrayUartRX[UartChanEC20].MaxKids=UART_EC20_BUFLEN;
    ArrayUartRX[UartChanEC20].Kid=UartRXBufEC20;
    ArrayUartRX[UartChanHost].MaxKids=UART_HOST_BUFLEN;
    ArrayUartRX[UartChanHost].Kid=UartRXBufHost;
    
    
    ArrayInit(&ArrayUartTX[UartChanDebug]);
    ArrayUartTX[UartChanDebug].MaxKids=UART_DEBUGTX_BUFLEN;
    ArrayUartTX[UartChanDebug].Kid=UartTXBufDebug;
    
    //开接收中断
    OpenUartRXInt();
    //    
}



uint8_t TaskCheckCmd;
extern void EC20MainTask(void);
extern void HostMainTask(void);

extern void EC20MakeCmd(unsigned char tch);
extern void MakeCmd(unsigned char tChan,unsigned char tch);
void UserMain(void)
{
    uint8_t tch;
    if(TaskCheckCmd)
    {
        TaskCheckCmd=0;
        //传送接收到的数据
        while(ArrayRead(&ArrayUartRX[UartChanDebug],&tch))
        {
            MakeCmd(UART_CHAN_DEBUG,tch);
        }
        
        while(ArrayRead(&ArrayUartRX[UartChanEC20],&tch))
        {
            EC20MakeCmd(tch);
        }
        
        while(ArrayRead(&ArrayUartRX[UartChanHost],&tch))
        {
            MakeCmd(UART_CHAN_HOST,tch);
        }
        
        // 解析模块指令
        EC20MainTask();
        // 解析主机(RTU)指令
        HostMainTask();
    }
}



//系统ms定时器
void HAL_SYSTICK_Callback(void)
{
    TaskCheckCmd=1;
}
