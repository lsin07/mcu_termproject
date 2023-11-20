#ifndef TYPES_AND_COMMANDS_H_
#define TYPES_AND_COMMANDS_H_
#include "device_registers.h"

#define GEAR_BUTTON_READ        (PORTC->ISFR & (1 << 8))
#define B_LEFT_BUTTON_READ      (PORTC->ISFR & (1 << 9))
#define B_RIGHT_BUTTON_READ     (PORTC->ISFR & (1 << 10))
#define LAMP_BUTTON_READ        (PORTC->ISFR & (1 << 11))
#define FTM0_CH2_PWM_OFF        FTM0->SC &= ~(FTM_SC_PWMEN4_MASK | FTM_SC_PWMEN5_MASK)
#define BLUE_LED_ON             PTD->PCOR |= 1 | (1 << 0)
#define BLUE_LED_OFF            PTD->PSOR |= 1 | (1 << 0)
#define RED_LED_ON              PTD->PCOR |= 1 | (1 << 15)
#define RED_LED_OFF             PTD->PSOR |= 1 | (1 << 15)
#define GREEN_LED_ON            PTD->PCOR |= 1 | (1 << 16)
#define GREEN_LED_OFF           PTD->PSOR |= 1 | (1 << 16)
#define B_LEFT_ON               PTD->PCOR |= 1 | (1 << 8)
#define B_LEFT_OFF              PTD->PSOR |= 1 | (1 << 8)
#define B_RIGHT_ON              PTD->PCOR |= 1 | (1 << 9)
#define B_RIGHT_OFF             PTD->PSOR |= 1 | (1 << 9)
#define BUZZER_ON               PTD->PSOR |= 1 | (1 << 7)
#define BUZZER_OFF              PTD->PCOR |= 1 | (1 << 7)
#define BUZZER_TOGGLE           PTD->PTOR |= 1 | (1 << 7)
#define PIEZO_ON                PTD->PSOR |= 1 | (1 << 10)
#define PIEZO_OFF               PTD->PCOR |= 1 | (1 << 10)
#define UWAVE_TRIG_SEND         PTD->PSOR |= (1 << 5)
#define UWAVE_TRIG_STOP         PTD->PCOR |= (1 << 5)
#define UWAVE_ECHO_READ         PTD->PDIR & (1 << 6)
#define POS_LAMP_ON             PTD->PCOR |= ((1 << 1) | (1 << 4))
#define POS_LAMP_OFF            PTD->PSOR |= ((1 << 1) | (1 << 4))
#define HEAD_LAMP_ON            PTD->PCOR |= ((1 << 2) | (1 << 3))
#define HEAD_LAMP_OFF           PTD->PSOR |= ((1 << 2) | (1 << 3))

/*
_GEAR_TYPE:
enum type
enum mapped as P = 1, R = 2, D = 3
*/
typedef enum
{
    P = 1,
    R,
    D
} _GEAR_TYPE;

/*
_LAMP_TYPE:
enum type
enum mapped as OFF = 0, POS_LAMP = 1, HEAD_LAMP = 2, AUTO = 3
*/
typedef enum
{
    OFF = 0,
    POS_LAMP,
    HEAD_LAMP,
    AUTO
} _LAMP_TYPE;


/*
_BLINKER_TYPE:
1byte union type
use .value to access to whole value
use .elements.left / right to access individual values
*/
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t right : 1;
        uint8_t left : 1;
    } elements;
} _BLINKER_TYPE;

/*
_UART_DATA_TYPE:
4byte union type
use .value to access to whole value
use .elements.blinker / gear / speed to access individual values
*/
typedef union
{
    uint32_t value;
    struct
    {
        uint32_t lamp : 2;
        uint32_t blinker : 2;
        uint32_t gear : 2;
        uint32_t speed : 10;
    } elements;
} _UART_DATA_TYPE;

#endif