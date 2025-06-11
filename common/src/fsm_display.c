/**
 * @file fsm_display.c
 * @brief Display system FSM main file.
 * @author alumno1
 * @author alumno2
 * @date fecha
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>
#include <stdio.h>

/* HW dependent includes */
#include "port_display.h"
#include "port_system.h"

/* Project includes */
#include "fsm.h"
#include "fsm_display.h"

/* Typedefs --------------------------------------------------------------------*/

/**
 * @brief Display FSM structure.
 *
 */
struct fsm_display_t {
    fsm_t f;                /**< Finite State Machine instance (must be first) */
    int32_t distance_cm;    /**< Distance in cm to be represented */
    bool new_color;         /**< Flag: new color must be set */
    bool status;            /**< Flag: display is active or not */
    bool idle;              /**< Flag: display is idle (no distance change) */
    uint32_t display_id;    /**< Unique identifier of the RGB LED display. */
};


/* Private functions -----------------------------------------------------------*/

/**
 * @brief Compute the RGB color levels based on distance.
 *
 * This function sets the RGB color duty cycle values (from 0 to PORT_DISPLAY_RGB_MAX_VALUE)
 * according to the distance detected by the ultrasound sensor. The result is stored in the
 * rgb_color_t structure provided as a pointer.
 *
 * @param p_color Pointer to the color structure to populate.
 * @param distance_cm Distance measured by the ultrasound sensor (in centimeters).
 */
static void _compute_display_levels(rgb_color_t *p_color, int32_t distance_cm)
{
    if (distance_cm >= DANGER_MIN_CM && distance_cm <= DANGER_MAX_CM) {
        p_color->r = PORT_DISPLAY_RGB_MAX_VALUE;
        p_color->g = 0;
        p_color->b = 0;
    }
    else if (distance_cm > WARNING_MIN_CM && distance_cm <= WARNING_MAX_CM) {
        p_color->r = (uint8_t)(0.60 * PORT_DISPLAY_RGB_MAX_VALUE);
        p_color->g = (uint8_t)(0.37 * PORT_DISPLAY_RGB_MAX_VALUE);
        p_color->b = 0;
    }
    else if (distance_cm > NO_PROBLEM_MIN_CM && distance_cm <= NO_PROBLEM_MAX_CM) {
        p_color->r = 0;
        p_color->g = PORT_DISPLAY_RGB_MAX_VALUE;
        p_color->b = 0;
    }
    else if (distance_cm > INFO_MIN_CM && distance_cm <= INFO_MAX_CM) {
        p_color->r = (uint8_t)(0.10 * PORT_DISPLAY_RGB_MAX_VALUE);
        p_color->g = (uint8_t)(0.35 * PORT_DISPLAY_RGB_MAX_VALUE);
        p_color->b = (uint8_t)(0.32 * PORT_DISPLAY_RGB_MAX_VALUE);
    }
    else if (distance_cm > OK_MIN_CM && distance_cm <= OK_MAX_CM) {
        p_color->r = 0;
        p_color->g = 0;
        p_color->b = PORT_DISPLAY_RGB_MAX_VALUE;
    }
    else {  // Inactive (distance out of range)
        p_color->r = 0;
        p_color->g = 0;
        p_color->b = 0;
    }
}


/* State machine input or transition functions */

/**
 * @brief Check if the display system is set to active.
 *
 * @param p_this Pointer to the internal FSM (fsm_t).
 * @return true if the display is active, false otherwise.
 */
static bool check_active(fsm_t *p_this)
{
    fsm_display_t *p_fsm = (fsm_display_t *)p_this;
    return p_fsm->status;
}

/**
 * @brief Check if a new color has to be set.
 *
 * @param p_this Pointer to the internal FSM (fsm_t).
 * @return true if a new color needs to be set, false otherwise.
 */
static bool check_set_new_color(fsm_t *p_this)
{
    fsm_display_t *p_fsm = (fsm_display_t *)p_this;
    return p_fsm->new_color;
}

/**
 * @brief Check if the display system is set to inactive.
 *
 * @param p_this Pointer to the internal FSM (fsm_t).
 * @return true if the display is inactive, false otherwise.
 */
static bool check_off(fsm_t *p_this)
{
    fsm_display_t *p_fsm = (fsm_display_t *)p_this;
    return !(p_fsm->status);
}


/* State machine output or action functions */

/**
 * @brief Turn ON the display for the first time (with no color).
 *
 * This function initializes the display in ON state, but with RGB color set to OFF.
 *
 * @param p_this Pointer to the internal FSM (fsm_t).
 */
