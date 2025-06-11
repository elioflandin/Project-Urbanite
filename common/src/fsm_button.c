/**
 * @file fsm_button.c
 * @brief Button FSM main file.
 * @author alumno1
 * @author alumno2
 * @date fecha
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>
#include <stdio.h>

/* HW dependent includes */
#include "port_button.h"
#include "port_system.h"

/* Project includes */
#include "fsm_button.h"
#include "fsm.h"

/**
 * @brief Structure of the Button FSM
 */
struct fsm_button_t 
{
    fsm_t f;                /*!< Button FSM */
    uint32_t debounce_time; /*!< Button debounce time in ms */
    uint32_t next_timeout;  /*!< Next timeout for the anti-debounce in ms */
    uint32_t tick_pressed;  /*!< Number of ticks when the button was pressed */
    uint32_t duration;      /*!< How much time the button has been pressed */
    uint32_t button_id;     /*!< Button ID. Must be unique */
};

static void fsm_button_init(fsm_button_t *p_fsm_button, uint32_t debounce_time, uint32_t button_id);

/* State machine input or transition functions */

/**
 * @brief Check if the button has been pressed (flag is true).
 *
 * @param p_this Pointer to an fsm_t struct castable to fsm_button_t.
 * @return true if the button was pressed.
 * @return false otherwise.
 */
static bool check_button_pressed(fsm_t *p_this);

/**
 * @brief Check if the button has been released (flag is false).
 *
 * @param p_this Pointer to an fsm_t struct castable to fsm_button_t.
 * @return true if the button was released.
 * @return false otherwise.
 */
static bool check_button_released(fsm_t *p_this);

/**
 * @brief Check if the debounce timeout has passed.
 *
 * Compares the current system tick to the next_timeout value stored in the FSM.
 *
 * @param p_this Pointer to an fsm_t struct castable to fsm_button_t.
 * @return true if the timeout has expired.
 * @return false otherwise.
 */
static bool check_timeout(fsm_t *p_this);


static bool check_button_pressed(fsm_t *p_this)
{
    fsm_button_t *p_fsm = (fsm_button_t *)p_this;
    return port_button_get_pressed(p_fsm->button_id);
}

static bool check_button_released(fsm_t *p_this)
{
    fsm_button_t *p_fsm = (fsm_button_t *)p_this;
    return !port_button_get_pressed(p_fsm->button_id);
}

static bool check_timeout(fsm_t *p_this)
{
    fsm_button_t *p_fsm = (fsm_button_t *)p_this;
    uint32_t now = port_system_get_millis();
    return now > p_fsm->next_timeout;
}


/* State machine output or action functions */

/**
 * @brief Store the duration of the button press.
 *
 * Computes the duration from tick_pressed and the current tick,
 * then sets the next timeout.
 *
 * @param p_this Pointer to an fsm_t struct castable to fsm_button_t.
 */
static void do_set_duration(fsm_t *p_this);

/**
 * @brief Store the current system tick as the tick_pressed value.
 *
 * Also sets the next timeout value according to the debounce time.
 *
 * @param p_this Pointer to an fsm_t struct castable to fsm_button_t.
 */
static void do_store_tick_pressed(fsm_t *p_this);


static void do_set_duration(fsm_t *p_this)
{
    fsm_button_t *p_fsm = (fsm_button_t *)p_this;
    uint32_t now = port_system_get_millis();
    p_fsm->duration = now - p_fsm->tick_pressed;
    p_fsm->next_timeout = now + p_fsm->debounce_time;
}

static void do_store_tick_pressed(fsm_t *p_this)
{
    fsm_button_t *p_fsm = (fsm_button_t *)p_this;
    uint32_t now = port_system_get_millis();
    p_fsm->tick_pressed = now;
    p_fsm->next_timeout = now + p_fsm->debounce_time;
}


/**
 * @brief Transition table of the button FSM.
 * 
 * Each row: { current_state, condition_function, next_state, action_function }
 */
static fsm_trans_t fsm_trans_button[] = {
    { BUTTON_RELEASED,       check_button_pressed,       BUTTON_PRESSED_WAIT,   do_store_tick_pressed },
    { BUTTON_PRESSED_WAIT,   check_timeout,              BUTTON_PRESSED,        NULL },
    { BUTTON_PRESSED,        check_button_released,      BUTTON_RELEASED_WAIT,  do_set_duration },
    { BUTTON_RELEASED_WAIT,  check_timeout,              BUTTON_RELEASED,       NULL },
    { -1,                    NULL,                       -1,                    NULL }
};


/* Other auxiliary functions */
/**
 * @brief Initialize a button FSM
 * 
 * This function initializes the default values of the FSM struct and calls to the port_button_init function to initialize the associated HW given the ID.
 * 
 * @param p_fsm_button Pointer to the button FSM.
 * @param debounce_time	Anti-debounce time in milliseconds
 * @param button_id	Unique button identifier number
 */
void fsm_button_init(fsm_button_t *p_fsm_button, uint32_t debounce_time, uint32_t button_id)
{
    fsm_init(&p_fsm_button->f, fsm_trans_button);

    p_fsm_button->debounce_time = debounce_time;  //Store configuration parameters
    p_fsm_button->button_id = button_id;

    p_fsm_button->tick_pressed = 0;  // Reset timing variables
    p_fsm_button->duration = 0;
    p_fsm_button->next_timeout = 0;

    port_button_init(button_id);  // Initialize the HW associated with the button
}

/* Public functions -----------------------------------------------------------*/
fsm_button_t *fsm_button_new(uint32_t debounce_time, uint32_t button_id)
{
    fsm_button_t *p_fsm_button = malloc(sizeof(fsm_button_t)); /* Do malloc to reserve memory of all other FSM elements, although it is interpreted as fsm_t (the first element of the structure) */
    fsm_button_init(p_fsm_button, debounce_time, button_id);   /* Initialize the FSM */
    return p_fsm_button;                                       /* Composite pattern: return the fsm_t pointer as a fsm_button_t pointer */
}

uint32_t fsm_button_get_debounce_time_ms(fsm_button_t *p_fsm)
{
    return p_fsm->debounce_time;
}

uint32_t fsm_button_get_duration(fsm_button_t *p_fsm)
{
    return p_fsm->duration;
}

void fsm_button_reset_duration(fsm_button_t *p_fsm)
{
    p_fsm->duration = 0;
}


void fsm_button_fire(fsm_button_t *p_fsm)
{
    //printf("fire\n");  // For debugging purpose
    fsm_fire(&p_fsm->f); // Is it also possible to it in this way: fsm_fire((fsm_t *)p_fsm);
}

void fsm_button_destroy(fsm_button_t *p_fsm)
{
    free(p_fsm);  // free(&p_fsm->f);  changed because we need to free all p_fsm bloc and not just &p_fsm->f
}

fsm_t *fsm_button_get_inner_fsm(fsm_button_t *p_fsm)
{
    return &p_fsm->f;
}

uint32_t fsm_button_get_state(fsm_button_t *p_fsm)
{
    return p_fsm->f.current_state;
}


bool fsm_button_check_activity(fsm_button_t *p_fsm) {
    uint32_t current_state = p_fsm->f.current_state;  // 1. Retrieve current FSM status
    return (current_state != BUTTON_RELEASED);  // 2. Return false if status is BUTTON_RELEASED, otherwise true
}
