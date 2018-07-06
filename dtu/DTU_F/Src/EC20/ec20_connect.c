#include<string.h>
#include"UserMain.h"
#include"ec20_connect.h"

T_EC20_ACTS ec20_act;

// AT
T_CMD_QUERY query_AT[] = { "AT\r\n", _wait(1500), _repeat(1000) };
T_URC_QUEUE urc_AT[] = 
{
    {"OK", strlen("OK"), EC20_URC_END, _next_node(NULL) }
};
// ATE0
T_CMD_QUERY query_ATE0[] = { "ATE0\r\n", _wait(500), _repeat(1000) };
T_URC_QUEUE urc_ATE0[] = 
{
    {"OK", strlen("OK"), EC20_URC_END, _next_node(NULL) }
};
// CPIN
T_CMD_QUERY query_CPIN[] = { "AT+CPIN?\r\n", _wait(2000), _repeat(2000) };
T_URC_QUEUE urc_CPIN[] = 
{
    { "+CME ERROR: 10", strlen("+CME ERROR: 10"), EC20_URC_ERROR, _next_node(urc_CPIN[1]) }
    { "+CPIN: READY",   strlen("+CPIN: READY"),   EC20_URC_MID,   _next_node(urc_CPIN[2]) }, 
    { "OK",             strlen("OK"),             EC20_URC_END,   _next_node(NULL) }, 
};
// QCCID
T_CMD_QUERY query_QCCID[] = { "AT+QCCID\r\n", _wait(2000), _repeat(1000) };
T_URC_QUEUE urc_QCCID[] = 
{
    { "+QCCID:", strlen("+QCCID:"), EC20_URC_ERROR, _next_node(NULL) }
};

void EC20_ctrl_reset(uint8_t cmd, uint8_t exception)
{
    ec20_act.cmd = cmd;
    ec20_act.step = EC20_STEP_QUERY;
    ec20_act.status = EC20_CMD_OK;
    ec20_act.error_count  = 0;
    ec20_act.except_count = 0;
    if(exception == EXCEPRION_CLR)
    {
        ec20_act.error_time  = 0;
        ec20_act.except_time = 0;
    }
}

void EC20_cmd_init(void)
{
    uint8_t i;

    // 复位cmd结构
    EC20_ctrl_reset(EC20_CMD_AT, EXCEPRION_CLR);
    // 初始化每一个cmd的指令表和URC列表
    ec20_act.query[EC20_CMD_AT] = query_AT;
    ec20_act.urc[EC20_CMD_AT]   = urc_AT;

    ec20_act.query[EC20_CMD_ATE0] = query_ATE0;
    ec20_act.urc[EC20_CMD_ATE0]   = urc_ATE0;

    ec20_act.query[EC20_CMD_CPIN] = query_CPIN;
    ec20_act.urc[EC20_CMD_CPIN]   = urc_CPIN;

    ec20_act.query[EC20_CMD_QCCID] = query_QCCID;
    ec20_act.urc[EC20_CMD_QCCID]   = urc_QCCID;
}

uint8_t EC20_step(uint8_t cmd)
{
    uint8_t *urc;
    uint8_t  matched;
    pT_URC_QUEUE urc_head;

    if(ec20_act.step == EC20_STEP_QUERY)
    {
        UartSendStr(UART_CHAN_EC20, ec20_act.cmdstr[cmd], strlen(ec20_act.cmdstr[cmd]));
        ec20_act.step = EC20_STEP_RCEV;
        ec20_act.status = EC20_CMD_OK;
    }
    else
    {
        urc = EC20_get_urc();
        if(urc != NULL)
        {
            matched = 0;
            urc_head = ec20_act.urc[cmd];
            while(1)
            {
                // 匹配urc字符串
                if(memcmp(urc, urc_head->str, urc_head->size)) { matched = 1; break; }
                // 移到下一个urc字符串
                if(urc_head->next == NULL) { break; }
                else                       { urc_head = urc_head->next; }
            }
            if(matched == 1)
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
                        print("urc[%d] has a wrong mark[%d]\r\n", cmd, urc_head->mark);
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
    // 与EC20交互
    EC20_step(ec20_act.cmd);
    // 处理交互结果
    switch(ec20_act.status)
    {
        case EC20_URC_ERROR:
            ec20_act.error_count++;
            if(ec20_act.error_count > ec20_act.repeat[ec20_act.cmd])
            {
                ec20_act.error_count = 0;
                ec20_act.error_time++;
                if(ec20_act.error_time > 10)
                    EC20_set_act(EC20_reset);
                else
                    EC20_ctrl_reset(ec20_act.cmd, EXCEPRION_UNCLR);
            }
            break;
        case EC20_CMD_EXCEPTION:
            ec20_act.except_count++;
            if(ec20_act.except_count > ec20_act.wait[ec20_act.cmd])
            {
                ec20_act.except_count = 0;
                ec20_act.except_time++;
                if(ec20_act.except_time > 10)
                    EC20_set_act(EC20_reset);
                else
                    EC20_ctrl_reset(ec20_act.cmd, EXCEPRION_UNCLR);
            }
            break;
        case EC20_CMD_DONE:
            if(ec20_act.cmd == EC20_CMD_QIOPEN)
            {
                EC20_set_act(EC20_transmit);
                break;
            }
            EC20_ctrl_reset(ec20_act.cmd++, EXCEPRION_CLR);
            if(ec20_act.cmd >= EC20_CMD_MAX)
            {
                print("error: ec20_act.cmd >= EC20_CMD_MAX\r\n");
                EC20_set_act(EC20_reset);
                break;
            }
            break;
        case EC20_CMD_OK:
        default: 
            break;
    }
}
