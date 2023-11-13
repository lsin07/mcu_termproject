#include "LPUART.h"

void LPUART1_init(void)
{
	PCC->PCCn[PCC_LPUART1_INDEX] &= ~(PCC_PCCn_CGC_MASK); // disable clock for config
	PCC->PCCn[PCC_LPUART1_INDEX] &= ~(PCC_PCCn_PCS(7));
	PCC->PCCn[PCC_LPUART1_INDEX] |= PCC_PCCn_PCS(2);
	PCC->PCCn[PCC_LPUART1_INDEX] |= PCC_PCCn_CGC_MASK;

	LPUART1->BAUD &= ~(LPUART_BAUD_M10_MASK);
	LPUART1->BAUD &= ~(LPUART_BAUD_SBNS_MASK);

	LPUART1->BAUD &= ~(LPUART_BAUD_OSR(31));
	LPUART1->BAUD |= LPUART_BAUD_OSR(15);
	LPUART1->BAUD &= ~(LPUART_BAUD_SBR(8191));
	LPUART1->BAUD |= LPUART_BAUD_SBR(52);

	LPUART1->CTRL &= ~(LPUART_CTRL_M7_MASK);
	LPUART1->CTRL &= ~(LPUART_CTRL_M_MASK);
	LPUART1->CTRL &= ~(LPUART_CTRL_PE_MASK);

	LPUART1->CTRL |= LPUART_CTRL_TE_MASK;
	LPUART1->CTRL |= LPUART_CTRL_RE_MASK;
}

void LPUART1_transmit_byte(uint8_t send)
{
	while ((LPUART1->STAT & LPUART_STAT_TDRE_MASK) == 0)
		;

	LPUART1->DATA = send;
}

void LPUART1_transmit_halfW(uint16_t send)
{
	uint8_t send_b[2];
	send_b[0] = (uint8_t)((send & 0xFF00) >> 8);
	send_b[1] = (uint8_t)((send & 0xFF));
	LPUART1_transmit_byte(send_b[0]);
	LPUART1_transmit_byte(send_b[1]);
}

void LPUART1_transmit_word(uint32_t send)
{
	uint8_t send_b[4];
	send_b[0] = (uint8_t)((send & 0xFF000000) >> 24);
	send_b[1] = (uint8_t)((send & 0xFF0000) >> 16);
	send_b[2] = (uint8_t)((send & 0xFF00) >> 8);
	send_b[3] = (uint8_t)((send & 0xFF));
	LPUART1_transmit_byte(send_b[0]);
	LPUART1_transmit_byte(send_b[1]);
	LPUART1_transmit_byte(send_b[2]);
	LPUART1_transmit_byte(send_b[3]);
}