static void do_set_on(fsm_t *p_this)
{
    fsm_display_t *p_fsm = (fsm_display_t *)p_this;
    port_display_set_rgb(p_fsm->display_id, COLOR_OFF);
}

/**
 * @brief Set the RGB color based on the distance measured.
 *
 * This function computes the RGB levels from the distance, sets the color,
 * clears the new_color flag, and sets the system to idle.
 *
 * @param p_this Pointer to the internal FSM (fsm_t).
 */
static void do_set_color(fsm_t *p_this)
{
    fsm_display_t *p_fsm = (fsm_display_t *)p_this;
    rgb_color_t color;

    _compute_display_levels(&color, p_fsm->distance_cm);  // 1. Compute color based on distance
    port_display_set_rgb(p_fsm->display_id, color);  // 2. Set color via port
    p_fsm->new_color = false;  // 3. Reset new_color flag
    p_fsm->idle = true;  // 4. Set idle flag (system active but stable)
}

/**
 * @brief Turn OFF the display system.
 *
 * This function sets the RGB LED to off and resets the idle flag.
 *
 * @param p_this Pointer to the internal FSM (fsm_t).
 */
static void do_set_off(fsm_t *p_this)
{
    fsm_display_t *p_fsm = (fsm_display_t *)p_this;
    port_display_set_rgb(p_fsm->display_id, COLOR_OFF);
    p_fsm->idle = false;
}


/* Transition table ------------------------------------------------------------*/

/**
 * @brief Array representing the transitions table of the FSM display
 *
 */
static fsm_trans_t fsm_trans_display[] = {
    { WAIT_DISPLAY, check_active,        SET_DISPLAY,  do_set_on    },
    { SET_DISPLAY,  check_set_new_color, SET_DISPLAY,  do_set_color },
    { SET_DISPLAY,  check_off,           WAIT_DISPLAY, do_set_off   },
    {-1, NULL, -1, NULL}
};

/* Other auxiliary functions */

/**
 * @brief Initialize the FSM for a display system.
 *
 * This function sets the default values in the FSM structure and initializes
 * the associated hardware display using the given ID.
 *
 * @param p_fsm_display Pointer to the FSM structure.
 * @param display_id Unique identifier of the RGB LED display.
 */
static void fsm_display_init(fsm_display_t *p_fsm_display, uint32_t display_id)
{
    fsm_init(&(p_fsm_display->f), fsm_trans_display);  // 1. Initialize the FSM structure
    p_fsm_display->display_id = display_id;  // 2. Set display ID
    p_fsm_display->distance_cm = -1;  // 3. Set an invalid distance

    p_fsm_display->new_color = false;  // 4. Reset all status flags
    p_fsm_display->status = false;
    p_fsm_display->idle = false;

    port_display_init(display_id);  // 5. Initialize the HW
}

/* Public functions -----------------------------------------------------------*/

fsm_display_t *fsm_display_new(uint32_t display_id)
{
    fsm_display_t *p_fsm_display = malloc(sizeof(fsm_display_t)); /* Do malloc to reserve memory of all other FSM elements, although it is interpreted as fsm_t (the first element of the structure) */
    fsm_display_init(p_fsm_display, display_id); /* Initialize the FSM */
    return p_fsm_display;
}

void fsm_display_fire(fsm_display_t *p_fsm)
{
    fsm_fire(&(p_fsm->f));
}

void fsm_display_destroy(fsm_display_t *p_fsm)
{
    free(p_fsm);
}

fsm_t* fsm_display_get_inner_fsm(fsm_display_t *p_fsm)
{
    return &(p_fsm->f);
}

uint32_t fsm_display_get_state(fsm_display_t *p_fsm)
{
    return p_fsm->f.current_state;
}

bool fsm_display_get_status(fsm_display_t *p_fsm)
{
    return p_fsm->status;
}

void fsm_display_set_status(fsm_display_t *p_fsm, bool pause)
{
    p_fsm->status = pause;
}

void fsm_display_set_distance(fsm_display_t *p_fsm, uint32_t distance_cm)
{
    p_fsm->distance_cm = distance_cm;
    p_fsm->new_color = true;
}

int32_t fsm_display_get_distance(fsm_display_t *p_fsm)
{
    return p_fsm->distance_cm;
}

void fsm_display_set_state(fsm_display_t *p_fsm, int8_t state) {
    p_fsm->f.current_state = state;
}

bool fsm_display_check_activity(fsm_display_t *p_fsm) {
    return (p_fsm->status && !p_fsm->idle);
}
