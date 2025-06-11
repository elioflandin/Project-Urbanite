/**
 * @file fsm_ultrasound.c
 * @brief Ultrasound sensor FSM main file.
 * @author alumno1
 * @author alumno2
 * @date fecha
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* HW dependent includes */
#include "port_ultrasound.h"
#include "port_system.h"

/* Project includes */
#include "fsm.h"
#include "fsm_ultrasound.h"

/* Typedefs --------------------------------------------------------------------*/
/**
 * @brief Structure for the finite state machine (FSM) of the ultrasound sensor.
 *
 */
typedef struct fsm_ultrasound_t
{
    fsm_t f;                 /**< Generic finite state machine. Must be the first field. */
    uint32_t distance_cm;    /**< Median distance measured (in cm) after a round of measurements. */
    bool status;             /**< Indicates if the ultrasound FSM is active (true) or paused (false). */
    bool new_measurement;    /**< Flag indicating whether a new distance has been calculated. */
    uint32_t ultrasound_id;  /**< Unique ID of the ultrasound sensor managed by the PORT layer. */
    uint32_t distance_arr[FSM_ULTRASOUND_NUM_MEASUREMENTS]; /**< Array to store last N measurements for median calculation. */
    uint8_t distance_idx;    /**< Index to track the current position in the distance array. */
} fsm_ultrasound_t;


/* Private functions -----------------------------------------------------------*/

/**
 * @brief Comparison function for qsort
 *
 * @param a	Pointer to the first element to compare.
 * @param b	Pointer to the second element to compare.
 * @return Result of the comparison.
 */
int _compare(const void *a, const void *b)
{
    return (*(uint32_t *)a - *(uint32_t *)b);
}

/* State machine input or transition functions */

/**
 * @brief Check if the ultrasound sensor is active and ready to start a new measurement.
 *
 * @param p_this Pointer to the base FSM (cast to fsm_ultrasound_t inside).
 * @return true if the ultrasound sensor is active and trigger is ready, false otherwise.
 */
static bool check_on(fsm_t *p_this)
{
    fsm_ultrasound_t *p_us = (fsm_ultrasound_t *)p_this;
    return p_us->status && port_ultrasound_get_trigger_ready(p_us->ultrasound_id);
}

/**
 * @brief Check if the ultrasound sensor has finished the trigger signal. This function returns the status of the trigger_end flag.
 *
 * @param p_this Pointer to the base FSM (cast to fsm_ultrasound_t inside).
 * @return true if trigger duration has ended, false otherwise.
 */
static bool check_trigger_end(fsm_t *p_this)
{
    fsm_ultrasound_t *p_us = (fsm_ultrasound_t *)p_this;
    return port_ultrasound_get_trigger_end(p_us->ultrasound_id);
}

/**
 * @brief Check if the ultrasound sensor has received the init of the echo signal.
 *
 * @param p_this Pointer to the base FSM (cast to fsm_ultrasound_t inside).
 * @return true if echo_init_tick > 0, false otherwise.
 */
static bool check_echo_init(fsm_t *p_this)
{
    fsm_ultrasound_t *p_us = (fsm_ultrasound_t *)p_this;
    return port_ultrasound_get_echo_init_tick(p_us->ultrasound_id) > 0;
}

/**
 * @brief Check if the echo signal has been fully received (rising and falling edges).
 *
 * @param p_this Pointer to the base FSM (cast to fsm_ultrasound_t inside).
 * @return true if echo_received is true, false otherwise.
 */
static bool check_echo_received(fsm_t *p_this)
{
    fsm_ultrasound_t *p_us = (fsm_ultrasound_t *)p_this;
    return port_ultrasound_get_echo_received(p_us->ultrasound_id);
}

/**
 * @brief Check if a new measurement is ready to be started.
 *
 * @param p_this Pointer to the base FSM (cast to fsm_ultrasound_t inside).
 * @return true if the trigger_ready flag is true, false otherwise.
 */
static bool check_new_measurement(fsm_t *p_this)
{
    fsm_ultrasound_t *p_us = (fsm_ultrasound_t *)p_this;
    return port_ultrasound_get_trigger_ready(p_us->ultrasound_id);
}

/**
 * @brief Check if the ultrasound FSM has been turned off. This function returns the negation of the status flag.
 *
 * @param p_this Pointer to the base FSM (cast to fsm_ultrasound_t inside).
 * @return true if FSM is inactive (off), false otherwise.
 */
static bool check_off(fsm_t *p_this)
{
    fsm_ultrasound_t *p_us = (fsm_ultrasound_t *)p_this;
    return !(p_us->status);
}


/* State machine output or action functions */

/**
 * @brief Start a measurement of the ultrasound transceiver.
 *
 * This function is called the first time the FSM is started.
 *
 * @param p_this Pointer to an fsm_t struct that contains an fsm_ultrasound_t.
 */
static void do_start_measurement(fsm_t *p_this)
{
    fsm_ultrasound_t *p_us = (fsm_ultrasound_t *)p_this;
    port_ultrasound_start_measurement(p_us->ultrasound_id);
}

