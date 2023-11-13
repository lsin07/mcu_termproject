#ifndef LPUART_H_
#define LPUART_H_
#include "device_registers.h"	/* include peripheral declarations S32K144 */

void LPUART1_init(void);
void LPUART1_transmit_byte(uint8_t send);
void LPUART1_transmit_halfW(uint16_t send);
void LPUART1_transmit_word(uint32_t send);

#endif