/**
 * @file fsm_urbanite.c
 * @brief Urbanite FSM main file.
 * @author Elio Flandin (elio.flandin@ensea.fr)
 * @date 2025-09-06
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>
#include <stdio.h>

/* HW dependent includes */
#include "port_system.h"

/* Project includes */
#include "fsm.h"
#include "fsm_display.h"
#include "fsm_ultrasound.h"
#include "fsm_button.h"
#include "fsm_urbanite.h"

/* Typedefs --------------------------------------------------------------------*/
/**
 * @brief Structure to define the Urbanite FSM.
 */
struct fsm_urbanite_t {
    fsm_t f;                                  /**< Urbanite FSM */
    fsm_button_t *p_fsm_button;               /**< Pointer to the button FSM */
    uint32_t on_off_press_time_ms;            /**< Time in milliseconds to consider ON/OFF */
    uint32_t pause_display_time_ms;           /**< Time in milliseconds to pause the display */
    bool is_paused;                           /**< Flag to indicate if the system is paused */
    fsm_ultrasound_t *p_fsm_ultrasound_rear;  /**< Pointer to the rear ultrasound FSM */
    fsm_display_t *p_fsm_display_rear;        /**< Pointer to the rear display FSM */
};


/* Private functions -----------------------------------------------------------*/

/* State machine input or transition functions */

/**
 * @brief Check if the button has been pressed for the required time to turn ON the Urbanite system.
 * 
 * This function gets the button press duration and compares it to the ON/OFF threshold.
 *
 * @param p_this Pointer to the fsm_t struct containing the Urbanite FSM.
 * @return true if the press duration is long enough to turn ON the system, false otherwise.
 */
static bool check_on(fsm_t *p_this)
{
    fsm_urbanite_t *p_urbanite = (fsm_urbanite_t *)p_this;
    uint32_t duration = fsm_button_get_duration(p_urbanite->p_fsm_button);
    return (duration > 0 && duration > p_urbanite->on_off_press_time_ms);
}

/**
 * @brief Check if the button has been pressed for the required time to turn OFF the Urbanite system.
 * 
 * This function simply reuses the check_on() logic as the ON/OFF threshold is the same.
 *
 * @param p_this Pointer to the fsm_t struct containing the Urbanite FSM.
 * @return true if the press duration is long enough to turn OFF the system, false otherwise.
 */
static bool check_off(fsm_t *p_this)
{
    return check_on(p_this);
}

/**
 * @brief Check if a new distance measurement is ready.
 * 
 * This function calls the ultrasound FSM to check if a new measurement is ready and return the result.
 *
 * @param p_this Pointer to the fsm_t struct containing the Urbanite FSM.
 * @return true if a new measurement is ready, false otherwise.
 */
static bool check_new_measure(fsm_t *p_this)
{
    fsm_urbanite_t *p_urbanite = (fsm_urbanite_t *)p_this;
    return fsm_ultrasound_get_new_measurement_ready(p_urbanite->p_fsm_ultrasound_rear);
}

/**
 * @brief Check if the display should be paused based on button press duration.
 * 
 * This function evaluates if the press duration falls within the pause threshold.
 *
 * @param p_this Pointer to the fsm_t struct containing the Urbanite FSM.
 * @return true if the duration is enough to pause the display, false otherwise.
 */
static bool check_pause_display(fsm_t *p_this)
{
    fsm_urbanite_t *p_urbanite = (fsm_urbanite_t *)p_this;
    uint32_t duration = fsm_button_get_duration(p_urbanite->p_fsm_button);
    return (duration > 0 && duration < p_urbanite->on_off_press_time_ms && duration > p_urbanite->pause_display_time_ms);
}


/**
 * @brief Check if any of the elements of the system is active.
 *
 * This function returns true if at least one of the elements in the system (button, ultrasound, or display) is active. Otherwise, it returns false.
 *
 * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
 * @return true if any element is active, false otherwise.
 */
static bool check_activity(fsm_t *p_this)
{
    fsm_urbanite_t *p_urbanite = (fsm_urbanite_t *)p_this;
    return (fsm_button_check_activity(p_urbanite->p_fsm_button) ||
            fsm_ultrasound_check_activity(p_urbanite->p_fsm_ultrasound_rear) ||
            fsm_display_check_activity(p_urbanite->p_fsm_display_rear));
}

/**
 * @brief Check if all the elements of the system are inactive.
 *
 * This function calls check_activity() and returns its negation.
 *
 * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
 * @return true if all elements are inactive, false otherwise.
 */
static bool check_no_activity(fsm_t *p_this)
{
    return !check_activity(p_this);
}

