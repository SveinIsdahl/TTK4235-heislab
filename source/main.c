#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "order.h"
#include "timer.h"

static void lights_reset();
static void lights_resetFloor(int floor);
int orderList[N_FLOORS][N_BUTTONS] = {0};

// To calculate posisiton after stop and direction
int prev_floor;  // Can never be -1
int current_floor;
MotorDirection current_dir;

enum {
    stopped,    // Stop button pressed
    invalid,    // On startup when starting between floors
    moving,     // When moving between floors.
    open_door,  // After
    idle        // When waiting for order
} elev_state;

int main() {
    elevio_init();
    lights_reset();
    if (elevio_floorSensor() == -1) {
        elev_state = invalid;
    } else {
        elev_state = idle;
    }

    while (1) {
        if (elev_state != invalid) {
            order_register(orderList);
        }
        current_floor = elevio_floorSensor();

        switch (elev_state) {
            case stopped:
                memset(orderList, 0, sizeof(orderList));  // Need to double check this works, simulator not good for testing
                elevio_stopLamp(1);

                elevio_motorDirection(DIRN_STOP);
                //If this is needed, uncomment but set global variable prev_dir == current_dir 
                //current_dir = DIRN_STOP;
                if (current_floor != -1) {
                    elevio_doorOpenLamp(1);
                    int stop = elevio_stopButton();
                    while (stop) {
                        stop = elevio_stopButton();
                    }
                    elevio_stopLamp(0);
                    elev_state = open_door;
                    break;
                }
                while (elevio_stopButton()) {
                }
                elevio_stopLamp(0);
                elev_state = idle;
                break;
            case invalid:
                if (elevio_stopButton()) {
                    elev_state = stopped;
                    break;
                }
                if (current_floor != -1) {
                    elevio_motorDirection(DIRN_STOP);
                    current_dir = DIRN_STOP;
                    elev_state = idle;
                    break;
                } else {
                    elevio_motorDirection(DIRN_UP);
                    current_dir = DIRN_UP;
                    break;
                }
            case moving:
                if (elevio_stopButton()) {
                    elev_state = stopped;
                    break;
                }
                if (current_floor == -1) {
                    break;
                } else {
                    prev_floor = current_floor;
                    
                    //elevio_motorDirection(DIRN_STOP); This can be used to slow down at every floor to prevent skipping, should only be needed with bad HW
                }

                // Has an order on current floor and current direction is NOT STOP (STOP would indicate a proper stop)
                // Stop if button in moving dir is pressed or cab
                if ((current_dir == DIRN_UP && orderList[current_floor][BUTTON_HALL_UP]) || (current_dir == DIRN_DOWN && orderList[current_floor][BUTTON_HALL_DOWN]) || (orderList[current_floor][BUTTON_CAB])) {
                    elev_state = open_door;
                    elevio_motorDirection(DIRN_STOP);
                    break;
                }
                // When we hit last order in current moving direction
                // Should this be handled by idle?
                //elev_state = moving;
                if (order_hasOrdersAbove(orderList, current_floor)) {
                    current_dir = DIRN_UP;
                    elevio_motorDirection(DIRN_UP);
                    break;
                }
                if (order_hasOrdersBelow(orderList, current_floor)) {
                    current_dir = DIRN_DOWN;
                    elevio_motorDirection(DIRN_DOWN);
                    break;
                }
                // Endestopp
                if ((current_floor == 0 || current_floor == (N_FLOORS - 1))) {
                    elevio_motorDirection(DIRN_STOP);
                    elev_state = idle;
                    break;
                }
                break;
            case open_door:
                order_clearFloorOrders(orderList, current_floor, current_dir);
                elevio_floorIndicator(current_floor);
                elevio_doorOpenLamp(1);
                timer_set();
                int time = timer_check();
                while (time > 0) {
                    if (elevio_stopButton()) {
                        // Do not turn lamp off here because it should stay on in stopped
                        elev_state = stopped;
                        break;
                    }
                    if (!elevio_obstruction()) {
                        timer_set();
                    }
                    order_register(orderList);
                    //Should not be needed to check, but better to not have UB 
                    order_clearFloorOrders(orderList, current_floor, current_dir);
                    time = timer_check();
                }
                // Order served, clear doorlamp and buttons
                elevio_doorOpenLamp(0);

                // Go to idle if not orders above or below
                if (!(order_hasOrdersAbove(orderList, current_floor) || order_hasOrdersBelow(orderList, current_floor))) {
                    current_dir = DIRN_STOP;
                    elev_state = idle;
                } else {
                    elev_state = moving;
                }
                break;
            case idle:
                //This means we were in stopped state
                if (current_floor == -1) {
                    MotorDirection next_dir = order_getDirectionAfterStop(orderList, prev_floor, current_dir);
                    //This is set here and not earlier because current_dir is used in above function to calculate position, so can't be DIRN_STOP

                    //No orders
                    if(next_dir == DIRN_STOP) {
                        break;
                    }
                    else {
                        current_dir = next_dir;
                        elev_state = moving;
                        elevio_motorDirection(next_dir);
                        break;
                    }
                }
                lights_resetFloor(current_floor);
                elevio_floorIndicator(current_floor);
                if (elevio_stopButton()) {
                    elev_state = stopped;
                    break;
                }

                if (order_hasActiveOrders(orderList)) {
                    // Elevator has been called on current floor
                    if (((orderList[current_floor][0]) || (orderList[current_floor][1]) || (orderList[current_floor][2])) && current_floor != -1) {
                        elev_state = open_door;
                        break;
                    }
                    MotorDirection dir = order_getDirection(orderList, current_floor, prev_floor, current_dir);
                    if (dir == DIRN_UP) {
                        elevio_motorDirection(dir);
                        current_dir = DIRN_UP;
                        elev_state = moving;
                        break;
                    }
                    if (dir == DIRN_DOWN) {
                        elevio_motorDirection(dir);
                        current_dir = DIRN_DOWN;
                        elev_state = moving;
                        break;
                    }
                }
                break;
            default:
                printf("Error, state is default\n");
                break;
        }
        // nanosleep(&(struct timespec){0, 20 * 1000 * 1000}, NULL);
    }

    return 0;
}

static void lights_reset() {
    for (int floor = 0; floor < N_FLOORS; floor++) {
        for (int type = 0; type < N_BUTTONS; type++) {
            elevio_buttonLamp(floor, type, 0);
        }
    }
    elevio_doorOpenLamp(0);
    elevio_stopLamp(0);
}
static void lights_resetFloor(int floor) {
    elevio_buttonLamp(floor, BUTTON_HALL_UP, 0);
    elevio_buttonLamp(floor, BUTTON_HALL_DOWN, 0);
    elevio_buttonLamp(floor, BUTTON_CAB, 0);
}
// TODO: Check if buttons pressed are same as moving direction
// Maybe bug when floor one has temp stop, light does not turn on