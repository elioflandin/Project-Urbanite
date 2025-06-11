/**
 * @file stm32f4_ultrasound.c
 * @brief Portable functions to interact with the ultrasound FSM library. All portable functions must be implemented in this file.
 * @author alumno1
 * @author alumno2
 * @date date
 */

/* Standard C includes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* HW dependent includes */
#include "port_ultrasound.h"
#include "port_system.h"

/* Microcontroller dependent includes */
#include "stm32f4_ultrasound.h"
#include "stm32f4_system.h"

/* Typedefs --------------------------------------------------------------------*/

/**
 * @brief Structure representing the hardware configuration of an ultrasound sensor
 */
typedef struct
{
    GPIO_TypeDef *p_trigger_port;  /*!< GPIO port for the trigger signal */
    GPIO_TypeDef *p_echo_port;     /*!< GPIO port for the echo signal */
    uint8_t trigger_pin;           /*!< Pin number for the trigger signal */
    uint8_t echo_pin;              /*!< Pin number for the echo signal */
    uint8_t echo_alt_fun;          /*!< Alternate function for the echo pin */
    bool trigger_ready;            /*!< Flag indicating if trigger is ready for new measurement */
    bool trigger_end;              /*!< Flag indicating that the trigger pulse is finished */
    bool echo_received;            /*!< Flag indicating that the echo signal has been received */
    uint32_t echo_init_tick;       /*!< Tick at which echo signal started */
    uint32_t echo_end_tick;        /*!< Tick at which echo signal ended */
    uint32_t echo_overflows;       /*!< Number of timer overflows during echo */
} stm32f4_ultrasound_hw_t;


/* Global variables */

/**
 * @brief Array of elements that represents the HW characteristics of the ultrasounds connected to the STM32F4 platform.
 * 
 */
static stm32f4_ultrasound_hw_t ultrasounds_arr[] = {
    [PORT_REAR_PARKING_SENSOR_ID] = {
        .p_trigger_port = STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO,
        .p_echo_port    = STM32F4_REAR_PARKING_SENSOR_ECHO_GPIO,
        .trigger_pin    = STM32F4_REAR_PARKING_SENSOR_TRIGGER_PIN,
        .echo_pin       = STM32F4_REAR_PARKING_SENSOR_ECHO_PIN,
        .echo_alt_fun   = 1,  // Tim2_CH2 is on PA1 (alternate function)
        .trigger_ready  = false,  
        .trigger_end    = false,
        .echo_received  = false,
        .echo_init_tick = 0,
        .echo_end_tick  = 0,
        .echo_overflows = 0
    }
};


/* Private functions ----------------------------------------------------------*/

/**
 * @brief Private function to retrieve the ultrasound struct associated to the given ID.
 * 
 * @param ultrasound_id ID of the ultrasound sensor.
 * @return Pointer to the struct if valid, NULL otherwise.
 */
static stm32f4_ultrasound_hw_t *_stm32f4_ultrasound_get(uint32_t ultrasound_id)
{
    if (ultrasound_id >= sizeof(ultrasounds_arr) / sizeof(ultrasounds_arr[0]))
        return NULL;

    return &ultrasounds_arr[ultrasound_id];
}

/**
 * @brief Configure the timer that controls the duration of the trigger signal.
 * 
 * This function configures the timer to generate internal interrupts to control the raise and fall of the trigger signal. 
 * The duration of the trigger signal is defined in the PORT_PARKING_SENSOR_TRIGGER_UP_US macro. 
 * This function is called by the port_ultrasound_init() public function to configure the timer that controls the duration of the trigger signal.
 * 
 */
static void _timer_trigger_setup(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;  // 1. Enable clock for timer TIM3 (connected to APB1)
    TIM3->CR1 &= ~TIM_CR1_CEN;  // 2. Disable counter
    TIM3->CR1 |= TIM_CR1_ARPE;  // 3. Enable autoload register
    TIM3->CNT = 0;  // 4. Reset counter to 0
    
    TIM3->PSC = 1;  // 5. Load PSC and ARR values
    TIM3->ARR = 80;
    
    // // 5. Compute PSC and ARR for 10 µs using efficient algorithm
    // double sysclk = (double)SystemCoreClock;
    // double duration = 10.0 / 1e6;  // 10 µs in seconds
    // double psc_d = (sysclk * duration) / 65535.0;
    // uint32_t psc = (uint32_t)round(psc_d);
    // if (psc == 0) psc = 1;
    // double arr_d = (sysclk * duration) / psc;
    // uint32_t arr = (uint32_t)round(arr_d);

    // TIM3->PSC = psc;
    // TIM3->ARR = arr;

    TIM3->EGR = TIM_EGR_UG;  // 6. Generate an update event to load precalculation values
    TIM3->SR &= ~TIM_SR_UIF;  // 7. Cleaning the interrupt flag
    TIM3->DIER |= TIM_DIER_UIE;  // 8. Enable interrupt update
    
    NVIC_SetPriority(TIM3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 4, 0));  // 9. Set interrupt priority in NVIC (priority 4, sub-priority 0)
}

