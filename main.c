#include "device_registers.h"
#include "clocks_and_modes.h"
#include "ADC.h"
#include "PWM.h"
#include "LPUART.h"
#include "init.h"
#include "types_and_commands.h"

/*
    Pin         Name            Type                Connect With
    ───────────────────────────────────────────────────────────────
    B4, B5      DC Motor        PWMOUT FTM0CH4/5    DC_IN1, DC_IN2
    C8          Gear Button     DIN                 SW3
    C9, C10     Blinker Button  DIN                 SW5, SW1
    C11         Lamp Button     DIN                 SW4
    C15         Potentiometer   ADCIN ADC0CH13      VR
    C16         Cds             ADCIN ADC0CH14      CDS
    D1 ~ 4      Lights          DOUT                LED3 ~ 6
    D5          UWave Trig      DOUT                uWAVE TRIG
    D6          UWave Echo      DIN                 uWAVE ECHO
    D7          Buzzer          DOUT                BUZZER
    D8, D9      Blinker Lamp    DOUT                LED8, LED1
    D10         Piezo           DOUT                PIEZO
*/

void SWM_Blinker(void);
void SWM_Lamp(void);
void SWM_Alarm(void);
void SWM_Sensor(void);
void SWM_UART(void);
void SWM_Throttle(void);
void SWM_Panel(void);
void SWM_ProxWarning(void);

int lpit0_ch0_flag_counter = 0;
int lpit0_ch1_flag_counter = 0;

int uwave_counter = 0;
int uwave_distance = 0;
int uwave_high = 0;
int buzzer_counter = 0;
int proximity_warning = 0;
int proximity_counter = 0;

GEAR_TYPE gear = P;
LAMP_TYPE lamp_mode = OFF; // which lamp mode is set
LAMP_TYPE lamp = OFF;      // which lamps to turn on
int lamp_auto_trig_low = 0;
int lamp_auto_trig_high = 1;
BLINKER_TYPE blinker_mode; // Is the blinker set or not
BLINKER_TYPE blinker;      // Are the blinker lamps on or off
int blinker_counter;
UART_DATA_TYPE d_send;

uint32_t vrResult, cdsResult;

int main(void)
{
    WDOG_disable();
    SOSC_init_8MHz();
    SPLL_init_160MHz();
    NormalRUNmode_80MHz();
    PORT_init();
    FTM_init();
    LPIT0_init();
    NVIC_init_IRQs();
    ADC_init();
    LPUART1_init();

    RED_LED_OFF;
    GREEN_LED_OFF;

    for (;;)
    {
        // Conducting continuous SWMs
        SWM_Lamp();
        SWM_Throttle();
    }
}

void LPIT0_Ch0_IRQHandler(void)
{
    lpit0_ch0_flag_counter++;

    // Conducting 100ms synced SWMs
    SWM_Blinker();
    SWM_Alarm();
    SWM_UART();
    SWM_ProxWarning();
    LPIT0->MSR |= LPIT_MSR_TIF0_MASK; /* Clear LPIT0 timer flag 0 */
}

void LPIT0_Ch1_IRQHandler(void)
{
    lpit0_ch1_flag_counter++;

    // Conducting 10us synced SWMs
    SWM_Sensor();
    LPIT0->MSR |= LPIT_MSR_TIF1_MASK;
}

/* This ISR handles panel button input. */
void PORTC_IRQHandler(void)
{
    // Conducting GPIO input interrupt connected SWMs
    SWM_Panel();

    PORTC->PCR[8] |= PORT_PCR_ISF_MASK;
    PORTC->PCR[9] |= PORT_PCR_ISF_MASK;
    PORTC->PCR[10] |= PORT_PCR_ISF_MASK;
    PORTC->PCR[11] |= PORT_PCR_ISF_MASK;
}

void SWM_Blinker(void)
{
    if (blinker_mode.value == 0)
    {
        blinker_counter = 0;
        blinker.value = 0;
    }
    else
    {
        if (blinker_counter < 5)
        {
            if (blinker_mode.elements.left)
                blinker.elements.left = 1;
            if (blinker_mode.elements.right)
                blinker.elements.right = 1;
        }
        else
            blinker.value = 0;
        if (++blinker_counter > 9)
            blinker_counter = 0;
    }
}

