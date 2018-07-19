看大家都在学操作系统，我也想学学。所以想用51写一个来玩玩，发现比较郁闷。

    弄了几下，不想再弄了，51弄这个没啥意思。我用的89S52，除了速度慢，RAM资源太少之外，其它都还过得去。弄了一点代码出来，放在那也没啥用，不如拿上来给新手看看，一个任务调度的雏形是什么样子的~~~~~~~~~这些代码没有经过优化， 我只求实现任务切换的功能。

    利用定时器2产生10mS的定时中断作为时钟节拍，任务切换时保存工作寄存器等操作 嵌入了汇编指令，因此Task_Switch.C文件要做相应的设置才能编译通过。受硬件资源和编译器的限制，有很多无奈。程序只好这样写了，不管怎么说，到底是能调度起来了。

注：这里是老版本，后面又改动的新版本。

/*******************************************************
本程序只供学习使用，未经作者允许，不能用于其它任何用途

AT89S52 MCU 使用24M晶振 时钟节拍设置为10mS

main.c file

Created by Computer-lov.
Date: 2005.10.27

Copyright(C) Computer-lov 2005-2015
All rigths reserved

******************************************************/

#include <at89x52.h>
#include "task_switch.h"
#include "MAIN.H"


//灯
#define LED1 P1_7
#define LED2 P1_6
#define LED3 P1_5
#define LED4 P1_4
#define LED5 P0_1
#define LED6 P3_7

#define ON_LED1()   LED1=0
#define OFF_LED1()  LED1=1

#define ON_LED2()   LED2=0
#define OFF_LED2()  LED2=1

#define ON_LED3()   LED3=0
#define OFF_LED3()  LED3=1

#define ON_LED4()   LED4=0
#define OFF_LED4()  LED4=1

#define ON_LED5()   LED5=0
#define OFF_LED5()  LED5=1

#define ON_LED6()   LED6=0
#define OFF_LED6()  LED6=1

//按钮
#define KEY1 P1_0
#define KEY2 P1_1
#define KEY3 P1_2
#define KEY4 P1_3

//OS运行标志
unsigned char OS_running;

//堆栈申请
unsigned char idata Stack[MAX_TASK][S_DEPTH];

//运行时间
unsigned int Running_Time;

//程序控制块
PCB pcb[MAX_TASK];

//当前运行任务的ID号
unsigned char Current_ID;

 

/////////////////////////////////////调用该函数使任务延时t个时钟节拍////////////////////////
///////////////////////////////////// 输入参数：0<t<256       //////////////////////////////
/////////////////////////////////////  一个时钟节拍为10mS    ///////////////////////////////
void OS_Delay(unsigned char t)
{
    EA=0;                         //关中
    pcb[Current_ID].Suspend=1;    //任务挂起
    pcb[Current_ID].Delay=t;      //设置延迟节拍数
    EA=1;                         //开中
    task_switch();                //任务切换
}
////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////挂起任务 ////////////////////////////////////////////////
/*void OS_Suspend(void)
{
EA=0;
pcb[Current_ID].Suspend=1;    //任务挂起
EA=1;
task_switch();                //任务切换
}*/
////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////创建一个任务////////////////////////////////////////////
///////////////////////////////函数入口：Task_ID 分配给任务的唯一ID号 //////////////////////////
//////////////////////////////           Task_Priority  任务优先级     /////////////////////////
///////////////////////////////          Task_p    任务入口地址
///////////////////////////////          Stack_p   任务堆栈栈低地址   ///////////////////////////
void Task_Create(unsigned char Task_ID,unsigned char Task_Priority,unsigned int Task_p,unsigned char Stack_p)
{
    unsigned char i;

    for(i=0;i<S_DEPTH;i++)
    {
        ((unsigned char idata *)Stack_p)[i]=0;         //初始化清空堆栈
    }

    ((unsigned char idata *)Stack_p)[0]=Task_p;         //将任务入口地址保存在堆栈
    ((unsigned char idata *)Stack_p)[1]=Task_p>>8;

    pcb[Task_ID].Task_SP=Stack_p+Num_PUSH_bytes+1;   //设置好堆栈指针
    pcb[Task_ID].Priority=Task_Priority;             //设置任务优先级
    pcb[Task_ID].Suspend=0;                          //任务初始不挂起
    pcb[Task_ID].Delay=0;                            //任务初始不延时
}
/////////////////////////////////////////////////////////////////////////////////////////////

 

 