/**
 * @brief Stop the trigger signal of the ultrasound sensor.
 *
 * @param p_this Pointer to an fsm_t struct that contains an fsm_ultrasound_t.
 */
static void do_stop_trigger(fsm_t *p_this)
{
    fsm_ultrasound_t *p_us = (fsm_ultrasound_t *)p_this;
    port_ultrasound_stop_trigger_timer(p_us->ultrasound_id);
    port_ultrasound_set_trigger_end(p_us->ultrasound_id, false);
}

/**
 * @brief Set the distance measured by the ultrasound sensor.
 *
 * This function calculates the distance based on the echo signal and stores it in the array.
 * If the array is full, it calculates the median and sets the new_measurement flag.
 *
 * @param p_this Pointer to an fsm_t struct that contains an fsm_ultrasound_t.
 */
static void do_set_distance(fsm_t *p_this)
{
    fsm_ultrasound_t *p_us = (fsm_ultrasound_t *)p_this;

    uint32_t init = port_ultrasound_get_echo_init_tick(p_us->ultrasound_id); // Step 1: Retrieve the echo init tick, echo end tick, and echo overflows from the ultrasound sensor
    uint32_t end = port_ultrasound_get_echo_end_tick(p_us->ultrasound_id);
    uint32_t overflows = port_ultrasound_get_echo_overflows(p_us->ultrasound_id);

    uint32_t total_time_us = (end + (overflows * (0xFFFF))) - init;  // Step 2: Calculate the time that the sound has taken to go back and forth

    // Step3 : Calculate the distance in cm taking into account the speed of sound.
    // SPEED_OF_SOUND_MS = 343 m/s => 0.0343 cm/us (divided by 10000) => one-way distance = (time / 2) * 0.0343
    // uint32_t distance_cm = (uint32_t)(((double)total_time_us / 2.0) * ((double)SPEED_OF_SOUND_MS / 10000.0));
    // uint32_t distance_cm = (uint32_t)((((double)total_time_us / 2.0) * ((double)SPEED_OF_SOUND_MS / 10000.0)) + 0.5);
    uint32_t distance_cm = (total_time_us * SPEED_OF_SOUND_MS + 10000) / 20000;  // Calculation without double

    p_us->distance_arr[p_us->distance_idx] = distance_cm;  // Step 4: Store the distance in the array of distances in the position of the index.
    p_us->distance_idx++;
   if (p_us->distance_idx >= FSM_ULTRASOUND_NUM_MEASUREMENTS)  // Step 5: If the array is full, sort the array by calling the function qsort()
    {
        qsort(p_us->distance_arr, FSM_ULTRASOUND_NUM_MEASUREMENTS, sizeof(uint32_t), _compare);
        // Step 6: If the array is full, compute the median of the array and store it in the field distance_cm, making sure to consider if FSM_ULTRASOUND_NUM_MEASUREMENTS is even or odd.
        if (FSM_ULTRASOUND_NUM_MEASUREMENTS % 2 == 1)
        {
            p_us->distance_cm = p_us->distance_arr[FSM_ULTRASOUND_NUM_MEASUREMENTS / 2];  
        }
        else
        {
            int i = FSM_ULTRASOUND_NUM_MEASUREMENTS / 2;
            p_us->distance_cm = (p_us->distance_arr[i - 1] + p_us->distance_arr[i]) / 2;
        }
        p_us->new_measurement = true;  // Step 7: If the array is full, set the flag new_measurement to indicate that a new measurement is ready.
        p_us->distance_idx = 0;  // Step 8: Increase the distance index. If the index is higher or equal FSM_ULTRASOUND_NUM_MEASUREMENTS, reset the index
    }

    port_ultrasound_stop_echo_timer(p_us->ultrasound_id);  // Step 9: Call function port_ultrasound_stop_echo_timer to stop the timer that controls the input capture of the echo signal
    port_ultrasound_reset_echo_ticks(p_us->ultrasound_id);  // Step 10: Call function port_ultrasound_reset_echo_ticks() to reset the time ticks of the echo signal
}

/**
 * @brief Stop the ultrasound sensor.
 *
 * @param p_this Pointer to an fsm_t struct that contains an fsm_ultrasound_t.
 */
static void do_stop_measurement(fsm_t *p_this)
{
    fsm_ultrasound_t *p_us = (fsm_ultrasound_t *)p_this;
    port_ultrasound_stop_ultrasound(p_us->ultrasound_id);
}

/**
 * @brief Start a new measurement.
 *
 * @param p_this Pointer to an fsm_t struct that contains an fsm_ultrasound_t.
 */
static void do_start_new_measurement(fsm_t *p_this)
{
    do_start_measurement(p_this);
}


/** 
 * @brief Transition table for the ultrasound FSM.
 * 
 * The order of transitions is important, especially in the SET_DISTANCE state.
 */
