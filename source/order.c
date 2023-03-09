#include "order.h"

void printOrders(int orders[N_FLOORS][N_BUTTONS]) {
    printf("HALL_UP HALL_DOWN CAB \n");
    for (int i = 0; i < 4; i++) {
        if (i == 0) {
            printf("1: ");
        } else if (i == 1) {
            printf("2: ");
        } else if (i == 2) {
            printf("3: ");
        } else if (i == 3) {
            printf("4: ");
        }

        for (int k = 0; k < 3; k++) {
            if (orders[i][k] == 0) {
                printf("0");
            } else if (orders[i][k] == 1) {
                printf("1");
            }
        }
        printf("\n");
    }
}

// Maybe change to a button function instead of order function to include stop and obstruction?
void registerOrder(int orders[N_FLOORS][N_BUTTONS]) {
    for (int floor = 0; floor < N_FLOORS; floor++) {    // floor 0-3
        for (int type = 0; type < N_BUTTONS; type++) {  // hall_up,hall_down,cab
            ButtonType calledButton = (ButtonType)type;
            int isPressed = elevio_callButton(floor, calledButton);
            orders[floor][type] = orders[floor][type] || isPressed;
            elevio_buttonLamp(floor, type, orders[floor][type]);
        }
    }
    //    printOrders(orders);
}

// Return 0 or 1
int hasActiveOrder(int orders[N_FLOORS][N_BUTTONS]) {
    for (int floor = 0; floor < N_FLOORS; floor++) {    // floor 0-3
        for (int type = 0; type < N_BUTTONS; type++) {  // hall_up,hall_down,cab
            ButtonType calledButton = (ButtonType)type;
            int isPressed = elevio_callButton(floor, calledButton);
            if (isPressed) return 1;
        }
    }
    return 0;
}
MotorDirection order_idle_getDirection(int orders[N_FLOORS][N_BUTTONS], int current_floor) {
    if (current_floor == 3) {
        return DIRN_DOWN;
    }
    if (current_floor == 0) {
        return DIRN_UP;
    }

    // Incase multiple order at the same time
    int closest_floor = 0;
    int diff = 100;
    for (int i = 0; i < N_FLOORS; i++) {
        if (orders[i][0] || orders[i][1] || orders[i][2]) {
            if (abs(closest_floor - current_floor) < diff) {
                diff = abs(closest_floor - current_floor);
                closest_floor = i;
            }
        }
    }
    if (closest_floor - current_floor > 0) {
        return DIRN_UP;
    }
    if (closest_floor - current_floor > 0) {
        return DIRN_DOWN;
    }
    return DIRN_STOP;
}

