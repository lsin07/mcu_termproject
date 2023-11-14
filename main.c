#include "device_registers.h"
#include "clocks_and_modes.h"
#include "ADC.h"
#include "LPUART.h"

/*
	Pin Mapping
	-----------------------------------------
	C13			Potentiometer	(ADCIN)
	C8			Gear Button		(DIN)
	C9, C10		Blinker Button	(DIN)
	D5			UWave Trig		(DOUT)
	D6			UWave Echo		(DIN)
	D7			Buzzer			(DOUT)
	D8 ~ D10	LED				(DOUT)
	D11			DC Motor		(PWMOUT)
*/

void PORT_init(void);
void WDOG_disable(void);
void LPIT0_init(void);
void NVIC_init_IRQs(void);
void LPUART1_data_generator(void);
uint8_t evenParity(uint32_t);

int lpit0_ch0_flag_counter = 0;
int lpit0_ch1_flag_counter = 0;

// global variables for uwave read
int uwave_counter = 0;
int uwave_distance = 0;
int uwave_high = 0;
int uwave_alarm = 0;

int gear = 1;
/* gear mode
0b00: error
0b01: P
0b10: R
0b11: D
*/

int blinker = 0;
/* blinker mode
0b00: off
0b01: right
0b10: left
0b11: both
*/

uint32_t d_send = 0x6138;
uint32_t adcResult;

int main(void)
{
	WDOG_disable();
	SOSC_init_8MHz();
	SPLL_init_160MHz();
	NormalRUNmode_80MHz();
	PORT_init();
	LPIT0_init();
	NVIC_init_IRQs();
	ADC_init();
	LPUART1_init();

	PTD->PSOR = 1 | (1 << 15);
	for (;;)
	{
		convertAdcChan(12);
		while (adc_complete() == 0)
			;
		adcResult = (uint32_t)(((double)read_adc_chx()) * 0.2046); // 0.2046 = 1023 / 5000
	}
}

void PORT_init(void)
{
	PCC->PCCn[PCC_PORTC_INDEX] = PCC_PCCn_CGC_MASK;
	PORTC->PCR[6] &= PORT_PCR_MUX(7);
	PORTC->PCR[6] |= PORT_PCR_MUX(2);
	PORTC->PCR[7] &= PORT_PCR_MUX(7);
	PORTC->PCR[7] |= PORT_PCR_MUX(2);
	PORTC->PCR[8] &= PORT_PCR_MUX(7);
	PORTC->PCR[8] |= PORT_PCR_MUX(1);
	PORTC->PCR[8] |= (10 << 16);
	PORTC->PCR[9] &= PORT_PCR_MUX(7);
	PORTC->PCR[9] |= PORT_PCR_MUX(1);
	PORTC->PCR[9] |= (10 << 16);
	PORTC->PCR[10] &= PORT_PCR_MUX(7);
	PORTC->PCR[10] |= PORT_PCR_MUX(1);
	PORTC->PCR[10] |= (10 << 16);

	// PTC12, PTC13: S32K144 Onboard SWs
	PORTC->PCR[12] &= PORT_PCR_MUX(7);
	PORTC->PCR[12] |= PORT_PCR_MUX(1);
	PORTC->PCR[13] &= PORT_PCR_MUX(7);
	PORTC->PCR[13] |= PORT_PCR_MUX(1);
	PTC->PDDR &= ~((1 << 8) | (1 << 9) | (1 << 10) | (1 << 12) | (1 << 13));

	PCC->PCCn[PCC_PORTD_INDEX] = PCC_PCCn_CGC_MASK;
	PORTD->PCR[0] &= PORT_PCR_MUX(7);
	PORTD->PCR[0] |= PORT_PCR_MUX(1);
	PORTD->PCR[1] &= PORT_PCR_MUX(7);
	PORTD->PCR[1] |= PORT_PCR_MUX(1);
	PORTD->PCR[2] &= PORT_PCR_MUX(7);
	PORTD->PCR[2] |= PORT_PCR_MUX(1);
	PORTD->PCR[3] &= PORT_PCR_MUX(7);
	PORTD->PCR[3] |= PORT_PCR_MUX(1);
	PORTD->PCR[4] &= PORT_PCR_MUX(7);
	PORTD->PCR[4] |= PORT_PCR_MUX(1);
	PORTD->PCR[5] &= PORT_PCR_MUX(7);
	PORTD->PCR[5] |= PORT_PCR_MUX(1);

	PORTD->PCR[6] &= PORT_PCR_MUX(7);
	PORTD->PCR[6] |= PORT_PCR_MUX(1);
	PORTD->PCR[15] &= PORT_PCR_MUX(7);
	PORTD->PCR[15] |= PORT_PCR_MUX(1);
	PTD->PDDR |= 1 | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 15);
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

