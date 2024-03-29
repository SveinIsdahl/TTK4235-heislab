#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "order.h"
#include "timer.h"

static void lights_reset();
int order_list[N_FLOORS][N_BUTTONS] = {0};

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
            order_register(order_list);
        }
        current_floor = elevio_floorSensor();

        switch (elev_state) {
            case stopped:
                memset(order_list, 0, sizeof(order_list));
                elevio_stopLamp(1);
                elevio_motorDirection(DIRN_STOP);
                
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
                }
                
                // Pri: high beacuse we always want to pick up/let people of in moving direction
                // Stop if button in moving dir is pressed or cab is pressed
                if ((current_dir == DIRN_UP && order_list[current_floor][BUTTON_HALL_UP]) || (current_dir == DIRN_DOWN && order_list[current_floor][BUTTON_HALL_DOWN]) || (order_list[current_floor][BUTTON_CAB])) {
                    elev_state = open_door;
                    //This clearing is the reason we remove both light when they are pressed
                    //Because one elevator is used, everyone has to go on anyways
                    order_clearFloorOrders(order_list, current_floor, DIRN_STOP);
                    elevio_motorDirection(DIRN_STOP);
                    break;
                }

                if (current_dir == DIRN_UP && (!order_hasOrdersAbove(order_list, current_floor)) && order_hasOrder(order_list, current_floor)) {
                    order_clearFloorOrders(order_list, current_floor, DIRN_STOP);
                    elev_state = open_door;
                    elevio_motorDirection(DIRN_STOP);
                    break;
                }
                if (current_dir == DIRN_DOWN && (!order_hasOrdersBelow(order_list, current_floor)) && order_hasOrder(order_list, current_floor)) {
                    order_clearFloorOrders(order_list, current_floor, DIRN_STOP);
                    elev_state = open_door;
                    elevio_motorDirection(DIRN_STOP);
                    break;
                }

                if (order_hasOrdersAbove(order_list, current_floor) && current_dir == DIRN_UP) {
                    current_dir = DIRN_UP;
                    elevio_motorDirection(DIRN_UP);
                    break;
                } else if (order_hasOrdersBelow(order_list, current_floor) && current_dir == DIRN_DOWN) {
                    current_dir = DIRN_DOWN;
                    elevio_motorDirection(DIRN_DOWN);
                    break;
                }
                
                // Pri: low, standard elevator stuff, used to determine next direction if no special cases,
                if (order_hasOrdersAbove(order_list, current_floor)) {
                    current_dir = DIRN_UP;
                    elevio_motorDirection(DIRN_UP);
                    break;
                } else if (order_hasOrdersBelow(order_list, current_floor)) {
                    current_dir = DIRN_DOWN;
                    elevio_motorDirection(DIRN_DOWN);
                    break;
                }

                // Pri: low because going to idle means we recalculate priority, not problem if we only have one way to go
                if ((current_floor == 0 || current_floor == (N_FLOORS - 1))) {
                    elevio_motorDirection(DIRN_STOP);
                    order_clearFloorOrders(order_list, current_floor, DIRN_STOP);
                    elev_state = idle;
                    break;
                }

                break;
            case open_door:
                if(current_floor == -1) {
                    elev_state = moving;
                    break;
                }
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
                    order_register(order_list);
                    time = timer_check();
                }
                order_clearFloorOrders(order_list, current_floor, current_dir);
                elevio_doorOpenLamp(0);
                // Go to idle if not orders above or below
                if (!(order_hasOrdersAbove(order_list, current_floor) || order_hasOrdersBelow(order_list, current_floor))) {
                    current_dir = DIRN_STOP;
                    elev_state = idle;
                } else {
                    elev_state = moving;
                }
                break;
            case idle:
                // This means we were in stopped state
                if (current_floor == -1) {
                    MotorDirection next_dir = order_getDirectionAfterStop(order_list, prev_floor, current_dir);
                    // This is set here and not earlier because current_dir is used in above function to calculate position, so can't be DIRN_STOP
                    if (next_dir == DIRN_STOP) {
                        break;
                    } else {
                        current_dir = next_dir;
                        elev_state = moving;
                        elevio_motorDirection(next_dir);
                        break;
                    }
                }
                elevio_floorIndicator(current_floor);
                if (elevio_stopButton()) {
                    elev_state = stopped;
                    break;
                }

                if (order_hasActiveOrders(order_list)) {
                    // Elevator has been called on current floor
                    if (order_hasOrder(order_list, current_floor) && current_floor != -1) {
                        elev_state = open_door;
                        break;
                    }
                    elev_state = moving;
                }
                break;
            default:
                printf("Error, state is default\n");
                break;
        }
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
