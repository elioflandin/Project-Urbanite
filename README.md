# Urbanite parking aid system

## Author

* **Elio Flandin** - email: [elio.flandin@ensea.fr](mailto:alumno@alumno.es)

## Urbanite Project

The Urbanite project implements a modular parking assistance system using state machines (FSMs) on STM32 microcontrollers.
The system is composed of independent FSMs for handling the button, ultrasound sensor, and RGB display, as well as an integrated FSM called fsm_urbanite which coordinates all components.

The folder docs/ contains full Doxygen-generated documentation and can bee consulted at https://elioflandin.github.io/Project-Urbanite/.

## Architecture

Each subsystem (button, ultrasound, display) is implemented using its own FSM for clarity, reusability, and testability.

The FSMs are initialized dynamically using malloc, and their logic is centralized via `fsm_fire()` calls inside the `while(1)` loop in `main.c`.

### Button FSM

Implements anti-debounce logic using a configurable debounce time.

Stores the duration of button presses for interpreting user intent.

Long presses toggle the system ON/OFF.

Short presses pause or resume the display output.

### Ultrasound FSM

Triggers distance measurements using a transducer and processes the received echo.

Produces new measurement data when available, communicated to the main FSM.

### Display FSM

Displays color-coded distances using an RGB LED: 

* LED does not light up at more than 200 cm,
* Blue for `safe distance` (between 200 and 175 cm), 
* Turquoise blue for `INFO` (between 175 and 150 cm), 
* Green for `NO PROBLEM` (between 150 and 50 cm), 
* Yellow for `WARNING` (between 50 and 25 cm),
* Red for `DANGER` (<25 cm).

Reacts to pause commands and can turn off or freeze based on user input (button).

### Urbanite Main FSM

Manages overall behavior of the system including ON/OFF state, pause functionality, and transitions to/from low-power mode.

Integrates all subsystems via pointer-based references.

Handles activity detection across subsystems to determine if the system should enter sleep mode.

Uses a transition table for clarity and maintainability.

### Low Power Mode Support

Entered through specific states in the main FSM (e.g., `SLEEP_WHILE_ON`, `SLEEP_WHILE_OFF`).

Implements `__WFI()` to wait for interrupts and reduce power usage.

Easily testable via debugger or current measurement.

### Integration and Testing

The FSM was thoroughly tested with a real circuit using a Nucleo STM32F4re board.

Full integration was achieved in `main.c`, with all subsystems interacting correctly.

Proper hardware abstraction was maintained using port_* functions.

Extensive logging and LED display behavior were verified in paused and active states.

A local Doxygen documentation was generated in the `docs/` folder, and GitHub Pages was configured to allow online access to the full API documentation.


#### Final photo of the parking aid system between 200 and 175 cm. LED lights up blue:

\image html \docs\assets\imgs\BLUE.jpg

#### Between 175 and 150 cm, the LED lights up turquoise blue :

\image html \docs\assets\imgs\TURQUOISE_BLUE.jpg

#### Between 150 and 50 cm, the LED lights up green :

\image html \docs\assets\imgs\GREEN.jpg

#### Between 50 and 25 cm, the LED lights up yellow :

\image html \docs\assets\imgs\YELLOW.jpg

#### At less than 25 cm, the LED lights up red :

\image html \docs\assets\imgs\RED.jpg

#### Final photo of the system in pause mode, nothing lights up, unless the detector is less than 25 cm away :

\image html \docs\assets\imgs\PAUSE.jpg

#### If the sensor is too close to the obstacle, even in pause mode :

\image html \docs\assets\imgs\RED_PAUSE.jpg


