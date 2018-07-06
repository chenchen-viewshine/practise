#ifndef __EC20_PROCESS_H__
#define __EC20_PROCESS_H__


void (*pEC20_fun)(void);
void EC20_set_act(pEC20_fun fun);
void EC20_process(void);


#endif // #ifndef __EC20_PROCESS_H__