/**
 * @brief Configure the timer as input capture to measure the echo signal duration.
 *        This configuration is specific to the ultrasound sensor.
 *
 * @param ultrasound_id Ultrasound sensor ID
 */
static void _timer_echo_setup(uint32_t ultrasound_id)
{
    if (ultrasound_id == PORT_REAR_PARKING_SENSOR_ID)
    {
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;  // 1. Enable TIM2 clock (TIM2 is on APB1)
        TIM2->CR1 &= ~TIM_CR1_CEN;  // 2. Disable counter
        TIM2->CNT = 0; 

        // 3. Set PSC and ARR for 1 µs resolution
        TIM2->PSC = 15;
        TIM2->ARR = 0xFFFF;

        // double sysclk = (double)SystemCoreClock;
        // double tick_us = 1.0 / 1e6;
        // double psc_d = (sysclk * tick_us) / 65535.0;
        // uint32_t psc = (uint32_t)round(psc_d);
        // if (psc == 0) psc = 1;
        // double arr_d = (sysclk * tick_us) / psc;
        // uint32_t arr = (uint32_t)round(arr_d);

        // TIM2->PSC = psc;
        // TIM2->ARR = arr;

        TIM2->CR1 |= TIM_CR1_ARPE;  // 4. Enable auto-reload preload and force register update
        TIM2->EGR |= TIM_EGR_UG;

        TIM2->CCMR1 &= ~TIM_CCMR1_CC2S_Msk;  // 5. Set channel 1 as input mapped to TI1 (TIM_CCMR1_CC2S_Msk = 10)
        TIM2->CCMR1 |= TIM_CCMR1_CC2S_0;

        TIM2->CCMR1 &= ~TIM_CCMR1_IC2F_Msk;  // 6. Disable digital filtering

        TIM2->CCER |= TIM_CCER_CC2P | TIM_CCER_CC2NP;  // 7. Capture both rising (CC2P) and falling (CC2NP) edges with a logic OR (|)

        TIM2->CCMR1 &= ~TIM_CCMR1_IC2PSC_Msk;  // 8. Input capture prescaler = 1 (no division)

        TIM2->CCER |= TIM_CCER_CC2E;  // 9. Enable channel 1 capture

        TIM2->DIER |= TIM_DIER_CC2IE;  // 10. Enable capture interrupt for channel 1

        TIM2->DIER |= TIM_DIER_UIE;  // 11. Enable update interrupt

        NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 0));  // 12. Set priority for TIM2 IRQ (priority 3, sub-priority 0)
    }
}

/**
 * @brief Configure the timer that controls the timeout between consecutive measurements.
 *
 * This timer will periodically generate an interrupt every PORT_PARKING_SENSOR_TIMEOUT_MS milliseconds.
 * It is configured using the efficient algorithm to calculate ARR and PSC values.
 */
static void _timer_new_measurement_setup(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;  // 1. Enable TIM5 clock (TIM5 is on APB1)

    TIM5->CR1 &= ~TIM_CR1_CEN;  // 2. Disable the counter to configure

    TIM5->PSC = 31;  // 3. Set PSC and ARR (100ms)
    TIM5->ARR = 50000;

    // // 3. Efficient algorithm to compute PSC and ARR
    // double timeout_s = (double)PORT_PARKING_SENSOR_TIMEOUT_MS / 1000.0;
    // double sysclk = (double)SystemCoreClock;
    // double max_arr = 65535.0;

    // double psc_double = (sysclk * timeout_s) / max_arr;  // Start with ARR = max, calculate PSC
    // uint32_t psc = (uint32_t)round(psc_double);

    // double arr_double = (sysclk * timeout_s) / (double)(psc);  // Recompute ARR
    // uint32_t arr = (uint32_t)round(arr_double);

    // if (arr > 65535)  // If ARR is too big, adjust PSC and recalculate ARR
    // {
    //     psc += 1;
    //     arr = (uint32_t)round((sysclk * timeout_s) / (double)(psc));
    // }

    // TIM5->PSC = psc;
    // TIM5->ARR = arr;

    TIM5->CR1 |= TIM_CR1_ARPE;  // 4. Enable auto-reload preload and force register update
    TIM5->EGR |= TIM_EGR_UG;

    TIM5->SR &= ~TIM_SR_UIF;  // 5. Clear update interrupt flag

    TIM5->DIER |= TIM_DIER_UIE;  // 6. Enable update interrupt

    NVIC_SetPriority(TIM5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));  // 7. Set interrupt priority (priority 5, sub-priority 0)
}



