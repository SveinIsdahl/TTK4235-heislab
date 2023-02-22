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

enum elev_state {
    invalid, // On startup when starting between floors
    moving,  // When moving between floors
    still,   // When still
    idle     // When waiting for order 
};

/*All functions related to a struct, takes in a pointer to struct as first argument*/
void add_order(order* ord){
}


int main(){

    elevio_init();

    int emptyList[4][3] = { 0 };
    /*order check1 = {BUTTON_HALL_UP, 0};
    order check2 = { BUTTON_HALL_DOWN, 2};
    */
    //printf(check1);
    //printf(check2);

    while(1){
        registerOrder(emptyList);

        /*        
        if(emptyList[0][0] == check1){
            printf("knapp hall up i etasje 0 ble trykket");
        }

        if(emptyList[2][1] == check2){
            printf("knapp hall down i etasje 3 ble trykket");
        }
        */
        //nanosleep(&(struct timespec){0, 20*1000*1000}, NULL);
    }
    
    
    return 0;
}
