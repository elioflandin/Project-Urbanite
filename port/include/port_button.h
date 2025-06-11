/**
 * @file port_button.h
 * @brief Header for the portable functions to interact with the HW of the buttons. The functions must be implemented in the platform-specific code.
 * @author alumno1
 * @author alumno2
 * @date fecha
 */

#ifndef PORT_BUTTON_H_
#define PORT_BUTTON_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>

/* Defines and enums ----------------------------------------------------------*/
/* Defines */
// Define here all the button identifiers that are used in the system

#define PORT_PARKING_BUTTON_ID 0                  /*!< ID of the user button (connected to PC13) */
#define PORT_PARKING_BUTTON_DEBOUNCE_TIME_MS 150  /*!< Button debounce time in milliseconds */

/* Function prototypes and explanation -------------------------------------------------*/

/**
 * @brief Initialize the GPIO port and pin connected to the button.
 */
void port_button_gpio_setup(void);

/**
 * @brief Initialize internal data for the button.
 */
void port_button_init(uint32_t button_id);

/**
 * @brief Get the current logical value of the button (true if pressed).
 */
bool port_button_get_value(uint32_t button_id);

/**
 * @brief Set the "pressed" state.
 */
void port_button_set_pressed(uint32_t button_id, bool pressed);

/**
 * @brief Get the "pressed" state (used by the FSM).
 */
bool port_button_get_pressed(uint32_t button_id);

/**
 * @brief Return true if the corresponding interrupt is pending.
 */
bool port_button_get_pending_interrupt(uint32_t button_id);

/**
 * @brief Clear the pending interrupt flag for the button.
 */
void port_button_clear_pending_interrupt(uint32_t button_id);

/**
 * @brief Disable interrupts for the button.
 */
void port_button_disable_interrupts(uint32_t button_id);



#endif