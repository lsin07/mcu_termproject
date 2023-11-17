#ifndef PWM_H_
#define PWM_H_
#include "device_registers.h"

void FTM_init(void);
void FTM0_CH2_PWM(uint32_t);

#endif