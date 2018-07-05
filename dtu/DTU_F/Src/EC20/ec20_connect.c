#include "ec20_connect.h"

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
    EC20_STEP_SEND,
    EC20_STEP_RCEV,
}E_EC20_STEPS;

typedef enum 
{
    EC20_URC_ERROR,
    EC20_URC_MID,
    EC20_URC_END
}E_EC20_URC_MARKS;

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
    uint8_t  step;   // 指令执行到了第几步
    uint8_t  status; // 指令执行结果：正常、异常、结束

    uint8_t  id;     // 指令编号
    uint8_t *cmdstr[EC20_CMD_MAX]; // 指令编号对应的字符串
    uint16_t count;  // 发送了指令字符串之后、我们在这个时间段内、等待EC20回复一个urc过来
    uint16_t wait[EC20_CMD_MAX];
    uint16_t repeat[EC20_CMD_MAX];  // 如果EC20没有回复urc、或者回复报错的urc、需要等待一段时间后重发指令
    pT_URC_QUEUE urc[EC20_CMD_MAX];// EC20回复的urc组成的序列
}T_EC20_ACTS, *pT_EC20_ACTS;

T_EC20_ACTS ec20_act;

// AT
T_URC_QUEUE urc_AT_START = 
{
    .str  = "OK",
    .size = strlen("OK"),
    .mark = EC20_URC_END,
    .next = NULL;
};
// ATE0
T_URC_QUEUE urc_ATE0_START = 
{
    .str  = "OK",
    .size = strlen("OK"),
    .mark = EC20_URC_END,
    .next = NULL;
};
// CPIN
T_URC_QUEUE urc_CPIN_END = 
{
    .str  = "OK",
    .size = strlen("OK"),
    .mark = EC20_URC_END,
    .next = NULL;
};

T_URC_QUEUE urc_CPIN_MID1 = 
{
    .str  = "+CPIN: READY",
    .size = strlen("+CPIN: READY"),
    .mark = EC20_URC_END,
    .next = urc_CPIN_END;
};

T_URC_QUEUE urc_CPIN_START = 
{
    .str  = "+CME ERROR: 10",
    .size = strlen("+CME ERROR: 10"),
    .mark = EC20_URC_END,
    .next = urc_CPIN_MID1;
};

void EC20_reset_cmd(uint8_t cmd)
{
    if(id < _countof(ec20_acts))
    {
        ec20_acts[cmd].cmd.id     = cmd;
        ec20_acts[cmd].cmd.cmdstr = NULL;
        ec20_acts[cmd].cmd.step   = 1;
        ec20_acts[cmd].cmd.status = EC20_CMD_OK;
    }
}

void EC20_cmd_init(void)
{
    uint8_t i;

    // 复位cmd结构
    for(i = 0; i < _countof(ec20_urc); i++)
    {
        EC20_reset_cmd(i);
    }
    // 初始化每一个cmd的URC列表
    ec20_act.urc[EC20_CMD_AT]   = &urc_AT_START;
    ec20_act.urc[EC20_CMD_ATE0] = &urc_ATE0_START;
    ec20_act.urc[EC20_CMD_CPIN] = &urc_CPIN_START;
}

uint8_t ec20_cmd = EC20_CMD_AT;
uint16_t error_count  = 0;
uint16_t except_count = 0;

void EC20_reset_count(void)
{
    ec20_cmd = EC20_CMD_AT;
    error_count  = 0;
    except_count = 0;
}

uint8_t EC20_step(uint8_t cmd)
{
    uint8_t *urc;
    uint8_t  match;
    pT_URC_QUEUE urc_head;

    ec20_act.count++;
    if(ec20_act.step == EC20_STEP_SEND)
    {
        EC20_send_str(ec20_act.cmdstr[cmd], strlen(ec20_act.cmdstr[cmd]));
        ec20_act.step = EC20_STEP_RCEV;
        ec20_act.status = EC20_CMD_OK;
    }
    else
    {
        urc = EC20_get_urc();
        if(urc != NULL)
        {
            match = 0;
            urc_head = ec20_act.urc[cmd];
            while(1)
            {
                // 匹配urc字符串
                if(memcmp(urc, urc_head->str, urc_head->size)) { match = 1; break; }
                // 移到下一个urc字符串
                if(urc_head->next == NULL) { break; }
                else                       { urc_head = urc_head->next; }
            }
            if(match == 1)
            {
                switch(urc_head->mark)
                {
                    case EC20_URC_ERROR:
                        ec20_act.status = EC20_CMD_ERROR;
                        break;
                    case EC20_URC_MID:
                        ec20_act.status = EC20_CMD_OK;
                        break;
                    case EC20_URC_END:
                        ec20_act.status = EC20_CMD_DONE;
                        break;
                    default: 
                        ec20_act.status = EC20_CMD_EXCEPTION;
                        break;
                }
            }
            else
            {
                // 不是这个指令所预期的urc
                ec20_act.status = EC20_CMD_EXCEPTION;
            }
        }
        else
        {
            // 没收到urc
            ec20_act.status = EC20_CMD_EXCEPTION;
        }
    }
}

void EC20_connect(void)
{
    uint8_t ec20_status;

    ec20_status = EC20_step(ec20_cmd);
    // 返回结果
    switch(ec20_status)
    {
        case EC20_URC_ERROR:
            if(ec20_act.count > ec20_act.repeat[ec20_cmd])
            {
                error_count++;
                if(error_count > 10)
                    EC20_set_act(EC20_reset);
            }
            break;
        case EC20_CMD_EXCEPTION:
            if(ec20_act.count > ec20_act.wait[ec20_cmd])
            {
                except_count++;
                if(except_count > 10)
                    EC20_set_act(EC20_reset);
            }
            break;
        case EC20_CMD_DONE:
            if(ec20_cmd == EC20_CMD_QIOPEN)
            {
                EC20_set_act(EC20_transmit);
                break;
            }
            if(ec20_cmd >= EC20_CMD_MAX)
            {
                print("error: ec20_cmd >= EC20_CMD_MAX\r\n");
                EC20_set_act(EC20_reset);
                break;
            }
            ec20_cmd++;
            EC20_reset_count();
            break;
        case EC20_CMD_OK:
        default: 
            break;
    }
}
