#include "PWM.h"

void FTM_init(void)
{

	// FTM0 clocking
	PCC->PCCn[PCC_FTM0_INDEX] &= ~PCC_PCCn_CGC_MASK;  // Ensure clk diabled for config
	PCC->PCCn[PCC_FTM0_INDEX] |= PCC_PCCn_PCS(0b010)  // Clocksrc=1, 8MHz SIRCDIV1_CLK
								 | PCC_PCCn_CGC_MASK; // Enable clock for FTM regs

	// FTM0 Initialization
	FTM0->SC = FTM_SC_PWMEN4_MASK | FTM_SC_PWMEN5_MASK // Enable PWM channel 1output
			   | FTM_SC_PS(0);						   // TOIE(timer overflow Interrupt Ena) = 0 (deafault)
													   // CPWMS(Center aligned PWM Select) =0 (default, up count)
	/* CLKS (Clock source) = 0 (default, no clock; FTM disabled) 	*/
	/* PS (Prescaler factor) = 1. Prescaler = 2 					*/

	FTM0->MOD = 8000 - 1; // FTM0 counter final value (used for PWM mode)
						  //  FTM0 Period = MOD-CNTIN+0x0001~=16000 ctr clks=8ms
						  // 8Mhz /2 =4MHz
	FTM0->CNTIN = FTM_CNTIN_INIT(0);

	FTM0->CONTROLS[4].CnSC |= FTM_CnSC_MSB_MASK;
	FTM0->CONTROLS[5].CnSC |= FTM_CnSC_MSB_MASK;
	FTM0->CONTROLS[4].CnSC |= FTM_CnSC_ELSA_MASK;
	FTM0->CONTROLS[5].CnSC |= FTM_CnSC_ELSA_MASK;
	FTM0->COMBINE |= FTM_COMBINE_SYNCEN2_MASK | FTM_COMBINE_COMP2_MASK | FTM_COMBINE_DTEN2_MASK;
}

void FTM0_CH2_PWM(uint32_t i)
{
	FTM0->SC = FTM_SC_PWMEN4_MASK | FTM_SC_PWMEN5_MASK;
	FTM0->CONTROLS[4].CnV = i; // 8000~0 duty; ex(7200=> Duty 0.1 / 800=>Duty 0.9)
	FTM0->CONTROLS[5].CnV = i; // 8000~0 duty; ex(7200=> Duty 0.1 / 800=>Duty 0.9)
	// start FTM0 counter with clk source = external clock (SOSCDIV1_CLK)
	FTM0->SC |= FTM_SC_CLKS(3);
}