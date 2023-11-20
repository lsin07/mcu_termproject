#include "init.h"

void PORT_init(void)
{
	/*=============== PORT B ===============*/
	PCC->PCCn[PCC_PORTB_INDEX] = PCC_PCCn_CGC_MASK;

	/*	PTB4: PWM output for DC motor
		Connect With: DC_IN1 */
	PORTB->PCR[4] |= PORT_PCR_MUX(2);

	/*	PTB5: PWM complimentary output
		Connect With: DC_IN2 */
	PORTB->PCR[5] |= PORT_PCR_MUX(2);

	/*=============== PORT C ===============*/
	PCC->PCCn[PCC_PORTC_INDEX] = PCC_PCCn_CGC_MASK;

	/* PTC7: UART TX */
	PORTC->PCR[7] &= PORT_PCR_MUX(7);
	PORTC->PCR[7] |= PORT_PCR_MUX(2);

	/*	PTC8: Gear SW
		Connect With: SW3 */
	PORTC->PCR[8] &= PORT_PCR_MUX(7);
	PORTC->PCR[8] |= PORT_PCR_MUX(1);
	PORTC->PCR[8] |= (10 << 16);

	/*	PTC9: Blinker Left SW
		Connect With: SW5 */
	PORTC->PCR[9] &= PORT_PCR_MUX(7);
	PORTC->PCR[9] |= PORT_PCR_MUX(1);
	PORTC->PCR[9] |= (10 << 16);

	/*	PTC10: Blinker Right SW
		Connect With: SW1 */
	PORTC->PCR[10] &= PORT_PCR_MUX(7);
	PORTC->PCR[10] |= PORT_PCR_MUX(1);
	PORTC->PCR[10] |= (10 << 16);

	/*	PTC11: Lamp SW
		Connect With: SW4 */
	PORTC->PCR[11] &= PORT_PCR_MUX(7);
	PORTC->PCR[11] |= PORT_PCR_MUX(1);
	PORTC->PCR[11] |= (10 << 16);

	/* PTC12, PTC13: S32K144 Onboard SWs for debug purposes */
	PORTC->PCR[12] &= PORT_PCR_MUX(7);
	PORTC->PCR[12] |= PORT_PCR_MUX(1);
	PORTC->PCR[13] &= PORT_PCR_MUX(7);
	PORTC->PCR[13] |= PORT_PCR_MUX(1);
	PTC->PDDR &= ~((1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) | (1 << 12) | (1 << 13));

	/*	PTC15: Potentiometer input (ADC_CH13)
		Connect with: VR */

	/*=============== PORT D ===============*/
	PCC->PCCn[PCC_PORTD_INDEX] = PCC_PCCn_CGC_MASK;

	/* PTD0: Onboard blue LED */
	// PORTD->PCR[0] &= PORT_PCR_MUX(7);
	// PORTD->PCR[0] |= PORT_PCR_MUX(1);

	/*	PTD1 ~ 4: Lamp
		Connect with: LED3 ~ 6*/
	PORTD->PCR[1] &= PORT_PCR_MUX(7);
	PORTD->PCR[1] |= PORT_PCR_MUX(1);
	PORTD->PCR[2] &= PORT_PCR_MUX(7);
	PORTD->PCR[2] |= PORT_PCR_MUX(1);
	PORTD->PCR[3] &= PORT_PCR_MUX(7);
	PORTD->PCR[3] |= PORT_PCR_MUX(1);
	PORTD->PCR[4] &= PORT_PCR_MUX(7);
	PORTD->PCR[4] |= PORT_PCR_MUX(1);

	/*	PTD5: uwave TRIG
		Connect with: uWAVE TRIG.*/
	PORTD->PCR[5] &= PORT_PCR_MUX(7);
	PORTD->PCR[5] |= PORT_PCR_MUX(1);

	/*	PTD6: uwave ECHO
		Connect with: uWAVE ECHO */
	PORTD->PCR[6] &= PORT_PCR_MUX(7);
	PORTD->PCR[6] |= PORT_PCR_MUX(1);

	/*	PTD7: buzzer
		Connect with: BUZZER */
	PORTD->PCR[7] &= PORT_PCR_MUX(7);
	PORTD->PCR[7] |= PORT_PCR_MUX(1);

	/*	PTD8: Left blinker lamp
		Connect with: LED8 */
	PORTD->PCR[8] &= PORT_PCR_MUX(7);
	PORTD->PCR[8] |= PORT_PCR_MUX(1);

	/*	PTD9: Right blinker lamp
		Connect with: LED1 */
	PORTD->PCR[9] &= PORT_PCR_MUX(7);
	PORTD->PCR[9] |= PORT_PCR_MUX(1);

	/*	PTD10: Piezo
		Connect with: PIEZO */
	PORTD->PCR[10] &= PORT_PCR_MUX(7);
	PORTD->PCR[10] |= PORT_PCR_MUX(1);

	/* PTD15, 16: Onboard red, green LED */
	PORTD->PCR[15] &= PORT_PCR_MUX(7);
	PORTD->PCR[15] |= PORT_PCR_MUX(1);
	PORTD->PCR[16] &= PORT_PCR_MUX(7);
	PORTD->PCR[16] |= PORT_PCR_MUX(1);
	PTD->PDDR |= (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 15) | (1 << 16);
	PTD->PDDR &= ~((1 << 6));
}

void WDOG_disable(void)
{
	WDOG->CNT = 0xD928C520;	  /* Unlock watchdog 		*/
	WDOG->TOVAL = 0x0000FFFF; /* Maximum timeout value 	*/
	WDOG->CS = 0x00002100;	  /* Disable watchdog 		*/
}

void LPIT0_init(void)
{
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
	LPIT0->MIER = 0x03;
	LPIT0->TMR[0].TVAL = 4000000; /* Chan 0 Timeout period: 40M clocks */
	LPIT0->TMR[0].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;
	LPIT0->TMR[1].TVAL = 400; /* Chan 0 Timeout period: 40M clocks */
	LPIT0->TMR[1].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;
	/* T_EN=1: Timer channel is enabled */
	/* CHAIN=0: channel chaining is disabled */
	/* MODE=0: 32 periodic counter mode */
	/* TSOT=0: Timer decrements immediately based on restart */
	/* TSOI=0: Timer does not stop after timeout */
	/* TROT=0 Timer will not reload on trigger */
	/* TRG_SRC=0: External trigger soruce */
	/* TRG_SEL=0: Timer chan 0 trigger source is selected*/
}

void NVIC_init_IRQs(void)
{
	/*LPIT ch0 overflow set*/
	S32_NVIC->ICPR[1] = 1 << (48 % 32);
	S32_NVIC->ISER[1] = 1 << (48 % 32);
	S32_NVIC->IP[48] = 0x0A;

	S32_NVIC->ICPR[1] = 1 << (49 % 32);
	S32_NVIC->ISER[1] = 1 << (49 % 32);
	S32_NVIC->IP[49] = 0x0B;

	S32_NVIC->ICPR[1] |= 1 << (61 % 32);
	S32_NVIC->ISER[1] |= 1 << (61 % 32);
	S32_NVIC->IP[61] = 0x10;
}