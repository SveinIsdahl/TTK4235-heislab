#include "order.h"
#include <stdio.h>

void printOrders(int orderList[4][3]){
    printf("HALL_UP HALL_DOWN CAB \n");
    for ( int i = 0; i < 4; i++){
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

void registerOrder(int orderList[4][3]){

    for ( int i = 0; i < 4; i++){
        for ( int k = 0 ; k < 3; k++ ){
            ButtonType calledButton = (ButtonType)k;
            if ( elevio_callButton(i, calledButton)){
                orderList[i][k] = 1;
                printOrders(orderList);
            }
        }
    }
}

