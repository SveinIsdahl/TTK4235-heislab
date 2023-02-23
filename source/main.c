#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "driver/elevio.h"
#include "order.h"
/*
 *Keep going in the same direction as long as possible and make every necessary stop along the way
 *2 queues, main queue for orders on hold, one for orders currently being exceuted. (Always do above algorithm if main queue gets larger or if currently excecuted queue is done
 * */

enum {
    stopped,    // Stop button pressed
    invalid,    // On startup when starting between floors
    moving_up,  // When moving between floors.
    moving_down,// When moving between floors.
    still_up,   // When on floor, but going to go up
    still_down, // When on floor, but going to go down
    idle        // When waiting for order 
} elev_state;


int main(){

    elevio_init();
    //Move into init-function
    int orderList[N_FLOORS][N_BUTTONS] = {0};
    registerOrder(orderList);
    if(elevio_floorSensor() == -1) {
        elev_state = invalid;
    }
    else {
        elev_state = idle;
    }

    while(1){
        registerOrder(orderList);
        switch (elev_state) {
            case stopped:
                while(elevio_stopButton()){
                   //Clear stuff 
                }
                break;
            case invalid:
                if(elevio_floorSensor() != -1) {
                    elev_state = idle;
                    elevio_motorDirection(DIRN_STOP);

                }
                elevio_motorDirection(DIRN_UP);
                break;
            case moving_up:
                //Check if buttons further up are pressed and stop if they are
                break;
            case moving_down:
                //-||-
                break;
            case still_up:
                //move up after closing doors
                break;
            case still_down:
                //move down after closing doors
                break;
            case idle:
                //wait for order,TODO: also check obstruction
                break;
            default:
                printf("In default");
                break;
        } 



        nanosleep(&(struct timespec){0, 20*1000*1000}, NULL);
    }
    
    
    return 0;
}
