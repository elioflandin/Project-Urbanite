/**
 * @file fsm_button.h
 * @brief Header for fsm_button.c file.
 * @author alumno1
 * @author alumno2
 * @date fecha
 */

#ifndef FSM_BUTTON_H_
#define FSM_BUTTON_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>

/* Other includes */
#include "fsm.h"

/* Defines and enums ----------------------------------------------------------*/
/* Enums */
/**
 * @brief Enumeration of the button FSM states.
 */
typedef enum {
    BUTTON_RELEASED,        /*!< Starting state. Also comes here when the button has been released */
    BUTTON_PRESSED_WAIT,    /*!< State to perform the anti-debounce mechanism for a falling edge */
    BUTTON_PRESSED,         /*!< State while the button is being pressed */
    BUTTON_RELEASED_WAIT    /*!< State to perform the anti-debounce mechanism for a rising edge */
} FSM_BUTTON;

/* Typedefs --------------------------------------------------------------------*/
typedef struct fsm_button_t fsm_button_t;  /*!< Structure to define the Button FSM */


/* Function prototypes and explanation -------------------------------------------------*/

/**
 * @brief Check whether the FSM is active (not in BUTTON_RELEASED state).
 * 
 * @param p_fsm Pointer to the FSM instance.
 * @return true if active, false otherwise.
 */
bool fsm_button_check_activity(fsm_button_t *p_fsm);

/**
 * @brief Destroy a button FSM and free its memory.
 * 
 * @param p_fsm Pointer to the FSM instance.
 */
void fsm_button_destroy(fsm_button_t *p_fsm);

/**
 * @brief Fire the button FSM to evaluate transitions and execute actions.
 * 
 * @param p_fsm Pointer to the FSM instance.
 */
void fsm_button_fire(fsm_button_t *p_fsm);

/**
 * @brief Get the debounce time configured for this button FSM.
 * 
 * @param p_fsm Pointer to the FSM instance.
 * @return uint32_t Debounce time in milliseconds.
 */
uint32_t fsm_button_get_debounce_time_ms(fsm_button_t *p_fsm);

/**
 * @brief Get the duration of the last valid button press.
 * 
 * @param p_fsm Pointer to the FSM instance.
 * @return uint32_t Duration in milliseconds.
 */
uint32_t fsm_button_get_duration(fsm_button_t *p_fsm);

/**
 * @brief Get a pointer to the internal FSM structure.
 * 
 * @param p_fsm Pointer to the FSM instance.
 * @return fsm_t* Pointer to the inner FSM.
 */
fsm_t* fsm_button_get_inner_fsm(fsm_button_t *p_fsm);

/**
 * @brief Get the current state of the FSM.
 * 
 * @param p_fsm Pointer to the FSM instance.
 * @return uint32_t Current state (from FSM_BUTTON enum).
 */
uint32_t fsm_button_get_state(fsm_button_t *p_fsm);

/**
 * @brief Create a new button FSM.
 * 
 * @param debounce_time_ms Debounce time in milliseconds.
 * @param button_id Unique ID of the button.
 * @return fsm_button_t* Pointer to the created FSM.
 */
fsm_button_t* fsm_button_new(uint32_t debounce_time_ms, uint32_t button_id);

/**
 * @brief Reset the duration flag to 0.
 * 
 * @param p_fsm Pointer to the FSM instance.
 */
void fsm_button_reset_duration(fsm_button_t *p_fsm);

#endif