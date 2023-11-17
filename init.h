#ifndef INIT_H_
#define INIT_H_
#include "device_registers.h"

void PORT_init(void);
void WDOG_disable(void);
void LPIT0_init(void);
void NVIC_init_IRQs(void);

#endif