/**
 * @brief Check if a new measurement is ready while in low power mode.
 *
 * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
 * @return true if a new measurement is ready, false otherwise.
 */
static bool check_activity_in_measure(fsm_t *p_this)
{
    return check_new_measure(p_this);
}


/* State machine output or action functions */

/**
 * @brief Turn the Urbanite system ON.
 * 
 * This function resets the duration of the button to avoid the system to turn OFF again, 
 * starts the ultrasound sensor (which will start measuring the distance) and sets the appropriate status of the display system.
 *
 * @param p_this Pointer to the fsm_t struct containing the Urbanite FSM.
 */
static void do_start_up_measure(fsm_t *p_this) {
    fsm_urbanite_t *p_fsm = (fsm_urbanite_t *)p_this;
    fsm_button_reset_duration(p_fsm->p_fsm_button);
    fsm_ultrasound_start(p_fsm->p_fsm_ultrasound_rear);
    fsm_display_set_status(p_fsm->p_fsm_display_rear, true);
    printf("[URBANITE][%ld] Urbanite system ON\n", port_system_get_millis());
}

/**
 * @brief Turn the Urbanite system OFF.
 * 
 * This function resets the duration of the button to avoid the system to turn ON again, 
 * stops the ultrasound sensor (which will stop measuring the distance) and turns the display system off.
 *
 * @param p_this Pointer to the fsm_t struct containing the Urbanite FSM.
 */
static void do_stop_urbanite(fsm_t *p_this) {
    fsm_urbanite_t *p_fsm = (fsm_urbanite_t *)p_this;
    fsm_button_reset_duration(p_fsm->p_fsm_button);
    fsm_ultrasound_stop(p_fsm->p_fsm_ultrasound_rear);
    fsm_display_set_status(p_fsm->p_fsm_display_rear, false);
    if (p_fsm->is_paused) {
        p_fsm->is_paused = false;
    }
    printf("[URBANITE][%ld] Urbanite system OFF\n", port_system_get_millis());
}

/**
 * @brief Pause or resume the display system.
 * 
 * This function resets the duration of the button to avoid the system to pause again and inverts the pause status.
 * Activate or deactivate the display depending on the new pause status.
 *
 * @param p_this Pointer to the fsm_t struct containing the Urbanite FSM.
 */
static void do_pause_display(fsm_t *p_this) {
    fsm_urbanite_t *p_fsm = (fsm_urbanite_t *)p_this;
    fsm_button_reset_duration(p_fsm->p_fsm_button);
    p_fsm->is_paused = !p_fsm->is_paused;
    fsm_display_set_status(p_fsm->p_fsm_display_rear, !p_fsm->is_paused);
    if (p_fsm->is_paused)
        printf("[URBANITE][%ld] Urbanite system display PAUSE\n", port_system_get_millis());
    else
        printf("[URBANITE][%ld] Urbanite system display RESUME\n", port_system_get_millis());
}

/**
 * @brief Display the distance measured by the ultrasound sensor.
 * 
 * This function gets the distance measured by the ultrasound sensor.
 * If the system is paused : If the distance is less than WARNING_MIN_CM / 2 cm, set the distance to the display and set the display status to true. Otherwise, set the display status to false.
 * If the system is not paused, set the distance to the display.
 *
 * @param p_this Pointer to the fsm_t struct containing the Urbanite FSM.
 */
static void do_display_distance(fsm_t *p_this) {
    fsm_urbanite_t *p_fsm = (fsm_urbanite_t *)p_this;
    uint32_t distance_cm = fsm_ultrasound_get_distance(p_fsm->p_fsm_ultrasound_rear);
    if (p_fsm->is_paused) {
        if (distance_cm < WARNING_MIN_CM / 2) {
            fsm_display_set_distance(p_fsm->p_fsm_display_rear, distance_cm);
            fsm_display_set_status(p_fsm->p_fsm_display_rear, true);
        } else {
            fsm_display_set_status(p_fsm->p_fsm_display_rear, false);
        }
    } else {
        fsm_display_set_distance(p_fsm->p_fsm_display_rear, distance_cm);
    }
    printf("[URBANITE][%ld] Distance: %ld cm\n", port_system_get_millis(), distance_cm);
}

/**
 * @brief Start the low power mode while the Urbanite is OFF.
 *
 * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
 */
static void do_sleep_off(fsm_t *p_this) {
    port_system_sleep();
}

/**
 * @brief Start the low power mode while the Urbanite is awakened by a debug breakpoint or similar in the SLEEP_WHILE_OFF state.
 *
 * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
 */
static void do_sleep_while_off(fsm_t *p_this) {
    port_system_sleep();
}

