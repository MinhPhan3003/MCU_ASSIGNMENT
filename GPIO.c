#include "GPIO.h"
#include "PCC.h"
#include "PORT.h"

void GPIO_SetOutput(volatile GPIO_typedef *gpio, uint32_t pin)
{
    if ((gpio == NULL) || (pin >= 32u)) {
        return;
    }
    gpio->PDDR |= (1UL << pin);
}

void GPIO_WritePin(volatile GPIO_typedef *gpio, uint32_t pin, uint32_t level)
{
    if ((gpio == NULL) || (pin >= 32u)) {
        return;
    }
    if (level != 0u) {
        gpio->PSOR = (1UL << pin);
    } else {
        gpio->PCOR = (1UL << pin);
    }
}

void GPIO_TogglePin(volatile GPIO_typedef *gpio, uint32_t pin)
{
    if ((gpio == NULL) || (pin >= 32u)) {
        return;
    }
    gpio->PTOR = (1UL << pin);
}

/* ------------------------------------------------------------------------
 *  EVB RGB LED - active low
 * ------------------------------------------------------------------------ */

void LED_Init(void)
{
    CLOCK_EnablePeripheral(PCC_PORTD, PCS_PCC_OFF);

    PORT_SetMux(LED_PORT, LED_BLUE_PIN,  PORT_MUX_GPIO);
    PORT_SetMux(LED_PORT, LED_RED_PIN,   PORT_MUX_GPIO);
    PORT_SetMux(LED_PORT, LED_GREEN_PIN, PORT_MUX_GPIO);

    /* Set the output level (off) BEFORE switching to output: no LED flash */
    LED_Off(LED_BLUE_PIN);
    LED_Off(LED_RED_PIN);
    LED_Off(LED_GREEN_PIN);

    GPIO_SetOutput(LED_GPIO, LED_BLUE_PIN);
    GPIO_SetOutput(LED_GPIO, LED_RED_PIN);
    GPIO_SetOutput(LED_GPIO, LED_GREEN_PIN);
}

void LED_On(uint32_t pin)
{
    GPIO_WritePin(LED_GPIO, pin, 0u);
}

void LED_Off(uint32_t pin)
{
    GPIO_WritePin(LED_GPIO, pin, 1u);
}

void LED_Toggle(uint32_t pin)
{
    GPIO_TogglePin(LED_GPIO, pin);
}
