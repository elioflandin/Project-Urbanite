/**
 * @file port_ultrasound.h
 * @brief Header for the portable functions to interact with the HW of the ultrasound sensors. The functions must be implemented in the platform-specific code.
 * @author alumno1
 * @author alumno2
 * @date fecha
 */
#ifndef PORT_ULTRASOUND_H_
#define PORT_ULTRASOUND_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>

/* Defines and enums ----------------------------------------------------------*/
#define PORT_REAR_PARKING_SENSOR_ID 0         /*!< Identifier for the rear ultrasound sensor. */
#define PORT_PARKING_SENSOR_TRIGGER_UP_US 10  /*!< Duration of the trigger signal in microseconds */
#define PORT_PARKING_SENSOR_TIMEOUT_MS 100    /*!< Timeout between consecutive measurements in milliseconds */
#define SPEED_OF_SOUND_MS 343                 /*!< Speed of sound in air in m/s */

/* Function prototypes and explanation -------------------------------------------------*/

/** 
 * @brief Initialize the ultrasound sensor hardware.
 * @param ultrasound_id Identifier of the ultrasound sensor
 */
void port_ultrasound_init(uint32_t ultrasound_id);

/** 
 * @brief Stop the trigger timer and set trigger pin to low.
 * @param ultrasound_id Identifier of the ultrasound sensor
 */
void port_ultrasound_stop_trigger_timer(uint32_t ultrasound_id);

/** 
 * @brief Get whether the trigger is ready to send.
 * @param ultrasound_id Identifier of the ultrasound sensor
 * @return true if ready to trigger, false otherwise
 */
bool port_ultrasound_get_trigger_ready(uint32_t ultrasound_id);

/** 
 * @brief Set the trigger readiness flag.
 * @param ultrasound_id Identifier of the ultrasound sensor
 * @param trigger_ready New readiness value
 */
void port_ultrasound_set_trigger_ready(uint32_t ultrasound_id, bool trigger_ready);

/** 
 * @brief Get whether the trigger period has ended.
 * @param ultrasound_id Identifier of the ultrasound sensor
 * @return true if trigger has ended, false otherwise
 */
bool port_ultrasound_get_trigger_end(uint32_t ultrasound_id);

/** 
 * @brief Set the trigger end flag.
 * @param ultrasound_id Identifier of the ultrasound sensor
 * @param trigger_end New trigger end value
 */
void port_ultrasound_set_trigger_end(uint32_t ultrasound_id, bool trigger_end);


/**
 * @brief Resets the time ticks of the echo signal once the distance has been calculated.
 *
 * @param ultrasound_id Identifier of the ultrasound sensor
 */
void port_ultrasound_reset_echo_ticks(uint32_t ultrasound_id);

/**
 * @brief Stop the timer that controls the echo signal because the echo signal has been received.
 *
 * @param ultrasound_id Identifier of the ultrasound sensor
 */
void port_ultrasound_stop_echo_timer(uint32_t ultrasound_id);

/**
 * @brief Get the time tick when the init of echo signal was received.
 *
 * @param ultrasound_id Identifier of the ultrasound sensor
 * @return uint32_t Tick value.
 */
uint32_t port_ultrasound_get_echo_init_tick(uint32_t ultrasound_id);

/**
 * @brief Set the time tick when the init of echo signal was received.
 *
 * @param ultrasound_id Identifier of the ultrasound sensor
 * @param echo_init_tick Tick time.
 */
void port_ultrasound_set_echo_init_tick(uint32_t ultrasound_id, uint32_t echo_init_tick);

/**
 * @brief Get the time tick when the end of echo signal was received.
 *
 * @param ultrasound_id Identifier of the ultrasound sensor
 * @return uint32_t Tick value.
 */
uint32_t port_ultrasound_get_echo_end_tick(uint32_t ultrasound_id);

/**
 * @brief Set the time tick when the end of echo signal was received.
 *
 * @param ultrasound_id Identifier of the ultrasound sensor
 * @param echo_end_tick Tick time.
 */
void port_ultrasound_set_echo_end_tick(uint32_t ultrasound_id, uint32_t echo_end_tick);

/**
 * @brief Get the number of overflows of the echo signal timer.
 *
 * @param ultrasound_id Identifier of the ultrasound sensor
 * @return uint32_t Number of overflows.
 */
uint32_t port_ultrasound_get_echo_overflows(uint32_t ultrasound_id);

/**
 * @brief Set the number of overflows of the echo signal timer.
 *
 * @param ultrasound_id Identifier of the ultrasound sensor
 * @param echo_overflows Number of overflows.
 */
void port_ultrasound_set_echo_overflows(uint32_t ultrasound_id, uint32_t echo_overflows);

/**
 * @brief Get the status of the echo signal.
 *
 * @param ultrasound_id Identifier of the ultrasound sensor
 * @return true if echo received, false otherwise.
 */
bool port_ultrasound_get_echo_received(uint32_t ultrasound_id);

/**
 * @brief Set the status of the echo signal.
 *
 * @param ultrasound_id Identifier of the ultrasound sensor
 * @param echo_received Echo signal status.
 */
void port_ultrasound_set_echo_received(uint32_t ultrasound_id, bool echo_received);


/**
 * @brief Start a new measurement of the ultrasound sensor.
 *
 * This function prepares and starts the timers associated with the trigger and echo signals.
 * It also enables the timer that controls the delay between measurements.
 * 
 * @param ultrasound_id Identifier of the ultrasound sensor
 */
void port_ultrasound_start_measurement(uint32_t ultrasound_id);

/**
 * @brief Start the timer that controls the time between new measurements. This function enables the interrupt and activates the timer used to schedule new measurements.
 *
 */
void port_ultrasound_start_new_measurement_timer(void);

/**
 * @brief Stop the timer that controls the time between new measurements.
 *
 * This function disables the timer used to schedule new measurements.
 */
void port_ultrasound_stop_new_measurement_timer(void);

/**
 * @brief Stop all timers related to the ultrasound sensor and reset echo ticks.
 *
 * This function stops the trigger, echo, and new measurement timers.
 * It also resets the echo tick counters of the specified ultrasound sensor.
 *
 * @param ultrasound_id Identifier of the ultrasound sensor
 */
void port_ultrasound_stop_ultrasound(uint32_t ultrasound_id);



#endif /* PORT_ULTRASOUND_H_ */