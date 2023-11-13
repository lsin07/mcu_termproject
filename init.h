#ifndef init_H_
#define init_H_
#include "device_registers.h"	/* include peripheral declarations S32K144 */

void PORT_init(void);
void WDOG_disable(void);
void LPIT0_init(uint32_t);

#endif