/**
 * @brief Start the low power mode while the Urbanite is awakened by a debug breakpoint or similar in the SLEEP_WHILE_ON state.
 *
 * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
 */
static void do_sleep_while_on(fsm_t *p_this) {
    port_system_sleep();
}

/**
 * @brief Start the low power mode while the Urbanite is measuring the distance and it is waiting for a new measurement.
 * 
 * This function is also called do_sleep_wait_command in the Urbanite subject.
 *
 * @param p_this Pointer to an fsm_t struct that contains an fsm_urbanite_t.
 */
static void do_sleep_while_measure(fsm_t *p_this) {
    port_system_sleep();
}


/* Transition table ------------------------------------------------------------*/

/** @brief Transition table of the FSM Urbanite. */
fsm_trans_t fsm_trans_urbanite[] = {
    { OFF,             check_on,                  MEASURE,         do_start_up_measure },
    { MEASURE,         check_off,                 OFF,             do_stop_urbanite },
    { MEASURE,         check_pause_display,       MEASURE,         do_pause_display },
    { MEASURE,         check_new_measure,         MEASURE,         do_display_distance },
    { MEASURE,         check_no_activity,         SLEEP_WHILE_ON,  do_sleep_while_measure },
    { SLEEP_WHILE_ON,  check_activity_in_measure, MEASURE,         NULL },
    { SLEEP_WHILE_ON,  check_no_activity,         SLEEP_WHILE_ON,  do_sleep_while_on },
    { OFF,             check_no_activity,         SLEEP_WHILE_OFF, do_sleep_off },
    { SLEEP_WHILE_OFF, check_activity,            OFF,             NULL },
    { SLEEP_WHILE_OFF, check_no_activity,         SLEEP_WHILE_OFF, do_sleep_while_off },
    { -1, NULL, -1, NULL }
};

/**
 * @brief Initialize the Urbanite FSM.
 *
 * This function initializes the FSM of the Urbanite system. It sets up the transition table,
 * initializes the pointers to the button, ultrasound, and display FSMs, and configures the
 * button press durations for ON/OFF and display pause detection. It also ensures that the
 * system starts in a non-paused state.
 *
 * @param p_fsm_urbanite Pointer to the Urbanite FSM to initialize.
 * @param p_fsm_button Pointer to the button FSM that controls the system.
 * @param on_off_press_time_ms Time in milliseconds to turn the system ON or OFF.
 * @param pause_display_time_ms Time in milliseconds to pause the display with a short press.
 * @param p_fsm_ultrasound_rear Pointer to the ultrasound FSM (rear sensor).
 * @param p_fsm_display_rear Pointer to the RGB display FSM (rear).
 */
static void fsm_urbanite_init(
    fsm_urbanite_t *p_fsm_urbanite,
    fsm_button_t *p_fsm_button,
    uint32_t on_off_press_time_ms,
    uint32_t pause_display_time_ms,
    fsm_ultrasound_t *p_fsm_ultrasound_rear,
    fsm_display_t *p_fsm_display_rear
)
{
    fsm_init(&p_fsm_urbanite->f, fsm_trans_urbanite);  // Initialize the FSM with the received pointer to fsm_t and its transition table

    // Initialize the FSM fields with the received parameters
    p_fsm_urbanite->p_fsm_button = p_fsm_button;
    p_fsm_urbanite->on_off_press_time_ms = on_off_press_time_ms;
    p_fsm_urbanite->pause_display_time_ms = pause_display_time_ms;
    p_fsm_urbanite->p_fsm_ultrasound_rear = p_fsm_ultrasound_rear;
    p_fsm_urbanite->p_fsm_display_rear = p_fsm_display_rear;
    p_fsm_urbanite->is_paused = false;  // System starts in unpaused state (initialize is_paused to false)
}

/* Public functions -----------------------------------------------------------*/

fsm_urbanite_t *fsm_urbanite_new(
    fsm_button_t* p_fsm_button,
    uint32_t on_off_press_time_ms,
    uint32_t pause_display_time_ms,
    fsm_ultrasound_t* p_fsm_ultrasound_rear,
    fsm_display_t* p_fsm_display_rear)
{
    fsm_urbanite_t *p_fsm_urbanite = malloc(sizeof(fsm_urbanite_t)); 
    fsm_urbanite_init(p_fsm_urbanite, p_fsm_button, on_off_press_time_ms, pause_display_time_ms, p_fsm_ultrasound_rear, p_fsm_display_rear);
    return p_fsm_urbanite;
}

void fsm_urbanite_fire(fsm_urbanite_t *p_fsm)
{
    fsm_fire(&(p_fsm->f));
}

void fsm_urbanite_destroy(fsm_urbanite_t *p_fsm)
{
    free(p_fsm);
}