void SWM_Lamp(void)
{
    // Read Cds
    if (lamp_mode == AUTO)
    {
        convertAdcChan(14);
        while (adc_complete() == 0)
            ;
        cdsResult = read_adc_chx();
    }

    // gear
    if (gear == P || proximity_warning > 1)
    {
        RED_LED_ON;
        GREEN_LED_OFF;
    }
    else if (gear == R)
    {
        RED_LED_ON;
        GREEN_LED_ON;
    }
    else
    {
        RED_LED_OFF;
        GREEN_LED_OFF;
    }

    // lamp_mode
    switch (lamp_mode)
    {
    case HEAD_LAMP:
        lamp = HEAD_LAMP;
        break;
    case POS_LAMP:
        lamp = POS_LAMP;
        break;
    case AUTO:
        /*  Conducts software Schmitt trigger debouncing
        Centerd at: 1500, 2500
        Offset: 300 */
        if (lamp_auto_trig_high)
        {
            lamp = OFF;
            if (cdsResult < 2200) // 2500 - 300
                lamp_auto_trig_high = 0;
            if (cdsResult < 1200) // 1500 - 300
                lamp_auto_trig_low = 0;
        }
        else if (lamp_auto_trig_low)
        {
            lamp = POS_LAMP;
            if (cdsResult > 2800) // 2500 + 300
                lamp_auto_trig_high = 1;
            if (cdsResult < 1200)
                lamp_auto_trig_low = 0;
        }
        else
        {
            lamp = HEAD_LAMP;
            if (cdsResult > 2800)
                lamp_auto_trig_high = 1;
            if (cdsResult > 1800) // 1500 + 300
                lamp_auto_trig_low = 1;
        }
        break;
    default:
        lamp = OFF;
    }

    // lamp
    switch (lamp)
    {
    case POS_LAMP:
        HEAD_LAMP_OFF;
        POS_LAMP_ON;
        break;
    case HEAD_LAMP:
        HEAD_LAMP_ON;
        POS_LAMP_ON;
        break;
    default:
        HEAD_LAMP_OFF;
        POS_LAMP_OFF;
        break;
    }

    // blinker
    if (blinker.elements.left)
        B_LEFT_ON;
    else
        B_LEFT_OFF;
    if (blinker.elements.right)
        B_RIGHT_ON;
    else
        B_RIGHT_OFF;
    if (blinker.value != 0)
        PIEZO_ON;
    else
        PIEZO_OFF;
}

void SWM_Alarm(void)
{
    if (gear == R)
    {
        if (uwave_distance < 80)
            BUZZER_ON;
        else if (uwave_distance < 160)
            BUZZER_TOGGLE;
        else if (uwave_distance < 240)
        {
            if (++buzzer_counter > 2)
            {
                BUZZER_TOGGLE;
                buzzer_counter = 0;
            }
        }
        else
            BUZZER_OFF;
    }
    else if (proximity_warning > 0)
    {
        if (buzzer_counter > 0)
        {
            BUZZER_TOGGLE;
            buzzer_counter--;
        }
        else
            BUZZER_OFF;
    }
    else
    {
        buzzer_counter = 0;
        BUZZER_OFF;
    }
}

void SWM_UART(void)
{
    d_send.elements.lamp = lamp;
    if (gear == P || proximity_warning == 2)
        d_send.elements.speed = 0;
    else
        d_send.elements.speed = vrResult >> 2; // squashing 12 bit data into 10 bit
    d_send.elements.gear = gear;
    d_send.elements.blinker = blinker.value;
    d_send.elements.prox = proximity_warning;
    LPUART1_transmit_word(d_send.value);
}

void SWM_Sensor(void)
{
    if (gear == R || gear == D)
    {
        if (uwave_counter == 0)
            UWAVE_TRIG_SEND;
        else if (uwave_counter == 2)
            UWAVE_TRIG_STOP;
        if (UWAVE_ECHO_READ)
            uwave_high++;

        uwave_counter++;
        if (uwave_counter >= 6000) // 10us * 6000 = 60ms
        {
            uwave_counter = 0;
            uwave_distance = uwave_high;
            uwave_high = 0;
        }
    }
}

void SWM_Throttle(void)
{
    // Read VR
    convertAdcChan(13);
    while (adc_complete() == 0)
        ;
    vrResult = read_adc_chx();

    // Generating PWM signal
    if (gear == P)
        FTM0_CH2_PWM_OFF;
    else if (gear == R)
        FTM0_CH2_PWM(4000 + (double)(vrResult) * 0.9768); // 0.9768 = 4000 / 4095
    else if (gear == D)
    {
        if (proximity_warning > 1)
            FTM0_CH2_PWM(4000);
        else
            FTM0_CH2_PWM(4000 - (double)(vrResult) * 0.9768); // 0.9768 = 4000 / 4095
    }
}

void SWM_Panel(void)
{
    if (GEAR_BUTTON_READ != 0) // gear
    {
        if (++gear > 3)
            gear = P;
    }

    if (B_LEFT_BUTTON_READ != 0) // blinker left
        blinker_mode.elements.left ^= 1;

    if (B_RIGHT_BUTTON_READ != 0) // blinker right
        blinker_mode.elements.right ^= 1;

    if (LAMP_BUTTON_READ != 0)
    {
        if (++lamp_mode > 3)
            lamp_mode = OFF;
    }
}

void SWM_ProxWarning(void)
{
    if (gear == D)
    {
        if (uwave_distance < 80)
        {
            if (proximity_counter > 5)
            {
                proximity_counter = 0;
                blinker_mode.value = 3;
                if (proximity_warning < 2)
                    buzzer_counter = 8;
                proximity_warning = 2;
            }
            else
                proximity_counter++;
        }
        else if (uwave_distance < 160 && proximity_warning < 2)
        {
            if (proximity_warning == 0)
                buzzer_counter = 4;
            proximity_warning = 1;
            proximity_counter = 0;
        }
        else if (proximity_warning < 2)
        {
            proximity_warning = 0;
            proximity_counter = 0;
        }
    }
    else
        proximity_warning = 0;
}