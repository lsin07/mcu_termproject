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

void blinker_control(void);
void lamp_control(void);
void buzzer_control(void);

int lpit0_ch0_flag_counter = 0;
int lpit0_ch1_flag_counter = 0;

int uwave_counter = 0;
int uwave_distance = 0;
int uwave_high = 0;
int buzzer_counter = 0;

_GEAR_TYPE gear = P;
_LAMP_TYPE lamp_mode = OFF; // which lamp mode is set
_LAMP_TYPE lamp = OFF;      // which lamps to turn on
int lamp_auto_trig_low, lamp_auto_trig_high;
_BLINKER_TYPE blinker_isSet; // Is the blinker set or not
_BLINKER_TYPE blinker;       // Are the blinker lamps on or off
int blinker_counter;
_UART_DATA_TYPE d_send;

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
        // Read VR
        convertAdcChan(13);
        while (adc_complete() == 0)
            ;
        vrResult = read_adc_chx();

        // Read Cds
        if (lamp_mode == AUTO)
        {
            convertAdcChan(14);
            while (adc_complete() == 0)
                ;
            cdsResult = read_adc_chx();
        }

        lamp_control();

        // Generating PWM signal
        if (gear == P)
            FTM0_CH2_PWM_OFF;
        else if (gear == R)
            FTM0_CH2_PWM(4000 + (double)(vrResult) * 0.9768); // 0.9768 = 4000 / 4095
        else if (gear == D)
            FTM0_CH2_PWM(4000 - (double)(vrResult) * 0.9768); // 0.9768 = 4000 / 4095
    }
}

/* This ISR handles 100ms sync which triggers other functions, while sending data through UART */
void LPIT0_Ch0_IRQHandler(void)
{
    lpit0_ch0_flag_counter++;

    // Conducting 100ms synced tasks
    blinker_control();
    if (gear == R)
        buzzer_control();

    // Generating UART data
    d_send.elements.lamp = lamp;
    if (gear == P)
        d_send.elements.speed = 0;
    else
        d_send.elements.speed = cdsResult >> 2; // squashing 12 bit data into 10 bit
    d_send.elements.gear = gear;
    d_send.elements.blinker = blinker.value;
    LPUART1_transmit_word(d_send.value);

    LPIT0->MSR |= LPIT_MSR_TIF0_MASK; /* Clear LPIT0 timer flag 0 */
}

/* This ISR handles uwave sensor.
The interrupt signals are made every 10us. */
void LPIT0_Ch1_IRQHandler(void)
{
    lpit0_ch1_flag_counter++;

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
    LPIT0->MSR |= LPIT_MSR_TIF1_MASK;
}

/* This ISR handles panel button input. */
void PORTC_IRQHandler(void)
{
    if (GEAR_BUTTON_READ != 0) // gear
    {
        if (++gear > 3)
            gear = P;
    }

    if (B_LEFT_BUTTON_READ != 0) // blinker left
        blinker_isSet.elements.left ^= 1;

    if (B_RIGHT_BUTTON_READ != 0) // blinker right
        blinker_isSet.elements.right ^= 1;

    if (LAMP_BUTTON_READ != 0)
    {
        if (++lamp_mode > 3)
            lamp_mode = OFF;
    }

    PORTC->PCR[8] |= PORT_PCR_ISF_MASK;
    PORTC->PCR[9] |= PORT_PCR_ISF_MASK;
    PORTC->PCR[10] |= PORT_PCR_ISF_MASK;
    PORTC->PCR[11] |= PORT_PCR_ISF_MASK;
}

/* This function controls blinker timer.
It is synced with LPIT0_CH0 and will be executed every 100ms.*/
void blinker_control(void)
{
    if (blinker_isSet.value == 0)
    {
        blinker_counter = 0;
        blinker.value = 0;
    }
    else
    {
        if (blinker_counter < 5)
        {
            if (blinker_isSet.elements.left)
                blinker.elements.left = 1;
            if (blinker_isSet.elements.right)
                blinker.elements.right = 1;
        }
        else
            blinker.value = 0;
        if (++blinker_counter > 9)
            blinker_counter = 0;
    }
}

/* This function controls lamps and piezoelectric speaker.
It will be running all the time. */
void lamp_control(void)
{
    // gear
    if (gear == P)
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
    if (lamp_mode == HEAD_LAMP)
        lamp = HEAD_LAMP;
    else if (lamp_mode == POS_LAMP)
        lamp = POS_LAMP;
    else if (lamp_mode == AUTO)
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
    else
        lamp = OFF;

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

/* This function controls buzzer.
It is synced with LPIT0_CH0 and will be executed every 100ms only if the gear is set to reverse.*/
void buzzer_control(void)
{
    if (uwave_distance < 100)
        BUZZER_ON;
    else if (uwave_distance < 200)
        BUZZER_TOGGLE;
    else if (uwave_distance < 300)
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