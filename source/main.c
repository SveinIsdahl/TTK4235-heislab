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

int orderList[N_FLOORS][N_BUTTONS] = {0};

//To calculate posisiton after stop
int prevFloor;
MotorDirection current_dir;
int current_floor;

// int _currentFloor = -1; // Only use setter and getter, values from 0 to
// N_FLOORS*2-1.
//  prevState/currentFloor to handle stop button between floors? With lots of
//  states prevState
//int timer = 0;

enum {
    stopped,          // Stop button pressed
    invalid,          // On startup when starting between floors
    moving,           // When moving between floors.
    open_door,       // After
    idle              // When waiting for order
} elev_state;
int main() {
    //init stuff
    elevio_init();
    registerOrder(orderList);
    if (elevio_floorSensor() == -1) {
        elev_state = invalid;
    } else {
        elev_state = idle;
    }

    while (1) {
        //Maybe move registerOrder to differnt states to have more fine grained control
        registerOrder(orderList);
        current_floor = elevio_floorSensor();
        
        switch (elev_state) {
            case stopped:
                printf("stopped\n");
                memset(orderList, 0, sizeof(orderList));
                elevio_stopLamp(1);
                
                elevio_motorDirection(DIRN_STOP);
                if (current_floor != -1) {
                        elevio_doorOpenLamp(1);
                        while (elevio_stopButton()) {
                            
                        }
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
                    elev_state = idle;
                    break;
                } else {
                    elevio_motorDirection(DIRN_UP);
                    break;
                }
            case moving:
                printf("moving\n");
                if (current_floor == -1) break;
                //Because movign state, order_idle_getDir is assumed to be called with currentDir
                MotorDirection next_dir = order_idle_getDirection(orderList, current_floor, current_dir);
                if(next_dir == DIRN_STOP) {
                    elev_state = open_door;
                    break;
                }
                current_dir = next_dir;
                break;
            case open_door:
                elevio_doorOpenLamp(1);
                elevio_floorIndicator(current_floor); //Maybe add -1 check here (FAT minus point for crashes)
                setTimer();
                while(checkTimer() > 0) {
                    if(elevio_stopButton()) {
                        //Do not turn lamp off here because it should stay on in stopped
                        elev_state = stopped;
                        break;
                    }
                    registerOrder(orderList);
                    if(elevio_obstruction()) {
                        break;
                    }
                }
                elevio_doorOpenLamp(0);
                elev_state = idle;
                break;
            //Do not assume we are on a floor here, could have returned from stop state and need to calculate where we are to get next place to be
            case idle:
                printf("idle\n");
                elevio_buttonLamp(current_floor, BUTTON_HALL_UP, 0);
                elevio_buttonLamp(current_floor, BUTTON_HALL_DOWN, 0);
                elevio_buttonLamp(current_floor, BUTTON_CAB, 0);

                if (elevio_stopButton()) {
                    elev_state = stopped;
                    break;
                }
                
                if (hasActiveOrder(orderList)) {
                    //Elevator has been called on current floor
                    if ((orderList[current_floor][0] == current_floor) || (orderList[current_floor][1] == current_floor) || (orderList[current_floor][2] == current_floor)) {
                        elev_state  = open_door;
                        break;
                    }
                    MotorDirection dir = order_idle_getDirection(orderList, current_floor, current_dir);
                    if (dir == DIRN_UP) {
                        elevio_motorDirection(dir);
                        memset(orderList[current_floor], 0, sizeof orderList[current_floor]);
                        current_dir = DIRN_UP;
                        elev_state = moving;
                        break;
                    }
                    if (dir == DIRN_DOWN) {
                        elevio_motorDirection(dir);
                        memset(orderList[current_floor], 0, sizeof orderList[current_floor]);
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

        //nanosleep(&(struct timespec){0, 20 * 1000 * 1000}, NULL);
    }

    return 0;
}
