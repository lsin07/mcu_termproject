#include "device_registers.h"
#include "clocks_and_modes.h"
#include "ADC.h"
#include "LPUART.h"
#include "init.h"

void delay_ms(volatile int ms);
int lpit0_ch0_flag_counter = 0;

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
	uint32_t adcResult;
	for (;;)
	{
		convertAdcChan(12);
		while (adc_complete() == 0)
			;
		adcResult = (uint32_t)(((double)read_adc_chx()) * 0.2046); // 0.2046 = 1023 / 5000
		n &= ~(0x7FE0);
		n |= adcResult << 5;
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
		if (adcResult > 500) PTD->PCOR |= 1;
		LPUART1_transmit_word(n);
		delay_ms(100);
	}
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