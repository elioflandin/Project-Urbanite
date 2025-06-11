/**
 * @file port_display.h
 * @brief Header for the portable functions to interact with the HW of the display system. The functions must be implemented in the platform-specific code.
 * @author alumno1
 * @author alumno2
 * @date fecha
 */
#ifndef PORT_DISPLAY_SYSTEM_H_
#define PORT_DISPLAY_SYSTEM_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>

/* Typedefs --------------------------------------------------------------------*/

/**
 * @brief RGB color structure with 8-bit resolution
 * 
 * Each field ranges from 0 (LED off) to 255 (maximum intensity)
 */
typedef struct {
    uint8_t r;  /*!< Red color value */
    uint8_t g;  /*!< Green color value */
    uint8_t b;  /*!< Blue color value */
} rgb_color_t;

/* Defines and enums ----------------------------------------------------------*/
/* Defines */
#define PORT_REAR_PARKING_DISPLAY_ID 0  /*!< Identifier for the rear parking display */
#define PORT_DISPLAY_RGB_MAX_VALUE 255  /*!< Maximum duty cycle value for RGB LEDs (8 bits) */

/// @brief Predefined RGB color values (based on distance ranges)
#define COLOR_RED        (rgb_color_t){255,   0,   0}   /*!< Danger: [0 - 25] cm */
#define COLOR_YELLOW     (rgb_color_t){ 94,  94,   0}   /*!< Warning: ]25 - 50] cm → 37% of 255 ≈ 94 */
#define COLOR_GREEN      (rgb_color_t){  0, 255,   0}   /*!< Safe: ]50 - 150] cm */
#define COLOR_TURQUOISE  (rgb_color_t){ 26,  89,  82}   /*!< Info: ]150 - 175] cm → 10%, 35%, 32% */
#define COLOR_BLUE       (rgb_color_t){  0,   0, 255}   /*!< OK: ]175 - 200] cm */
#define COLOR_OFF        (rgb_color_t){  0,   0,   0}   /*!< Inactive or out of range (LED off) */

/* Function prototypes and explanation -------------------------------------------------*/

/**
 * @brief Initializes the hardware configuration of the specified RGB display.
 *
 * @param display_id Unique identifier of the RGB LED display.
 */
void port_display_init(uint32_t display_id);

/**
 * @brief Updates the color of the specified RGB display.
 *
 * @param display_id Unique identifier of the RGB LED display.
 * @param color RGB color to set
 */
void port_display_set_rgb(uint32_t display_id, rgb_color_t color);

#endif /* PORT_DISPLAY_SYSTEM_H_ */