#pragma once
#include "driver/elevio.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    ButtonType btn;
    int floor;
} order;
void order_print(int orders[N_FLOORS][N_BUTTONS]);
void order_register(int orders[N_FLOORS][N_BUTTONS]);
int order_hasActiveOrders(int orders[N_FLOORS][N_BUTTONS]);
MotorDirection order_getDirection(int orders[N_FLOORS][N_BUTTONS], int current_floor, int prev_floor, MotorDirection current_dir);
int order_hasOrdersAbove(int orders[N_FLOORS][N_BUTTONS], int current_floor);
int order_hasOrdersBelow(int orders[N_FLOORS][N_BUTTONS], int current_floor);