fsm_trans_t fsm_trans_ultrasound[] = {
    { WAIT_START,      check_on,              TRIGGER_START,   do_start_measurement },      /* WAIT_START      -> TRIGGER_START */
    { TRIGGER_START,   check_trigger_end,     WAIT_ECHO_START, do_stop_trigger },           /* TRIGGER_START   -> WAIT_ECHO_START */
    { WAIT_ECHO_START, check_echo_init,       WAIT_ECHO_END,   NULL },                      /* WAIT_ECHO_START -> WAIT_ECHO_END */
    { WAIT_ECHO_END,   check_echo_received,   SET_DISTANCE,    do_set_distance },           /* WAIT_ECHO_END   -> SET_DISTANCE */
    { SET_DISTANCE,    check_new_measurement, TRIGGER_START,   do_start_new_measurement },  /* SET_DISTANCE    -> TRIGGER_START (new measurement) */
    { SET_DISTANCE,    check_off,             WAIT_START,      do_stop_measurement },       /* SET_DISTANCE    -> WAIT_START (if sensor is turned off) */
    { -1, NULL, -1, NULL }
};


/* Other auxiliary functions */

/**
 * @brief Initialize a ultrasound FSM
 * 
 * This function initializes the default values of the FSM struct and calls to the port_ultrasound_init function to initialize the associated HW given the ID.
 * 
 * @param p_fsm_ultrasound Pointer to the button FSM.
 * @param ultrasound_id	Unique button identifier number
 */
void fsm_ultrasound_init(fsm_ultrasound_t *p_fsm_ultrasound, uint32_t ultrasound_id)
{
    // Initialize the FSM
    fsm_init(&p_fsm_ultrasound->f, fsm_trans_ultrasound);

    /* TODO alumnos: */
    // Initialize the fields of the FSM structure
    p_fsm_ultrasound->distance_cm = 0;
    p_fsm_ultrasound->status = false;
    p_fsm_ultrasound->new_measurement = false;
    p_fsm_ultrasound->ultrasound_id = ultrasound_id;
    p_fsm_ultrasound->distance_idx = 0;

    // Initialize distance array to 0
    memset(p_fsm_ultrasound->distance_arr, 0, sizeof(p_fsm_ultrasound->distance_arr));

    // Initialize the associated ultrasound HW via the port
    port_ultrasound_init(ultrasound_id);
}

/* Public functions -----------------------------------------------------------*/
fsm_ultrasound_t *fsm_ultrasound_new(uint32_t ultrasound_id)
{
    fsm_ultrasound_t *p_fsm_ultrasound = malloc(sizeof(fsm_ultrasound_t)); /* Do malloc to reserve memory of all other FSM elements, although it is interpreted as fsm_t (the first element of the structure) */
    fsm_ultrasound_init(p_fsm_ultrasound, ultrasound_id);                  /* Initialize the FSM */
    return p_fsm_ultrasound;
}

void fsm_ultrasound_fire(fsm_ultrasound_t *p_fsm)
{
    fsm_fire(&p_fsm->f);
}

void fsm_ultrasound_destroy(fsm_ultrasound_t *p_fsm)
{
        free(p_fsm);
}

fsm_t* fsm_ultrasound_get_inner_fsm(fsm_ultrasound_t *p_fsm)
{
    return &p_fsm->f;
}

uint32_t fsm_ultrasound_get_state(fsm_ultrasound_t *p_fsm)
{
    return p_fsm->f.current_state;
}

uint32_t fsm_ultrasound_get_distance(fsm_ultrasound_t *p_fsm)
{
    p_fsm->new_measurement = false;
    return p_fsm->distance_cm;
}

void fsm_ultrasound_stop(fsm_ultrasound_t *p_fsm)
{
    p_fsm->status = false;
    port_ultrasound_stop_ultrasound(p_fsm->ultrasound_id);
}

void fsm_ultrasound_start(fsm_ultrasound_t *p_fsm)
{
    p_fsm->status = true;
    p_fsm->distance_cm = 0;
    p_fsm->distance_idx = 0;
    port_ultrasound_reset_echo_ticks(p_fsm->ultrasound_id);
    port_ultrasound_set_trigger_ready(p_fsm->ultrasound_id, true);
    port_ultrasound_start_new_measurement_timer();
}

bool fsm_ultrasound_get_status(fsm_ultrasound_t *p_fsm)
{
    return p_fsm->status;
}

void fsm_ultrasound_set_status(fsm_ultrasound_t *p_fsm, bool status)
{
    p_fsm->status = status;
}

bool fsm_ultrasound_get_ready(fsm_ultrasound_t *p_fsm)
{
    return port_ultrasound_get_trigger_ready(p_fsm->ultrasound_id);
}

bool fsm_ultrasound_get_new_measurement_ready(fsm_ultrasound_t *p_fsm)
{
    return p_fsm->new_measurement;
}


// Other auxiliary functions
void fsm_ultrasound_set_state(fsm_ultrasound_t *p_fsm, int8_t state)
{
    p_fsm->f.current_state = state;
}


bool fsm_ultrasound_check_activity(fsm_ultrasound_t *p_fsm)
{
    return false;
}
