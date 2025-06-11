/**
 * @file interr.c
 * @brief Interrupt service routines for the STM32F4 platform.
 * @author SDG2. Román Cárdenas (r.cardenas@upm.es) and Josué Pagán (j.pagan@upm.es)
 * @date 2025-01-01
 */

 #include <stdio.h>
 
// Include HW dependencies:
#include "stm32f4_system.h"
#include "stm32f4_ultrasound.h"
#include "stm32f4xx.h"

// Include headers of different port elements:
#include "port_system.h"
#include "port_button.h"
#include "port_ultrasound.h"

//------------------------------------------------------
// INTERRUPT SERVICE ROUTINES
//------------------------------------------------------
/**
 * @brief Interrupt service routine for the System tick timer (SysTick).
 *
 * @note This ISR is called when the SysTick timer generates an interrupt.
 * The program flow jumps to this ISR and increments the tick counter by one millisecond.
 *
 * > **TO-DO alumnos:**
 * >
 * > ✅ 1. **Increment the System tick counter `msTicks` in 1 count.** To do so, use the function `port_system_get_millis()` and `port_system_set_millis()`.
 *
 * @warning **The variable `msTicks` must be declared volatile!** Just because it is modified by a call of an ISR, in order to avoid [*race conditions*](https://en.wikipedia.org/wiki/Race_condition). **Added to the definition** after *static*.
 *
 */
void SysTick_Handler(void)
{
    uint32_t currentTicks = port_system_get_millis();  // local variable
    port_system_set_millis(currentTicks + 1);
}

/**
 * @brief Interrupt service routine for the System tick timer (SysTick).
 * 
 * First, this function identifies the line/ pin which has raised the interruption. Then, perform the desired action. Before leaving it cleans the interrupt pending register.
 * 
 */
void EXTI15_10_IRQHandler(void)
{
  /* Resume the SysTick counter after timer/echo interruption (Version 4) */
  port_system_systick_resume();

  /* ISR parking button */
  if (port_button_get_pending_interrupt(PORT_PARKING_BUTTON_ID))
  {
    //printf("interrupt\n");  // For debugging purpose
    if (port_button_get_value(PORT_PARKING_BUTTON_ID))
      port_button_set_pressed(PORT_PARKING_BUTTON_ID, false);
    else
      port_button_set_pressed(PORT_PARKING_BUTTON_ID, true);  // button pressed

    port_button_clear_pending_interrupt(PORT_PARKING_BUTTON_ID);
  }
}

/**
 * @brief TIM3 interrupt handler.
 * This ISR is called when the trigger duration timer (TIM3) expires.
 * It clears the interrupt flag and sets the trigger_end flag.
 */
void TIM3_IRQHandler(void)
{
    // 1. Clear the update interrupt flag (UIF) in the status register (SR)
    TIM3->SR &= ~TIM_SR_UIF;

    // 2. Set the flag indicating that the trigger signal duration has ended
    port_ultrasound_set_trigger_end(PORT_REAR_PARKING_SENSOR_ID, true);
}


/**
 * @brief Interrupt Service Routine (ISR) for TIM2.
 *
 * This timer controls the duration of the echo signal using input capture mode.
 * The interrupt can be caused by:
 *  - An overflow of the timer counter (UIF)
 *  - An input capture event on channel 2 (CC2IF)
 *
 * Actions:
 *  - On overflow: increment the echo_overflows counter.
 *  - On input capture: record either the init or end tick of the echo signal.
 */
void TIM2_IRQHandler(void)
{
    /* Resume the SysTick counter after timer/echo interruption (Version 4) */
    port_system_systick_resume();

    // Check if the update interrupt flag (UIF) is set -> overflow occurred
    if (TIM2->SR & TIM_SR_UIF)
    {
        uint32_t current_overflows = port_ultrasound_get_echo_overflows(PORT_REAR_PARKING_SENSOR_ID);   // Retrieve and increment the number of overflows
        port_ultrasound_set_echo_overflows(PORT_REAR_PARKING_SENSOR_ID, current_overflows + 1);

        TIM2->SR &= ~TIM_SR_UIF;  // Clear the update interrupt flag
    }

    // Check if the input capture interrupt flag for channel 2 (CC2IF) is set
    if (TIM2->SR & TIM_SR_CC2IF)
    {
        uint32_t current_tick = TIM2->CCR2;  // Read the captured value from CCR2 (automatically clears CC2IF)

        uint32_t echo_init = port_ultrasound_get_echo_init_tick(PORT_REAR_PARKING_SENSOR_ID);  // Read current init and end ticks
        uint32_t echo_end = port_ultrasound_get_echo_end_tick(PORT_REAR_PARKING_SENSOR_ID);

        if (echo_init == 0 && echo_end == 0)
        {
            // First capture (rising edge): store the initial tick
            port_ultrasound_set_echo_init_tick(PORT_REAR_PARKING_SENSOR_ID, current_tick);
        }
        else
        {
            // Second capture (falling edge): store the final tick and mark echo as received
            port_ultrasound_set_echo_end_tick(PORT_REAR_PARKING_SENSOR_ID, current_tick);
            port_ultrasound_set_echo_received(PORT_REAR_PARKING_SENSOR_ID, true);
        }
    }
}

/**
 * @brief Interrupt service routine for the TIM5 timer.
 * 
 * This timer controls the timeout between two consecutive ultrasound measurements.
 * When this interrupt occurs, it means that a new measurement can be started.
 */
void TIM5_IRQHandler(void)
{
    TIM5->SR &= ~TIM_SR_UIF;  // 1. Clear the update interrupt flag (UIF)
    port_ultrasound_set_trigger_ready(PORT_REAR_PARKING_SENSOR_ID, true);  // 2. Set the trigger_ready flag to true
}
