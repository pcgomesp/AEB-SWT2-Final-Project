#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "aeb_actions.h"

void issue_car_cluster_alert(double my_ttc){
    if(my_ttc < 2){
        printf("Placeholder: Send CAN frame to activate alarm in Car Cluster actuator\n");
    } else {
        ;
    }
}