/**
 * @file fsm_display.h
 * @brief Header for fsm_display.c file.
 * @author alumno1
 * @author alumno2
 * @date fecha
 */

#ifndef FSM_DISPLAY_SYSTEM_H_
#define FSM_DISPLAY_SYSTEM_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>
#include "fsm.h"

/* Defines and enums ----------------------------------------------------------*/

#define DANGER_MIN_CM         0    /*!< Minimum distance in cm to show the DANGER status. */
#define DANGER_MAX_CM         25   /*!< Maximum distance in cm to show the DANGER status. Equal to WARNING_MIN_CM */
#define WARNING_MIN_CM        25   /*!< Minimum distance in cm to show the WARNING status. Equal to DANGER_MAX_CM */
#define WARNING_MAX_CM        50   /*!< Maximum distance in cm to show the WARNING status. Equal to NO_PROBLEM_MIN_CM */
#define NO_PROBLEM_MIN_CM     50   /*!< Minimum distance in cm to show the NO_PROBLEM status. Equal to WARNING_MAX_CM */
#define NO_PROBLEM_MAX_CM     150  /*!< Maximum distance in cm to show the NO_PROBLEM status. Equal to INFO_MIN_CM */
#define INFO_MIN_CM           150  /*!< Minimum distance in cm to show the INFO status. Equal to NO_PROBLEM_MAX_CM */
#define INFO_MAX_CM           175  /*!< Maximum distance in cm to show the INFO status. Equal to OK_MIN_CM */
#define OK_MIN_CM             175  /*!< Minimum distance in cm to show the OK status. Equal to INFO_MAX_CM */
#define OK_MAX_CM             200  /*!< Maximum distance in cm to show the OK status. Beyond this value, the display should be turned off. */

/* Enums */

/**
 * @brief Enumerated states for the Display FSM.
 */
typedef enum {
    WAIT_DISPLAY = 0,  // Initial state. Waits for the system to activate the display and provide a distance.
    SET_DISPLAY        // Active state. The FSM sets the RGB color based on the distance and remains idle until it changes.
} FSM_DISPLAY_SYSTEM;

/* Typedefs --------------------------------------------------------------------*/

/**
 * @brief Opaque structure of the display FSM.
 *
 * Internal fields are defined in the source file (.c).
 */
typedef struct fsm_display_t fsm_display_t;

/* Function prototypes and explanation -------------------------------------------------*/

/**
 * @brief Create a new display FSM.
 *
 * @param display_id Unique display ID.
 * @return fsm_display_t* Pointer to the newly created display FSM.
 */
fsm_display_t* fsm_display_new(uint32_t display_id);

/**
 * @brief Destroy the display FSM and free the memory.
 *
 * @param p_fsm Pointer to the display FSM.
 */
void fsm_display_destroy(fsm_display_t *p_fsm);

/**
 * @brief Set the distance (in cm) for the display FSM to represent.
 *
 * @param p_fsm Pointer to the display FSM.
 * @param distance_cm Distance in centimeters to represent.
 */
void fsm_display_set_distance(fsm_display_t *p_fsm, uint32_t distance_cm);

/**
 * @brief Get the current distance set in the display FSM.
 *
 * @param p_fsm Pointer to the display FSM.
 * @return Distance in centimeters.
 */
int32_t fsm_display_get_distance(fsm_display_t *p_fsm);

/**
 * @brief Set the current state of the display FSM.
 *
 * @param p_fsm Pointer to the display FSM.
 * @param state New state of the FSM.
 */
void fsm_display_set_state(fsm_display_t *p_fsm, int8_t state);

/**
 * @brief Get the current state of the display FSM.
 *
 * @param p_fsm Pointer to the display FSM.
 * @return uint32_t Current FSM state.
 */
uint32_t fsm_display_get_state(fsm_display_t *p_fsm);

/**
 * @brief Fire the FSM of the display system.
 *
 * This function triggers the FSM transitions and executes the corresponding actions.
 *
 * @param p_fsm Pointer to the display FSM.
 */
void fsm_display_fire(fsm_display_t *p_fsm);

/**
 * @brief Get a pointer to the internal FSM structure of the display FSM.
 *
 * @param p_fsm Pointer to the display FSM.
 * @return fsm_t* Pointer to the inner fsm_t structure.
 */
fsm_t* fsm_display_get_inner_fsm(fsm_display_t *p_fsm);

/**
 * @brief Set the status (active or paused) of the display system.
 *
 * @param p_fsm Pointer to the display FSM.
 * @param pause true if the system should be paused, false if active.
 */
void fsm_display_set_status(fsm_display_t *p_fsm, bool pause);

/**
 * @brief Get the current status (active or paused) of the display system.
 *
 * @param p_fsm Pointer to the display FSM.
 * @return true if the display system is active, false otherwise (pause).
 */
bool fsm_display_get_status(fsm_display_t *p_fsm);

/**
 * @brief Check if the display is active and not idle.
 *
 * @param p_fsm Pointer to the display FSM.
 * @return true if active and not idle, false otherwise.
 */
bool fsm_display_check_activity(fsm_display_t *p_fsm);

#endif /* FSM_DISPLAY_SYSTEM_H_ */