void LPIT0_Ch0_IRQHandler(void)
{
	/*
	this ISR handles 0.1s sync, while sending data through UART
	*/
	lpit0_ch0_flag_counter++; /* Increment LPIT0 timeout counter */

	// generating LPUART send data
	LPUART1_data_generator();
	LPUART1_transmit_word(d_send);

	if (blinker & (1 << 1)) {
		if (lpit0_ch0_flag_counter % 10 == 0)
			d_send |= (1 << 2);
		else if (lpit0_ch0_flag_counter % 10 == 5)
			d_send &= ~(1 << 2);
	}
	if (blinker & 1) {
		if (lpit0_ch0_flag_counter % 10 == 0)
			d_send |= (1 << 1);
		else if (lpit0_ch0_flag_counter % 10 == 5)
			d_send &= ~(1 << 1);
	}
	
	LPIT0->MSR |= LPIT_MSR_TIF0_MASK; /* Clear LPIT0 timer flag 0 */
}

void LPIT0_Ch1_IRQHandler(void)
{
	/*
	This ISR handles uwave sensor.
	The interrupt signals are made every 10us.
	*/
	lpit0_ch1_flag_counter++;

	if (uwave_counter == 0)
		PTD->PSOR |= (1 << 5);
	if (uwave_counter == 2)
		PTD->PCOR |= (1 << 5);
	if ((PTD->PDIR & (1 << 6)))
		uwave_high++;

	uwave_counter++;
	if (uwave_counter >= 6000)
	{
		uwave_counter = 0;
		uwave_distance = uwave_high;
		if (uwave_distance < 100)
			uwave_alarm = 3;
		else if (uwave_distance < 200)
			uwave_alarm = 2;
		else if (uwave_distance < 300)
			uwave_alarm = 1;
		else
			uwave_alarm = 0;
		uwave_high = 0;
	}
	LPIT0->MSR |= LPIT_MSR_TIF1_MASK;
}

void PORTC_IRQHandler(void)
{
	/* Ths ISR handles panel button input. */

	if ((PORTC->ISFR & (1 << 8)) != 0) // gear
	{
		gear++;
		if (gear > 3) gear = 1;
	}

	if ((PORTC->ISFR & (1 << 9)) != 0) // blinker left
		blinker ^= (1 << 1);

	if ((PORTC->ISFR & (1 << 10)) != 0) // blinker right
		blinker ^= 1;

	PORTC->PCR[8] |= PORT_PCR_ISF_MASK; // Port Control Register ISF bit '1' set
	PORTC->PCR[9] |= PORT_PCR_ISF_MASK; // Port Control Register ISF bit '1' set
	PORTC->PCR[10] |= PORT_PCR_ISF_MASK; // Port Control Register ISF bit '1' set
}

void LPUART1_data_generator(void)
{
	// speed
	// Debug display로 사용 가능!
	d_send &= ~(0x7FE0);	// 원래값: 0x7FE0
	d_send |= adcResult << 5; // 원래값: adcResult

	// gear
	d_send &= ~(0x18);
	d_send |= gear << 3;

	// blinker
	if ((blinker & (1 << 1)) == 0) d_send &= ~(1 << 2);
	if ((blinker & 1) == 0) d_send &= ~(1 << 1);
}