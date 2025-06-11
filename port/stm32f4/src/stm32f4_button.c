/**
 * @file stm32f4_button.c
 * @brief Portable functions to interact with the button FSM library. All portable functions must be implemented in this file.
 * @author alumno1
 * @author alumno2
 * @date fecha
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdio.h>

/* HW dependent includes */
#include "port_button.h" // Used to get general information about the buttons (ID, etc.)
#include "port_system.h" // Used to get the system tick

/* Microcontroller dependent includes */
// TO-DO alumnos: include the necessary files to interact with the GPIOs
#include "stm32f4_system.h"
#include "stm32f4_button.h"

/* Typedefs --------------------------------------------------------------------*/

/**
 * @brief Structure to define the HW dependencies of a button status.
 * 
 */
typedef struct
{
    GPIO_TypeDef *p_port;  /*!< GPIO where the button is connected */
    uint8_t pin;           /*!< Pin/line where the button is connected */
    uint8_t pupd_mode;     /*!< Pull-up/Pull-down mode */
    bool flag_pressed;     /*!< Flag to indicate that the button has been pressed */
} stm32f4_button_hw_t;

/**
 * @brief Static array to hold the hardware configuration of each button in the system.
 * 
 */
static stm32f4_button_hw_t buttons_arr[] = {
    [PORT_PARKING_BUTTON_ID] = {
        .p_port = STM32F4_PARKING_BUTTON_GPIO,   // GPIOC
        .pin = STM32F4_PARKING_BUTTON_PIN,       // 13
        .pupd_mode = STM32F4_GPIO_PUPDR_NOPULL,  // No pull-up/down
        .flag_pressed = false                    // Button not pressed initially
    }
};

/* Global variables ------------------------------------------------------------*/

/* Private functions ----------------------------------------------------------*/
/**
 * @brief Get the button status struct with the given ID.
 *
 * @param button_id Button ID.
 *
 * @return Pointer to the button state struct.
 * @return NULL If the button ID is not valid.
 */
stm32f4_button_hw_t *_stm32f4_button_get(uint32_t button_id)
{
    // Return the pointer to the button with the given ID. If the ID is not valid, return NULL.
    if (button_id < sizeof(buttons_arr) / sizeof(buttons_arr[0]))
    {
        return &buttons_arr[button_id];
    }
    else
    {
        return NULL;
    }
}

/* Public functions -----------------------------------------------------------*/
void port_button_init(uint32_t button_id)
{
    // Retrieve the button struct using the private function and the button ID
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);

    if (p_button == NULL)   // if the button doesn't exist, do nothing
        return;

    stm32f4_system_gpio_config(  // Configure GPIO as input with no pull-up or pull-down resistors
        p_button->p_port,
        p_button->pin,
        STM32F4_GPIO_MODE_IN,
        STM32F4_GPIO_PUPDR_NOPULL
    );

    stm32f4_system_gpio_config_exti(  // Configure external interrupt on both rising and falling edges
        p_button->p_port,
        p_button->pin,
        STM32F4_TRIGGER_RISING_EDGE |
        STM32F4_TRIGGER_FALLING_EDGE |
        STM32F4_TRIGGER_ENABLE_INTERR_REQ
    );

    stm32f4_system_gpio_exti_enable(  // Enable the interrupt line with priority 1 and subpriority 0
        p_button->pin,
        1,  // Priority level
        0   // Subpriority level
    );
}

void stm32f4_button_set_new_gpio(uint32_t button_id, GPIO_TypeDef *p_port, uint8_t pin)
{
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    p_button->p_port = p_port;
    p_button->pin = pin;
}


bool port_button_get_value(uint32_t button_id) {
    stm32f4_button_hw_t* btn = _stm32f4_button_get(button_id);
    return (btn->p_port->IDR & (1 << btn->pin));  // Active low
}

void port_button_set_pressed(uint32_t button_id, bool value) {
    _stm32f4_button_get(button_id)->flag_pressed = value;
}

bool port_button_get_pressed(uint32_t button_id) {
    return _stm32f4_button_get(button_id)->flag_pressed;
}

bool port_button_get_pending_interrupt(uint32_t button_id) {
    stm32f4_button_hw_t* btn = _stm32f4_button_get(button_id);
    return (EXTI->PR & (1 << btn->pin)) != 0;
}

void port_button_clear_pending_interrupt(uint32_t button_id) {
    stm32f4_button_hw_t* btn = _stm32f4_button_get(button_id);
    EXTI->PR |= (1 << btn->pin);  // Clear by writing 1
}

void port_button_disable_interrupts(uint32_t button_id) {
    stm32f4_button_hw_t* btn = _stm32f4_button_get(button_id);
    EXTI->IMR &= ~(1 << btn->pin);
}