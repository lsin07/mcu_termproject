#include "init.h"

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
