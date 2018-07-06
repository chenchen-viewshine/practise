#include"ec20_init.h"

// 初始化EC20
// 重启EC20

void EC20_act_0ff(void)
{
    uint8_t tBuf[4];
    if(delay == 0)
    {
        tBuf[0] = DTU_OK;
        tBuf[1] = DTU_Version[0];
        tBuf[2] = DTU_Version[1];
        tBuf[3] = DTU_Version[2];
        SendCmdHostMan(tBuf, sizeof(tBuf));
        EC20_PWRKEY_Ctrl(1);
        print("模块复位\r\n");
    }
}
void EC20_act_key(void)
{
    EC20_PWRKEY_Ctrl(0);
}
void EC20_act_on(void)
{
    EC20_PWRKEY_Ctrl(1);
}

void EC20_reset(void)
{
    static uint16_t step  = 0;
    static uint16_t delay = 0;

    if(delay > 0)
    {
        delay--;
        return;
    }
    switch(step)
    {
        case 0: 
            EC20_act_0ff();
            delay = 50;
            step  = 1;
            break;
        case 1:
            EC20_act_key();
            delay = 200;
            step  = 2;
            break;
        case 2:
            EC20_act_on();
            delay = 10000;
            step  = 3;
            break;
        case 3:
        default:
            step  = 0;
            delay = 0;
            EC20_ctrl_reset(EC20_CMD_AT, EXCEPRION_CLR);
            EC20_set_act(EC20_connect);
            break;
    }
}