/////////////////////////////////////空闲任务，优先级最低///////////////////////////////////
////////////////////////////////////二个LED不停的闪烁 //////////////////////////////////////
void task_idle(void)
{
    static unsigned long int i;    //使用static申明局部变量，避免临时变量使用相同地址
    while(1)
    {
        ON_LED1();    //LED1亮
        for(i=0;i<0x2000;i++)        //延迟
        {
        }
        OFF_LED1();   //LED1关
        for(i=0;i<0x2000;i++)
        {
            ON_LED6();    //LED6闪烁很快，看起来是一直亮的
            OFF_LED6();
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////任务1  检测按钮1 并控制LED2亮灭//////////////////////////
void task_1(void)
{
    // static unsigned int  j;
    while(1)
    {
        ON_LED2();
        while(KEY1)OS_Delay(6);     //等待KEY1按键按下
        while(!KEY1)OS_Delay(6);    //等待KEY1释放
        OFF_LED2();
        while(KEY1)OS_Delay(6);
        while(!KEY1)OS_Delay(6);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////任务2  检测按钮2 并控制LED3亮灭//////////////////////////
void task_2(void)
{
    // static unsigned int  j;
    while(1)
    {
        ON_LED3();
        while(KEY2)OS_Delay(5);
        while(!KEY2)OS_Delay(5);
        OFF_LED3();
        while(KEY2)OS_Delay(5);
        while(!KEY2)OS_Delay(5);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////任务3  检测按钮3 并控制LED4亮灭//////////////////////////
void task_3(void)
{
    // static unsigned int  j;
    while(1)
    {
        ON_LED4();
        while(KEY3)OS_Delay(5);
        while(!KEY3)OS_Delay(5);
        OFF_LED4();
        while(KEY3)OS_Delay(5);
        while(!KEY3)OS_Delay(5);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////任务4  控制LED5每秒闪一次//////////////////////////
void task_4(void)
{
    // static unsigned int  j;
    while(1)
    {
        ON_LED5();
        OS_Delay(100);   //LED5每隔1S闪一次
        OFF_LED5();
        OS_Delay(100);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////主函数//////////////////////////////////////////////////////
void main(void)
{
    EA=0;                    //关中
    ET2=1;                   //定时器2开中断

    T2CON=0x00;              //定时器自动重装模式
    T2MOD=0x00;              //如果提示这里编译通不过，可将本行删除；或自己将定义添上
                            //因为keil自带的at89x52.h中没有T2MOD的定义
    RCAP2H=0xB1;
    RCAP2L=0xE0;             //定时时间为10ms

    Task_Create(0,5,(unsigned int)(void *)(&task_idle),(unsigned char)Stack[0]);   //任务0初始化
    Task_Create(1,4,(unsigned int)(void *)(&task_1),(unsigned char)Stack[1]);      //任务1初始化
    Task_Create(2,3,(unsigned int)(void *)(&task_2),(unsigned char)Stack[2]);      //任务2初始化
    Task_Create(3,2,(unsigned int)(void *)(&task_3),(unsigned char)Stack[3]);      //任务3初始化
    Task_Create(4,1,(unsigned int)(void *)(&task_4),(unsigned char)Stack[4]);      //任务4初始化

    OS_running=0;                          //任务未开始运行

    Current_ID=MAX_TASK-1;                 //当前任务为最后一个任务

    pcb[Current_ID].Task_SP-=Num_PUSH_bytes;    //调整任务堆栈指针，因为这时任务还未开始调度
                                               //第一次进入中断时，会压栈。所以先将堆栈指针
                                               //往下调Num_PUSH_bytes个字节，避免堆栈溢出
                                               //调整后的SP紧接着的两个字节就是最后一个任务的入口地址
                                               //在第一次中断发生时，返回地址被压入SP后面的两个地址
                                               //在第一次进入中断后，将SP往前调整两字节，这样程序返回时，
                                               //将返回到最后一个任务，而不再返回主函数

    SP=pcb[Current_ID].Task_SP;                   //修改堆栈指针。使其指向任务当前任务的堆栈段

    TR2=1;             //启动定时器2
    EA=1;              //开中断

    while(1);          //死循环。定时器中断发生后，任务开始调度
}
