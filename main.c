/**
 * @file main.c
 * @brief Main file.
 * @author Sistemas Digitales II
 * @date 2025-01-01
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C libraries */
#include <stdio.h> // printf
#include <stdlib.h>
#include <stdint.h>

/* HW libraries */
#include "port_system.h"
#include "port_button.h"
#include "port_ultrasound.h"
#include "port_display.h"

/* Project includes */
#include "fsm.h"
#include "fsm_display.h"
#include "fsm_ultrasound.h"
#include "fsm_button.h"
#include "fsm_urbanite.h"

/* Defines ------------------------------------------------------------------*/

#define URBANITE_ON_OFF_PRESS_TIME_MS 1000  /*!< Time in ms to activate the Urbanite system, started mainly due to a parking maneuver (long press) */
#define URBANITE_PAUSE_DISPLAY_TIME_MS 500  /*!< Time in ms to pause the display */


/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* Init board */
    port_system_init();

    fsm_button_t *p_fsm_button = fsm_button_new(PORT_PARKING_BUTTON_DEBOUNCE_TIME_MS, PORT_PARKING_BUTTON_ID);  // Create the FSM for the button
    fsm_ultrasound_t *p_fsm_ultrasound = fsm_ultrasound_new(PORT_REAR_PARKING_SENSOR_ID);                       // Create the FSM for the ultrasound sensor
    fsm_display_t *p_fsm_display = fsm_display_new(PORT_REAR_PARKING_DISPLAY_ID);                               // Create the FSM for the display (LED)

    fsm_urbanite_t *p_fsm_urbanite = fsm_urbanite_new(  // Create the Urbanite FSM
        p_fsm_button,
        URBANITE_ON_OFF_PRESS_TIME_MS,
        URBANITE_PAUSE_DISPLAY_TIME_MS,
        p_fsm_ultrasound,
        p_fsm_display
    );
    
    /* Infinite loop */
    while (1)
    {
        fsm_button_fire(p_fsm_button);          // Update the FSM for the button
        fsm_ultrasound_fire(p_fsm_ultrasound);  // Update the FSM for the ultrasound sensor
        fsm_display_fire(p_fsm_display);        // Update the FSM for the display
        fsm_urbanite_fire(p_fsm_urbanite);      // Update the Urbanite FSM

    } // End of while(1)

    fsm_button_destroy(p_fsm_button);          // Free the memory of the button FSM
    fsm_ultrasound_destroy(p_fsm_ultrasound);  // Free the memory of the ultrasound sensor FSM
    fsm_display_destroy(p_fsm_display);        // Free the memory of the display FSM
    fsm_urbanite_destroy(p_fsm_urbanite);      // Free the memory of the Urbanite FSM

    return 0;
}
