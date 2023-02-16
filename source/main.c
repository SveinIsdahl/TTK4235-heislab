#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "driver/elevio.h"
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
typedef struct {
    ButtonType btn;
    int floor;
} order;


int main(){
    elevio_init();

    while(1){


     
        nanosleep(&(struct timespec){0, 20*1000*1000}, NULL);
    }

    return 0;
}