/* Public functions -----------------------------------------------------------*/
void port_ultrasound_init(uint32_t ultrasound_id)
{
    /* Get the ultrasound sensor */
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound == NULL)
        return;

    /* TO-DO alumnos: */
    /* Initialize ultrasound fields */
    p_ultrasound->echo_init_tick = 0;
    p_ultrasound->echo_end_tick = 0;
    p_ultrasound->echo_overflows = 0;
    p_ultrasound->trigger_ready = true;
    p_ultrasound->trigger_end = false;
    p_ultrasound->echo_received = false;


    /* Trigger pin configuration */
    stm32f4_system_gpio_config(
        p_ultrasound->p_trigger_port,
        p_ultrasound->trigger_pin,
        STM32F4_GPIO_MODE_OUT,
        STM32F4_GPIO_PUPDR_NOPULL
    );

    /* Echo pin configuration */
    stm32f4_system_gpio_config(
        p_ultrasound->p_echo_port,
        p_ultrasound->echo_pin,
        STM32F4_GPIO_MODE_AF,
        STM32F4_GPIO_PUPDR_NOPULL
    );

    /* Assign the correct alternate function to the Echo pin */
    stm32f4_system_gpio_config_alternate(
        p_ultrasound->p_echo_port,
        p_ultrasound->echo_pin,
        p_ultrasound->echo_alt_fun
    );

    /* Configure timers */
    _timer_trigger_setup();
    _timer_echo_setup(ultrasound_id);
    _timer_new_measurement_setup();  
}

void port_ultrasound_stop_trigger_timer(uint32_t ultrasound_id)
{
    // Get the ultrasound sensor
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound == NULL)
        return;

    stm32f4_system_gpio_write(p_ultrasound->p_trigger_port, p_ultrasound->trigger_pin, false);  // 1. Set Trigger pin to low state (logic 0)
    
    // 2. Disable the timer used to control trigger duration
    TIM3->CR1 &= ~TIM_CR1_CEN;  // Bit CEN = 0 → counter disable
}


// Getters and setters functions

/* Getter for trigger_ready flag */
bool port_ultrasound_get_trigger_ready(uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound == NULL)
        return false;

    return p_ultrasound->trigger_ready;
}

/* Setter for trigger_ready flag */
void port_ultrasound_set_trigger_ready(uint32_t ultrasound_id, bool trigger_ready)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound == NULL)
        return;

    p_ultrasound->trigger_ready = trigger_ready;
}

/* Getter for trigger_end flag */
bool port_ultrasound_get_trigger_end(uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound == NULL)
        return false;

    return p_ultrasound->trigger_end;
}

/* Setter for trigger_end flag */
void port_ultrasound_set_trigger_end(uint32_t ultrasound_id, bool trigger_end)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound == NULL)
        return;

    p_ultrasound->trigger_end = trigger_end;
}


// Util
void stm32f4_ultrasound_set_new_trigger_gpio(uint32_t ultrasound_id, GPIO_TypeDef *p_port, uint8_t pin)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->p_trigger_port = p_port;
    p_ultrasound->trigger_pin = pin;
}

void stm32f4_ultrasound_set_new_echo_gpio(uint32_t ultrasound_id, GPIO_TypeDef *p_port, uint8_t pin)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->p_echo_port = p_port;
    p_ultrasound->echo_pin = pin;
}


/**
 * @brief Stop the timer that measures the echo signal. This function stops the timer used for measuring the echo duration and disables the counter to stop the measurement process.
 * 
 * @param ultrasound_id Ultrasound ID. Used to select the correct sensor from the array.
 */
void port_ultrasound_stop_echo_timer(uint32_t ultrasound_id)
{
    if (ultrasound_id == PORT_REAR_PARKING_SENSOR_ID)
    {
        TIM2->CR1 &= ~TIM_CR1_CEN; // Disable echo timer
    }
}


void port_ultrasound_reset_echo_ticks(uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound == NULL)
        return;

    p_ultrasound->echo_init_tick = 0;
    p_ultrasound->echo_end_tick = 0;
    p_ultrasound->echo_overflows = 0;
    p_ultrasound->echo_received = false;
}


