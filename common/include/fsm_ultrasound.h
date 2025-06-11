/**
 * @file fsm_ultrasound.h
 * @brief Header for fsm_ultrasound.c file.
 * @author alumno1
 * @author alumno2
 * @date fecha
 */

#ifndef FSM_ULTRASOUND_H_
#define FSM_ULTRASOUND_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>
#include "fsm.h"

/* Defines and enums ----------------------------------------------------------*/
#define FSM_ULTRASOUND_NUM_MEASUREMENTS 5  /*!< Number of measurements to store in the array */

/**
 * @brief Enumerator for the ultrasound finite state machine
 *
 */
typedef enum {
    WAIT_START = 0,   /*!< Starting state. Also used when a distance measurement has completed or a timeout occurred */
    TRIGGER_START,    /*!< State to send the trigger pulse to the ultrasound sensor */
    WAIT_ECHO_START,  /*!< State to wait for the start of the echo signal */
    WAIT_ECHO_END,    /*!< State to wait for the end of the echo signal */
    SET_DISTANCE      /*!< State to compute and store the measured distance */
} FSM_ULTRASOUND;


/* Typedefs --------------------------------------------------------------------*/
typedef struct fsm_ultrasound_t fsm_ultrasound_t;  /*!< Structure to define the ultrasound FSM */


/* Function prototypes and explanation -------------------------------------------------*/

/**
 * @brief Set the state of the ultrasound FSM.
 *
 * This function sets the current state of the ultrasound FSM.
 *
 * > &nbsp;&nbsp;&nbsp;&nbsp;💡 This function is important because the struct is private and external functions such as those of the unit tests cannot access the state of the FSM directly. \n
 * 
 * @param p_fsm Pointer to an `fsm_ultrasound_t` struct.
 * @param state New state of the ultrasound FSM.
 */
void fsm_ultrasound_set_state(fsm_ultrasound_t *p_fsm, int8_t state);

/**
 * @brief Create a new ultrasound FSM.
 * 
 * @param ultrasound_id Identifier of the ultrasound sensor
 * @return Pointer to the ultrasound FSM.
 */
fsm_ultrasound_t* fsm_ultrasound_new(uint32_t ultrasound_id);

/**
 * @brief Destroy an ultrasound FSM.
 * 
 * @param p_fsm Pointer to an fsm_ultrasound_t struct.
 */
void fsm_ultrasound_destroy(fsm_ultrasound_t *p_fsm);

/**
 * @brief Start the ultrasound sensor.
 * 
 * This function resets all counters and sets the ultrasound FSM as active.
 * 
 * @param p_fsm Pointer to an fsm_ultrasound_t struct.
 */
void fsm_ultrasound_start(fsm_ultrasound_t *p_fsm);

/**
 * @brief Stop the ultrasound sensor.
 * 
 * This function resets the timers and sets the FSM as inactive.
 * 
 * @param p_fsm Pointer to an fsm_ultrasound_t struct.
 */
void fsm_ultrasound_stop(fsm_ultrasound_t *p_fsm);

/**
 * @brief Fire the ultrasound FSM.
 * 
 * This function triggers the transition evaluation and associated actions.
 * 
 * @param p_fsm Pointer to an fsm_ultrasound_t struct.
 */
void fsm_ultrasound_fire(fsm_ultrasound_t *p_fsm);

/**
 * @brief Check if a new measurement is ready.
 * 
 * @param p_fsm Pointer to the ultrasound FSM.
 * @return true if a new measurement is available, false otherwise.
 */
bool fsm_ultrasound_get_new_measurement_ready(fsm_ultrasound_t *p_fsm);

/**
 * @brief Return the distance of the last object detected.
 * 
 * This function resets the internal flag after returning the distance.
 * 
 * @param p_fsm Pointer to an fsm_ultrasound_t struct.
 * @return Distance in centimeters.
 */
uint32_t fsm_ultrasound_get_distance(fsm_ultrasound_t *p_fsm);

/**
 * @brief Get the ready flag of the trigger signal from the HW.
 * 
 * @param p_fsm Pointer to an fsm_ultrasound_t struct.
 * @return true if ready, false otherwise.
 */
bool fsm_ultrasound_get_ready(fsm_ultrasound_t *p_fsm);

/**
 * @brief Get the FSM internal status.
 * 
 * @param p_fsm Pointer to an fsm_ultrasound_t struct.
 * @return true if active, false if paused.
 */
bool fsm_ultrasound_get_status(fsm_ultrasound_t *p_fsm);

/**
 * @brief Set the FSM internal status.
 * 
 * @param p_fsm Pointer to an fsm_ultrasound_t struct.
 * @param status New status to set.
 */
void fsm_ultrasound_set_status(fsm_ultrasound_t *p_fsm, bool status);

/**
 * @brief Get the current state of the FSM.
 * 
 * @param p_fsm Pointer to an fsm_ultrasound_t struct.
 * @return Current state of the FSM.
 */
uint32_t fsm_ultrasound_get_state(fsm_ultrasound_t *p_fsm);

/**
 * @brief Get the inner FSM.
 * 
 * @param p_fsm Pointer to an fsm_ultrasound_t struct.
 * @return Pointer to the inner FSM.
 */
fsm_t* fsm_ultrasound_get_inner_fsm(fsm_ultrasound_t *p_fsm);

/**
 * @brief Check if the FSM is doing a distnace measurement.
 * 
 * @param p_fsm Pointer to an fsm_ultrasound_t struct.
 * @return Always false.
 */
bool fsm_ultrasound_check_activity(fsm_ultrasound_t *p_fsm);


#endif /* FSM_ULTRASOUND_H_ */