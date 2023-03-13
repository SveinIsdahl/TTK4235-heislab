#pragma once
#include "driver/elevio.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    ButtonType btn;
    int floor;
} order;
void printOrders(int orders[N_FLOORS][N_BUTTONS]);
void registerOrder(int orders[N_FLOORS][N_BUTTONS]);
int hasActiveOrder(int orders[N_FLOORS][N_BUTTONS]);
MotorDirection order_getDirection(int orders[N_FLOORS][N_BUTTONS], int current_floor, int prev_floor, MotorDirection current_dir);
void order_ordersToFloorMap(int *order_list[N_FLOORS], int orders[N_FLOORS][N_BUTTONS]);
int hasOrdersAbove(int orders[N_FLOORS][N_BUTTONS], int current_floor);
int hasOrdersBelow(int orders[N_FLOORS][N_BUTTONS], int current_floor);