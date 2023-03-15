#include "order.h"

void order_print(int orders[N_FLOORS][N_BUTTONS]) {
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
void order_register(int orders[N_FLOORS][N_BUTTONS]) {
    for (int floor = 0; floor < N_FLOORS; floor++) {    // floor 0-3
        for (int type = 0; type < N_BUTTONS; type++) {  // hall_up,hall_down,cab
            ButtonType calledButton = (ButtonType)type;
            int isPressed = elevio_callButton(floor, calledButton);
            orders[floor][type] = orders[floor][type] || isPressed;
            elevio_buttonLamp(floor, type, isPressed);
        }
    }
    //    order_print(orders);
}

// Return 0 or 1
int order_hasActiveOrders(int orders[N_FLOORS][N_BUTTONS]) {
    for (int floor = 0; floor < N_FLOORS; floor++) {    // floor 0-3
        for (int type = 0; type < N_BUTTONS; type++) {  // hall_up,hall_down,cab
            int isPressed = orders[floor][type];
            if (isPressed) return 1;
        }
    }
    return 0;
}
int order_hasOrdersAbove(int orders[N_FLOORS][N_BUTTONS], int current_floor) {
    int ordersAbove = 0;
    for (int i = current_floor + 1; i < N_FLOORS; i++) {
        if (orders[i][0] || orders[i][1] || orders[i][2]) {
            ordersAbove = 1;
        }
    }
    return ordersAbove;
}
int order_hasOrdersBelow(int orders[N_FLOORS][N_BUTTONS], int current_floor) {
    int orderBelow = 0;
    for (int i = current_floor - 1; i >= 0; i--) {
        if (orders[i][0] || orders[i][1] || orders[i][2]) {
            orderBelow = 1;
        }
    }
    return orderBelow;
}
MotorDirection order_getDirection(int orders[N_FLOORS][N_BUTTONS], int current_floor, int prev_floor, MotorDirection current_dir) {
    // No current direction => get closest order and start serving that
    if (current_dir == DIRN_STOP) {
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
        if (closest_floor - current_floor < 0) {
            return DIRN_DOWN;
        }
        printf("Shold not be here\n");
        return DIRN_STOP;
    }

    int ordersAbove = order_hasOrdersAbove(orders, current_floor);
    int ordersBelow = order_hasOrdersBelow(orders, current_floor);

    if (ordersAbove) printf("Has orders above \n");
    if (ordersBelow) printf("Has orders below \n");
    printf("Dir: %d\n", current_dir);
    order_print(orders);
    //Continue in same direction
    if (current_dir == DIRN_UP) {
        if (ordersAbove) {
            return DIRN_UP;
        } else if (ordersBelow) {
            return DIRN_DOWN;
        } else {
            return DIRN_STOP;
        }
    }
    if (current_dir == DIRN_DOWN) {
        if (ordersBelow) {
            return DIRN_UP;
        } else if (ordersAbove) {
            return DIRN_DOWN;
        } else {
            return DIRN_STOP;
        }
    }

    printf("Should not be here 1\n");
    return DIRN_STOP;
}
//Currently assumes only one order in orders (e.g gets closest)
MotorDirection order_getDirectionAfterStop(int orders[N_FLOORS][N_BUTTONS], int prev_floor, MotorDirection dir) {
    if(!order_hasActiveOrders(orders)) {
        return DIRN_STOP;
    }

    int next_floor;
    if(dir == DIRN_DOWN) {
        next_floor = prev_floor - 1;
        if(order_hasOrdersAbove(orders, next_floor)) {
            return DIRN_UP;
        }
        else if(order_hasOrdersBelow(orders, prev_floor)) {
            return DIRN_DOWN;
        }
        else {
            printf("Stopped error1\n");
        }
    }
    else if(dir == DIRN_UP) {
        next_floor = prev_floor + 1;
        if(order_hasOrdersAbove(orders, prev_floor)) {
            return DIRN_UP;
        }
        else if(order_hasOrdersBelow(orders, next_floor)) {
            return DIRN_DOWN;
        }
        else {
            printf("Stopped error2\n");
        }
    }
    return DIRN_STOP; //Should not reach here
}