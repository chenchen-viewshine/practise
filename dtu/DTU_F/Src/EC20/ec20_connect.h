#ifndef __EC20_CONNECT_H__
#define __EC20_CONNECT_H__


#define _next_node(p)  (p)
#define _wait(t)       (t)
#define _repeat(t)     (t)

typedef enum
{
    EC20_CMD_AT = 0,
    EC20_CMD_ATE0,
    EC20_CMD_CPIN,
    EC20_CMD_QCCID,
    EC20_CMD_CIMI,
    EC20_CMD_CEREG,
    EC20_CMD_CSQ,
    EC20_CMD_QICSGP,
    EC20_CMD_QIDEACT,
    EC20_CMD_QIACT,
    EC20_CMD_QIDNSCFG,
    EC20_CMD_QIOPEN,
    EC20_CMD_QISEND,
    EC20_CMD_QIRD,
    EC20_CMD_QIGET,  // EC20通知说自己发送了多少个字节过来
    EC20_CMD_MAX
}E_EC20_CMD;

typedef enum 
{
    EC20_CMD_OK,
    EC20_CMD_DONE,
    EC20_CMD_ERROR,
    EC20_CMD_EXCEPTION
}E_EC20_STATUS;

typedef enum 
{
    EC20_STEP_QUERY,
    EC20_STEP_RCEV,
}E_EC20_STEPS;

typedef enum 
{
    EC20_URC_ERROR,
    EC20_URC_MID,
    EC20_URC_END
}E_EC20_URC_MARKS;

typedef enum 
{
    EXCEPRION_CLR,
    EXCEPRION_UNCLR
}E_EXCEPTION_CLEAR;

// 联网
typedef struct 
{
    uint8_t *str;  // urc字符串
    uint8_t  size; // strlen(urc)
    uint8_t  mark; // 0=urc是错误码；1=中间步骤的urc、还有后续的urc；2=收到这个urc就表示指令达到了预期效果、可以结束了
    pT_URC_QUEUE next;
}T_URC_QUEUE, *pT_URC_QUEUE;

typedef struct 
{
    uint8_t *cmdstr; // 指令编号对应的字符串
    uint16_t wait;   // EC20必须在wait时间段内回复一个urc、否则报异常
    uint16_t repeat; // 如果EC20没有回复urc、或者回复报错的urc、等待repeat时间后重发指令
}T_CMD_QUERY, *pT_CMD_QUERY;

typedef struct 
{
    uint8_t  cmd;    // 指令编号
    uint8_t  step;   // 指令执行到了第几步
    uint8_t  status; // 指令执行结果：正常、异常、结束
    uint16_t error_time, // 连续10次报错：重启EC20
    uint16_t except_time,// 连续10次异常：重启EC20
    uint16_t error_count,
    uint16_t except_count,
    pT_CMD_QUERY query[EC20_CMD_MAX];
    pT_URC_QUEUE urc[EC20_CMD_MAX];// EC20回复的urc组成的序列
}T_EC20_ACTS, *pT_EC20_ACTS;

void EC20_ctrl_reset(uint8_t cmd, uint8_t exception)
void EC20_cmd_init(void);
void EC20_connect(void);


#endif // #ifndef __EC20_CONNECT_H__
