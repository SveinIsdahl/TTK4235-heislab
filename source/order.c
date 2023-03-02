#include "order.h"
#include "driver/elevio.h"
#include <stdio.h>

void printOrders(int orderList[4][3]){
    printf("HALL_UP HALL_DOWN CAB \n");
    for (int i = 0; i < 4; i++){
        if (i == 0){
            printf("1: ");
        } else if (i == 1){
            printf("2: ");
        } else if (i == 2){
            printf("3: ");
        } else if (i == 3){
            printf("4: ");
        }

        for ( int k = 0 ; k < 3; k++ ){
            if (orderList[i][k] == 0){
                printf("0");
            } else if (orderList[i][k] == 1){
                printf("1");
            }
        }
        printf("\n");
    }
}

//Maybe change to a button function instead of order function to include stop and obstruction?
void registerOrder(int orderList[N_FLOORS][N_BUTTONS]){
    for (int floor = 0; floor < N_FLOORS; floor++){//floor 0-3
        for (int type = 0 ; type < N_BUTTONS; type++ ){//hall_up,hall_down,cab
            ButtonType calledButton = (ButtonType)type;
            int isPressed = elevio_callButton(floor, calledButton);
            orderList[floor][type] = orderList[floor][type] || isPressed;
            elevio_buttonLamp(floor, type, orderList[floor][type]);
        }
    }
//    printOrders(orderList);
}

//Only called in idle currently, therefore should check if we have multiple buttons pressed an give closest
//Currently ignores if orderList uses stop or obstruction
