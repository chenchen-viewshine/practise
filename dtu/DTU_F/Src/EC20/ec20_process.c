#include"ec20_process.h"

/*
* 整个模块分三部分：
* a) 重启EC20
* b) 联网
* c) 收发数据
*/
void (*pEC20_fun)(void);
pEC20_fun EC20_fun = NULL;

void EC20_set_act(pEC20_fun fun)
{
    if(fun != NULL)
    {
        EC20_fun = fun;
    }
}

void EC20_init(void)
{
    EC20_cmd_init();
    EC20_set_act(EC20_reset);
}

// 1ms运行一次
void EC20_process(void)
{
    EC20_fun();
}
