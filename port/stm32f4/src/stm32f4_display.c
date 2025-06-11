/**
 * @file stm32f4_display.c
 * @brief Portable functions to interact with the display system FSM library. All portable functions must be implemented in this file.
 * @author alumno1
 * @author alumno2
 * @date fecha
 */

/* Standard C includes */
#include <stdio.h>

/* HW dependent includes */
#include "port_display.h"
#include "port_system.h"

/* Microcontroller dependent includes */
#include "stm32f4_system.h"
#include "stm32f4_display.h"

/* Defines --------------------------------------------------------------------*/

/* Typedefs --------------------------------------------------------------------*/

/**
 * @brief Structure to define the HW dependencies of an RGB LED.
 */
typedef struct {
    GPIO_TypeDef *p_port_red;   /*!< GPIO where the RED LED is connected */
    uint8_t pin_red;            /*!< Pin where the RED LED is connected */

    GPIO_TypeDef *p_port_green; /*!< GPIO where the GREEN LED is connected */
    uint8_t pin_green;          /*!< Pin where the GREEN LED is connected */

    GPIO_TypeDef *p_port_blue;  /*!< GPIO where the BLUE LED is connected */
    uint8_t pin_blue;           /*!< Pin where the BLUE LED is connected */
} stm32f4_display_hw_t;

/* Global variables */

/**
 * @brief Array of elements that represents the HW characteristics of the RGB LED of the display systems connected to the STM32F4 platform.
 * 
 */
static stm32f4_display_hw_t displays_arr[] = {
    [PORT_REAR_PARKING_DISPLAY_ID] = {
        .p_port_red   = STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO,
        .pin_red      = STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN,
        .p_port_green = STM32F4_REAR_PARKING_DISPLAY_RGB_G_GPIO,
        .pin_green    = STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN,
        .p_port_blue  = STM32F4_REAR_PARKING_DISPLAY_RGB_B_GPIO,
        .pin_blue     = STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN
    }
};

/* Private functions -----------------------------------------------------------*/

/**
 * @brief Get the pointer to the hardware configuration of a given display.
 *
 * @param display_id Display ID.
 * @return Pointer to stm32f4_display_hw_t structure, or NULL if ID is invalid.
 */
static stm32f4_display_hw_t *_stm32f4_display_get(uint32_t display_id)
{
    // Return the pointer to the display with the given ID. If the ID is not valid, return NULL.
    // TO-DO alumnos
    if (display_id >= (sizeof(displays_arr) / sizeof(displays_arr[0]))) {
        return NULL;
    }
    return &displays_arr[display_id];
}

/**
 * @brief Configure the timer that generates PWM signals for the RGB display.
 *
 * This function sets up TIM4 in PWM mode 1 at 50 Hz for channels 1 (Red), 3 (Green), and 4 (Blue).
 * It is only configured for PORT_REAR_PARKING_DISPLAY_ID.
 *
 * @param display_id Display system identifier.
 */
void _timer_pwm_config(uint32_t display_id)
{
    // uint32_t f_timer = SystemCoreClock;
    // printf("SystemCoreClock = %lu\r\n", f_timer);  // Debug -> results : SystemCoreClock = 16.000.000 = 16MHz

    if (display_id != PORT_REAR_PARKING_DISPLAY_ID)
        return;

    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;  // 1. Enable TIM4 clock (on APB1 bus)

    TIM4->CR1 &= ~TIM_CR1_CEN;  // 2. Disable the timer counter (CEN = 0), enable auto-reload preload (ARPE = 1)
    TIM4->CR1 |= TIM_CR1_ARPE;

    TIM4->CNT = 0;  // 3. Reset the counter, and configure ARR and PSC for 50 Hz
    // TIM4->PSC = 1599;  // Prescaler and Auto-Reload for 50 Hz (20ms)
    // TIM4->ARR = 199;
    TIM4->PSC = 4;  // If we want better PWM resolution
    TIM4->ARR = 63999;
    // printf("PSC = %lu, ARR = %lu\r\n", (uint32_t)TIM4->PSC, (uint32_t)TIM4->ARR);  // Debug

    TIM4->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC3E | TIM_CCER_CC4E);  // 4. Disable output compare on channels 1, 3, and 4

    TIM4->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP |   // 5. Clear CCxP and CCxNP of the output compare register (CCER) for each one of the corresponding channels
                    TIM_CCER_CC3P | TIM_CCER_CC3NP |
                    TIM_CCER_CC4P | TIM_CCER_CC4NP);

    // 6. Configure PWM mode 1 and enable preload for channels 1, 3, 4
    // Channel 1 (Red) → CCMR1
    TIM4->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM4->CCMR1 |= (6 << TIM_CCMR1_OC1M_Pos);  // OC1M = 110 → PWM mode 1
    TIM4->CCMR1 |= TIM_CCMR1_OC1PE;           // Preload enable

    // Channel 3 (Green) → CCMR2
    TIM4->CCMR2 &= ~TIM_CCMR2_OC3M;
    TIM4->CCMR2 |= (6 << TIM_CCMR2_OC3M_Pos);  // OC3M = 110 → PWM mode 1
    TIM4->CCMR2 |= TIM_CCMR2_OC3PE;            // Preload enable

    // Channel 4 (Blue) → CCMR2
    TIM4->CCMR2 &= ~TIM_CCMR2_OC4M;
    TIM4->CCMR2 |= (6 << TIM_CCMR2_OC4M_Pos);  // OC4M = 110 → PWM mode 1
    TIM4->CCMR2 |= TIM_CCMR2_OC4PE;            // Preload enable

    TIM4->EGR |= TIM_EGR_UG;  // 7. Generate an update event to load ARR and PSC into shadow registers

    // volatile uint32_t test_cr1 = TIM4->CR1;  // Degub
    // printf("TIM4->CR1 = 0x%08lX\r\n", test_cr1);
    // TIM4->CR1 &= ~TIM_CR1_CEN;  // Explicit disable of CEN
}


