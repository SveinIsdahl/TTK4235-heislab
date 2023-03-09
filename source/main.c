#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include "driver/elevio.h"
#include "order.h"
/*
 *Keep going in the same direction as long as possible and make every necessary stop along the way
 *2 queues, main queue for orders on hold, one for orders currently being exceuted. (Always do above algorithm if main queue gets larger or if currently excecuted queue is done
 Remember to clear timer when stop is pressed
 * */

int orderList[N_FLOORS][N_BUTTONS] = {0};
// int _currentFloor = -1; // Only use setter and getter, values from 0 to N_FLOORS*2-1.
//  prevState/currentFloor to handle stop button between floors? With lots of states prevState
int timer = 0;
int stopped_btn = 0;
enum
{
    stopped,         // Stop button pressed
    invalid,         // On startup when starting between floors
    moving_up,       // When moving between floors.
    moving_down,     // When moving between floors.
    stationary_up,   // When on floor, but going to go up
    stationary_down, // When on floor, but going to go down
    idle             // When waiting for order
} elev_state;
int main()
{

    elevio_init();
    // Global, should be mutexes or maybe g_
    // Move into init-function
    // Array for all floors and between floors
    registerOrder(orderList);
    if (elevio_floorSensor() == -1)
    {
        elev_state = invalid;
    }
    else
    {
        elev_state = idle;
    }
    int floor;
    while (1)
    {
        registerOrder(orderList);
        floor = elevio_floorSensor();
        stopped_btn = elevio_stopButton();
        // Add orderList to
        switch (elev_state)
        {
        case stopped:
            printf("stopped\n");
            memset(orderList, 0, sizeof(orderList));
            elevio_stopLamp(1);
            elevio_motorDirection(DIRN_STOP);
            while (elevio_stopButton())
            {
                if (floor != -1)
                {
                    // open door
                    // setTimer(3);
                    // Time delta for 3 sec, move delta if setTimer is called again
                    // Poll timer?
                }
            }
            elevio_stopLamp(0);
            elev_state = idle;
            break;
        case invalid:
            printf("invalid\n");
            if (stopped_btn)
            {
                elev_state = stopped;
                break;
            }
            if (floor != -1)
            {
                elevio_motorDirection(DIRN_STOP);
                elev_state = idle;
                break;
            }
            else
            {
                elevio_motorDirection(DIRN_UP);
                break;
            }
        case moving_up:
            printf("moving_up\n");
            if (floor == -1)
                break;
            if (orderList[floor][0] || orderList[floor][1] || orderList[floor][2])
            {
                elevio_motorDirection(DIRN_STOP);
                elev_state = stationary_up;
            }
            break;
        case moving_down:
            printf("moving_down\n");
            if (floor == -1)
                break;
            if (orderList[floor][0] || orderList[floor][1] || orderList[floor][2])
            {
                elevio_motorDirection(DIRN_STOP);
                elev_state = stationary_down;
            }
            break;
        case stationary_up:
            printf("stationary up\n");
            memset(orderList[floor], 0, sizeof orderList[floor]);
            while (timer != 0)
            {
                elevio_doorOpenLamp(floor);
                if (elevio_stopButton())
                {
                    elev_state = stopped;
                    break;
                }
            }
            elev_state = moving_up;
            break;
        case stationary_down:
            printf("stationary_down");
            memset(orderList[floor], 0, sizeof orderList[floor]);
            while (timer != 0)
            {
                elevio_doorOpenLamp(floor);
                if (elevio_stopButton())
                {
                    elev_state = stopped;
                    break;
                }
            }
            elev_state = moving_down;
            break;
        case idle:
            printf("idle\n");
            if (stopped_btn)
            {
                elev_state = stopped;
                break;
            }
            if (elevio_obstruction())
            {
                // timer stuff
            }
            if (hasActiveOrder())
            {
            }

            break;
        default:
            printf("In default\n");
            break;
        }

        nanosleep(&(struct timespec){0, 20 * 1000 * 1000}, NULL);
    }

    return 0;
}
