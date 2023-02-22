#pragma once


#define N_FLOORS 4

typedef enum { 
    DIRN_DOWN   = -1,
    DIRN_STOP   = 0,
    DIRN_UP     = 1
} MotorDirection;


#define N_BUTTONS 3

typedef enum { 
    BUTTON_HALL_UP      = 0,
    BUTTON_HALL_DOWN    = 1,
    BUTTON_CAB          = 2
} ButtonType;


void elevio_init(void);

/**
 *@brief elevio_motorDirection sets the moving direction
 *param[in] motorDirection is up, down or stop
 */
void elevio_motorDirection(MotorDirection dirn);

/**
 *@brief elevio_buttonLamp ligths up button chosen
 *param[in] floor value 0 to 3
 *param[in] ButtonType either UP, DOWN or CAB
 *param[in] value 1 ON 0 OFF
 */
void elevio_buttonLamp(int floor, ButtonType button, int value);
/**
 *@brief elevio_floorIndicator ligths up floor indicator lamp
 *param[in] floor 0 to 3
 */
void elevio_floorIndicator(int floor);
/**
 *@brief elevio_doorOpenLamp ligths up door open lamp
 *param[in] value 1 ON 0 OFF
 */
void elevio_doorOpenLamp(int value);
/**
 *@brief elevio_stopLamp ligths up stop lamp
 *param[in] value 1 ON 0 OFF
 */
void elevio_stopLamp(int value);


/**
 * @brief elevio_callButton checks if button is called 1 ON 0 OFF
 * param[in] floor 0 to 3
 * param[in] ButtonType either UP, DOWN or CAB
 */
int elevio_callButton(int floor, ButtonType button);
/**
 * @brief elevio_floorSensor returns current floor, between 0 and 3 floors
 */
int elevio_floorSensor(void);
/**
 * @brief elevio_stopButton 1 if stop button pushed, else 0
 */
int elevio_stopButton(void);
/**
 *@brief elevio_obstruction 1 if obstructed, else 0
 */
int elevio_obstruction(void);

