#include "driver/elevio.h"

void printOrders(int orderList[4][3]);

typedef struct {
    ButtonType btn;
    int floor;
} order;

void registerOrder(int orderList[4][3]);