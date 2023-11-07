#include "device_registers.h"
#include "clocks_and_modes.h"
#include "ADC.h"

int lpit0_ch0_flag_counter = 0;

void PORT_init(void)
{
	PCC->PCCn[PCC_PORTC_INDEX] = PCC_PCCn_CGC_MASK;
	PORTC->PCR[6] &= PORT_PCR_MUX(7);
	PORTC->PCR[6] |= PORT_PCR_MUX(2);
	PORTC->PCR[7] &= PORT_PCR_MUX(7);
	PORTC->PCR[7] |= PORT_PCR_MUX(2);
	PORTC->PCR[12] &= PORT_PCR_MUX(7);
	PORTC->PCR[12] |= PORT_PCR_MUX(1);
	PORTC->PCR[13] &= PORT_PCR_MUX(7);
	PORTC->PCR[13] |= PORT_PCR_MUX(1);
	PTC->PDDR &= ~((1 << 12) | (1 << 13));

	PCC->PCCn[PCC_PORTD_INDEX] = PCC_PCCn_CGC_MASK;
	PORTD->PCR[0] &= PORT_PCR_MUX(7);
	PORTD->PCR[0] |= PORT_PCR_MUX(1);
	PORTD->PCR[15] &= PORT_PCR_MUX(7);
	PORTD->PCR[15] |= PORT_PCR_MUX(1);
	PTD->PDDR |= 1 | (1 << 15);
}

void WDOG_disable(void)
{
	WDOG->CNT = 0xD928C520;	  /* Unlock watchdog 		*/
	WDOG->TOVAL = 0x0000FFFF; /* Maximum timeout value 	*/
	WDOG->CS = 0x00002100;	  /* Disable watchdog 		*/
}

void LPIT0_init(uint32_t delay)
{
	uint32_t timeout;
	/*!
	 * LPIT Clocking:
	 * ==============================
	 */
	PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(6);	/* Clock Src = 6 (SPLL2_DIV2_CLK)*/
	PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clk to LPIT0 regs       */

	/*!
	 * LPIT Initialization:
	 */
	LPIT0->MCR |= LPIT_MCR_M_CEN_MASK; /* DBG_EN-0: Timer chans stop in Debug mode */
									   /* DOZE_EN=0: Timer chans are stopped in DOZE mode */
									   /* SW_RST=0: SW reset does not reset timer chans, regs */
									   /* M_CEN=1: enable module clk (allows writing other LPIT0 regs) */

	timeout = delay * 40000;
	LPIT0->TMR[0].TVAL = timeout; /* Chan 0 Timeout period: 40M clocks */
	LPIT0->TMR[0].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;
	/* T_EN=1: Timer channel is enabled */
	/* CHAIN=0: channel chaining is disabled */
	/* MODE=0: 32 periodic counter mode */
	/* TSOT=0: Timer decrements immediately based on restart */
	/* TSOI=0: Timer does not stop after timeout */
	/* TROT=0 Timer will not reload on trigger */
	/* TRG_SRC=0: External trigger soruce */
	/* TRG_SEL=0: Timer chan 0 trigger source is selected*/
}

void delay_ms(volatile int ms)
{
	LPIT0_init(ms); /* Initialize PIT0 for 1 second timeout  */
	while (0 == (LPIT0->MSR & LPIT_MSR_TIF0_MASK))
	{
	}								  /* Wait for LPIT0 CH0 Flag */
	lpit0_ch0_flag_counter++;		  /* Increment LPIT0 timeout counter */
	LPIT0->MSR |= LPIT_MSR_TIF0_MASK; /* Clear LPIT0 timer flag 0 */
}

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

int main(void)
{
	WDOG_disable();
	SOSC_init_8MHz();
	SPLL_init_160MHz();
	NormalRUNmode_80MHz();
	PORT_init();
	ADC_init();
	LPUART1_init();

	uint32_t n = 0x6138;
	uint32_t adcResultInMv;
	for (;;)
	{
		convertAdcChan(12);
		while (adc_complete() == 0)
			;
		adcResultInMv = (uint32_t)(((double)read_adc_chx()) * 1024 / 5000);
		if (adcResultInMv > 999) adcResultInMv = 999;
		n &= ~(0x7FE0);
		n |= adcResultInMv << 5;
		if (PTC->PDIR & (1 << 12))
		{
			n |= (1 << 2);
			PTD->PCOR |= 1;
		}
		else
		{
			n &= ~(1 << 2);
			PTD->PSOR |= 1;
		}

		if (PTC->PDIR & (1 << 13))
		{
			n |= (1 << 1);
			PTD->PCOR |= (1 << 15);
		}
		else	
		{
			n &= ~(1 << 1);
			PTD->PSOR |= (1 << 15);
		}
		if (adcResultInMv > 500) PTD->PCOR |= 1;
		LPUART1_transmit_word(n);
		delay_ms(100);
	}
}