/* Public functions -----------------------------------------------------------*/

/**
 * @brief Initialize the hardware configuration of a given RGB LED display.
 *
 * This function configures the GPIOs and alternate functions of the three RGB pins.
 * It also initializes the PWM timer and turns the LED off by default.
 *
 * @param display_id Display system identifier (e.g., PORT_REAR_PARKING_DISPLAY_ID)
 */
void port_display_init(uint32_t display_id)
{
    // 1. Retrieve the hardware configuration struct
    stm32f4_display_hw_t *config = _stm32f4_display_get(display_id);
    if (config == NULL) {
        return; // If invalid display ID
    }

    // 2. Configure the GPIOs as alternate function, no pull-up or pull-down
    const uint8_t MODE_AF = 2;    // GPIO_MODE_AF = 2
    const uint8_t NOPULL = 0;     // GPIO_NOPULL = 0
    stm32f4_system_gpio_config(config->p_port_red, config->pin_red, MODE_AF, NOPULL);
    stm32f4_system_gpio_config(config->p_port_green, config->pin_green, MODE_AF, NOPULL);
    stm32f4_system_gpio_config(config->p_port_blue, config->pin_blue, MODE_AF, NOPULL);

    // 3. Configure the alternate function (AF2 for TIM4 on PB6, PB8, PB9)
    const uint8_t af_tim4 = 2;
    stm32f4_system_gpio_config_alternate(config->p_port_red, config->pin_red, af_tim4);
    stm32f4_system_gpio_config_alternate(config->p_port_green, config->pin_green, af_tim4);
    stm32f4_system_gpio_config_alternate(config->p_port_blue, config->pin_blue, af_tim4);

    // 4. Configure the PWM timer (TIM4)
    _timer_pwm_config(display_id);

    // 5. Set the initial LED color to off
    port_display_set_rgb(display_id, COLOR_OFF);
    // volatile uint32_t cr1_final = TIM4->CR1;  // Debug
    // printf("CR1 after init = 0x%08lX\r\n", cr1_final);
}

/**
 * @brief Set the RGB LED display to the specified color using PWM.
 *
 * This function disables the PWM timer, sets the duty cycle of each LED color
 * based on the rgb_color_t values, and re-enables the timer.
 *
 * @param display_id Display system identifier
 * @param color RGB color to display
 */
void port_display_set_rgb(uint32_t display_id, rgb_color_t color)
{
    // Check that the display ID is supported
    if (display_id != PORT_REAR_PARKING_DISPLAY_ID)
        return;

    TIM4->CR1 &= ~TIM_CR1_CEN;  // Disable the timer before updating values

    // If all components are zero, turn off all channels
    if (color.r == 0 && color.g == 0 && color.b == 0) {
        TIM4->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC3E | TIM_CCER_CC4E); // Disable all channels
    } else {
        // === RED LED (Channel 1) ===
        if (color.r == 0) {
            TIM4->CCER &= ~TIM_CCER_CC1E; // Disable red channel
        } else {
            // Scale r from 0–255 to 0–ARR
            TIM4->CCR1 = (uint32_t)(color.r * TIM4->ARR) / PORT_DISPLAY_RGB_MAX_VALUE;
            TIM4->CCER |= TIM_CCER_CC1E; // Enable red channel
        }

        // === GREEN LED (Channel 3) ===
        if (color.g == 0) {
            TIM4->CCER &= ~TIM_CCER_CC3E; // Disable green channel
        } else {
            TIM4->CCR3 = (uint32_t)(color.g * TIM4->ARR) / PORT_DISPLAY_RGB_MAX_VALUE;
            TIM4->CCER |= TIM_CCER_CC3E; // Enable green channel
        }

        // === BLUE LED (Channel 4) ===
        if (color.b == 0) {
            TIM4->CCER &= ~TIM_CCER_CC4E; // Disable blue channel
        } else {
            TIM4->CCR4 = (uint32_t)(color.b * TIM4->ARR) / PORT_DISPLAY_RGB_MAX_VALUE;
            TIM4->CCER |= TIM_CCER_CC4E; // Enable blue channel
        }
    }

    // Force update of registers and restart the timer, only if an LED is on
    if (color.r != 0 || color.g != 0 || color.b != 0) {
        TIM4->EGR |= TIM_EGR_UG;      // Update generation
        TIM4->CR1 |= TIM_CR1_CEN;     // Re-enable the timer
    }
}
