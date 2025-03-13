#include <stdio.h>
#include <unistd.h>
#include <mqueue.h>
#include "mq_utils.h"
#include "constants.h"
#include "actuators.h"

int main()
{
    mqd_t mq_receiver = open_mq(ACTUATORS_MQ);
    char buffer[MQ_MAX_MSG_SIZE];
    while (1)
    {
        read_mq(mq_receiver, buffer);
        printf("Received message: <%s>\n", buffer);
        sleep(1);
    }

    return 0;
}

StateSeatBelt getSeatBeltState(SeatBelt seatBelt){
    StateSeatBelt currentSeatBelt = LOOSE_BELT;

    if(seatBelt.shouldTightenBelt == true){
        currentSeatBelt = TIGHT_BELT;
    } else {
        currentSeatBelt = LOOSE_BELT;
    }

    return currentSeatBelt;
}

StateDoorLock getDoorLockState(DoorLock doorLock){
    StateDoorLock currentDoorLock = LOCKED_DOOR;

    if(doorLock.shouldUnlockDoor == true){
        currentDoorLock = UNLOCKED_DOOR;
    } else {
        currentDoorLock = LOCKED_DOOR;
    }

    return currentDoorLock;
}

StateABS getABSState(ABS abs){
    StateABS currentABS = ABS_DEACTIVATED;

    if(abs.shouldActivateBreak == true){
        currentABS = ABS_ACTIVATED;
    } else {
        currentABS = ABS_DEACTIVATED;
    }

    return currentABS;
}