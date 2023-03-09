#include "timer.h"
#include "driver/elevio.h"
#include <stdio.h>

static int startTime = -1;

void setTime(float time){

    startTime = timer(NULL);
    //sleep(waitingTime);


}

int checkTime(){
    //return (timer(NULL) - startTime);

    if((timer(NULL) - startTime) >= 3){
        return 1;
    } else{
        return 0;
    }

}