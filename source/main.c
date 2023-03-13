#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "order.h"
#include "timer.h"
/*
 *Keep going in the same direction as long as possible and make every necessary
 stop along the way *2 queues, main queue for orders on hold, one for orders
 currently being exceuted. (Always do above algorithm if main queue gets larger
 or if currently excecuted queue is done Remember to clear timer when stop is
 pressed
 * */
static void lights_reset();

int orderList[N_FLOORS][N_BUTTONS] = {0};

// To calculate posisiton after stop and direction
int prevFloor; //Can never be -1
MotorDirection current_dir;
MotorDirection next_dir;
int current_floor;

// int _currentFloor = -1; // Only use setter and getter, values from 0 to
// N_FLOORS*2-1.
//  prevState/currentFloor to handle stop button between floors? With lots of
//  states prevState
// int timer = 0;

enum {
    stopped,    // Stop button pressed
    invalid,    // On startup when starting between floors
    moving,     // When moving between floors.
    open_door,  // After
    idle        // When waiting for order
} elev_state;
int main() {
    // init stuff
    elevio_init();
    lights_reset();
    if (elevio_floorSensor() == -1) {
        elev_state = invalid;
    } else {
        elev_state = idle;
    }

    while (1) {
        // Maybe move registerOrder to differnt states to have more fine grained control
        if (elev_state != invalid) {
            registerOrder(orderList);
        }
        current_floor = elevio_floorSensor();

        switch (elev_state) {
            case stopped:
                printf("stopped\n");
                memset(orderList, 0, sizeof(orderList));
                elevio_stopLamp(1);

                elevio_motorDirection(DIRN_STOP);
                current_dir = DIRN_STOP;
                if (current_floor != -1) {
                    elevio_doorOpenLamp(1);
                    int stop = elevio_stopButton();
                    while (!stop) {
                        stop = elevio_stopButton();
                        printf("stop %d\n", stop);
                    }
                    printf("stop %d\n", stop);
                    elevio_stopLamp(0);
                    elev_state = open_door;
                    break;
                }
                while (elevio_stopButton()) {
                }
                elev_state = idle;
                break;
            case invalid:
                printf("invalid\n");
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
                //printf("moving\n");
                //Continue moving, no need to calculate during movement
                if (current_floor == -1) break;

                //DIRN_STOP => siste ordre eller flere ordre
    //open_door må vite (ved hjelp av current, må next dir introduseres globalt?) om den skal til idle eller moving. 
                //Aldri send til moving uten order, skaper atm inifnte open door loop 
                //TODO: Tror enkelste løsning er å introdusere next_dir globalt ettersom open_door atm ikke kan vite om neste ting som skal gjøres er idle eller moving
                //Maybe add current dir?
                next_dir = order_getDirection(orderList, current_floor, prevFloor, current_dir);

                //Has an order on current floor and current direction is NOT STOP (STOP would indicate a proper stop)
                if (((orderList[current_floor][0]) || (orderList[current_floor][1]) || (orderList[current_floor][2])) && current_dir != DIRN_STOP) {
                    elev_state = open_door;
                    elevio_motorDirection(DIRN_STOP);
                    break;
                }
                
                

                else if(next_dir != DIRN_STOP) {
                    elev_state = moving;
                    if(hasOrdersAbove(orderList, current_floor)) {
                        current_dir = DIRN_UP;
                        elevio_motorDirection(DIRN_UP);
                        break;
                    }
                    if(hasOrdersBelow(orderList, current_floor)) {
                        current_dir = DIRN_DOWN;
                        elevio_motorDirection(DIRN_DOWN);
                        break;
                    }
                }
                printf("Should not be here 2\n");
                break;
            case open_door:
                printf("Open_door\n");
                memset(orderList[current_floor], 0, sizeof orderList[current_floor]);
                printOrders(orderList);
                if(elevio_floorSensor() != 1) {
                    elevio_floorIndicator(elevio_floorSensor());
                }
                elevio_doorOpenLamp(1);
                setTimer();
                int time = checkTimer();
                while (time > 0) {
                    if (elevio_stopButton()) {
                        // Do not turn lamp off here because it should stay on in stopped
                        elev_state = stopped;
                        break;
                    }
                    registerOrder(orderList);
                    if (elevio_obstruction()) {
                        printf("Obstructed\n");
                        time = checkTimer();
                        printf("Before setTimer: %d\n", time);
                        setTimer();
                        time = checkTimer();
                        printf("After setTimer: %d\n", time);
                    }
                    time = checkTimer();
                }
                //Order served, clear doorlamp and button
                elevio_doorOpenLamp(0);
                elevio_buttonLamp(current_floor, 0, 0);
                elevio_buttonLamp(current_floor, 1, 0);
                elevio_buttonLamp(current_floor, 2, 0);
                
                if(!(hasOrdersAbove(orderList, current_floor) || hasOrdersBelow(orderList, current_floor))) {
                    current_dir = DIRN_STOP;
                    elev_state = idle;
                }
                else {
                    elev_state = moving;
                }
                
                break;
            // Do not assume we are on a floor here, could have returned from stop state and need to calculate where we are to get next place to be
            case idle:
                //printf("idle\n");
                if (current_floor != -1) {
                    elevio_buttonLamp(current_floor, BUTTON_HALL_UP, 0);
                    elevio_buttonLamp(current_floor, BUTTON_HALL_DOWN, 0);
                    elevio_buttonLamp(current_floor, BUTTON_CAB, 0);
                    elevio_floorIndicator(current_floor);  // Maybe add -1 check here (FAT minus point for crashes)
                }
               
                if (elevio_stopButton()) {
                    elev_state = stopped;
                    break;
                }

                if (hasActiveOrder(orderList)) {
                    printf("Got order\n");
                    // Elevator has been called on current floor
                    if ((orderList[current_floor][0]) || (orderList[current_floor][1]) || (orderList[current_floor][2])) {
                        printOrders(orderList);
                        printf("Order is on same floor (%d)\n", current_floor);
                        elev_state = open_door;
                        break;
                    }
                    MotorDirection dir = order_getDirection(orderList, current_floor, prevFloor, current_dir);
                    if (dir == DIRN_UP) {
                        elevio_motorDirection(dir);
                        //memset(orderList[current_floor], 0, sizeof orderList[current_floor]);
                        current_dir = DIRN_UP;
                        prevFloor = current_floor;
                        elev_state = moving;
                        break;
                    }
                    if (dir == DIRN_DOWN) {
                        elevio_motorDirection(dir);
                        //memset(orderList[current_floor], 0, sizeof orderList[current_floor]);
                        current_dir = DIRN_DOWN;
                        prevFloor = current_floor;
                        elev_state = moving;
                        break;
                    }
                }
                break;
            default:
                printf("Error, state is default\n");
                break;
        }

        //nanosleep(&(struct timespec){0, 20 * 1000 * 1000}, NULL);
    }

    return 0;
}

// Does not change floorIndicator
static void lights_reset() {
    for (int floor = 0; floor < N_FLOORS; floor++) {
        for (int type = 0; type < N_BUTTONS; type++) {
            elevio_buttonLamp(floor, type, 0);
        }
    }
    elevio_doorOpenLamp(0);
    elevio_stopLamp(0);
}
/*
int orderList[N_FLOORS][N_BUTTONS] = {
    {0, 0, 0},
    {0, 0, 0},
    {0, 1, 0}, //
    {0, 0, 0}
};

//To calculate posisiton after stop
MotorDirection current_dir = DIRN_STOP;
int current_floor = 1;

int main() {
    MotorDirection dir = order_getDirection(orderList, current_floor, current_dir);
    printf("Next dir = %d\n", dir);
}
*/