uint32_t port_ultrasound_get_echo_init_tick(uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound == NULL)
        return 0;
    return p_ultrasound->echo_init_tick;
}

void port_ultrasound_set_echo_init_tick(uint32_t ultrasound_id, uint32_t echo_init_tick)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound != NULL)
    {
        p_ultrasound->echo_init_tick = echo_init_tick;
    }
}

uint32_t port_ultrasound_get_echo_end_tick(uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound == NULL)
        return 0;
    return p_ultrasound->echo_end_tick;
}

void port_ultrasound_set_echo_end_tick(uint32_t ultrasound_id, uint32_t echo_end_tick)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound != NULL)
    {
        p_ultrasound->echo_end_tick = echo_end_tick;
    }
}

bool port_ultrasound_get_echo_received(uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound == NULL)
        return false;
    return p_ultrasound->echo_received;
}

void port_ultrasound_set_echo_received(uint32_t ultrasound_id, bool echo_received)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound != NULL)
    {
        p_ultrasound->echo_received = echo_received;
    }
}

uint32_t port_ultrasound_get_echo_overflows(uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound == NULL)
        return 0;
    return p_ultrasound->echo_overflows;
}

void port_ultrasound_set_echo_overflows(uint32_t ultrasound_id, uint32_t echo_overflows)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound != NULL)
    {
        p_ultrasound->echo_overflows = echo_overflows;
    }
}

void port_ultrasound_start_measurement(uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    if (p_ultrasound == NULL)
        return;

    // 1. Reset trigger_ready flag
    p_ultrasound->trigger_ready = false;

    // 2. Reset counters of all timers
    
    if (ultrasound_id == PORT_REAR_PARKING_SENSOR_ID)
    {
        TIM2->CNT = 0; // Echo timer
        TIM3->CNT = 0; // Trigger timer
    }
    TIM5->CNT = 0; // New measurement timer

    // 3. Set the trigger pin to high
    stm32f4_system_gpio_write(p_ultrasound->p_trigger_port, p_ultrasound->trigger_pin, true);
    /*
    uint32_t test_trigger = p_ultrasound->p_trigger_port->ODR & (1 << p_ultrasound->trigger_pin);  // Debug
    printf("DEBUG: trigger ODR bit = %lu\n", test_trigger);
    printf("DEBUG: GPIO addr = %p, PIN = %d\n", (void*)p_ultrasound->p_trigger_port, p_ultrasound->trigger_pin);
    uint32_t trig_ready = p_ultrasound->trigger_ready;
    printf("trigger ready = %lu\n", trig_ready);

    uint32_t odr = p_ultrasound->p_trigger_port->ODR;  // Check that we have the right ODR
    printf("ODR: %08lX\n", odr);
    uint32_t moder = p_ultrasound->p_trigger_port->MODER;  // Check that we have the right MODER
    printf("MODER: %08lX\n", moder);
*/
    // 4. Enable interrupts in NVIC
    
    if (ultrasound_id == PORT_REAR_PARKING_SENSOR_ID)
    {
        NVIC_EnableIRQ(TIM2_IRQn); // Echo timer interrupt
        NVIC_EnableIRQ(TIM3_IRQn); // Trigger timer interrupt
    }
    NVIC_EnableIRQ(TIM5_IRQn); // New measurement timer interrupt

    // 5. Enable all timers
    
    if (ultrasound_id == PORT_REAR_PARKING_SENSOR_ID)
    {
        TIM2->CR1 |= TIM_CR1_CEN; // Enable echo timer
        TIM3->CR1 |= TIM_CR1_CEN; // Enable trigger timer
    }
    TIM5->CR1 |= TIM_CR1_CEN; // Enable new measurement timer
}

void port_ultrasound_start_new_measurement_timer(void)
{
    NVIC_EnableIRQ(TIM5_IRQn);       // 1. Enable interrupt in NVIC
    TIM5->CR1 |= TIM_CR1_CEN;        // 2. Enable new measurement timer
}

void port_ultrasound_stop_new_measurement_timer(void)
{
    TIM5->CR1 &= ~TIM_CR1_CEN;       // Disable new measurement timer
}

void port_ultrasound_stop_ultrasound(uint32_t ultrasound_id)
{
    port_ultrasound_stop_trigger_timer(ultrasound_id);
    port_ultrasound_stop_echo_timer(ultrasound_id);
    port_ultrasound_stop_new_measurement_timer();
    port_ultrasound_reset_echo_ticks(ultrasound_id);
}
