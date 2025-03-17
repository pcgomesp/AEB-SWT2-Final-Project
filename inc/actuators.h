#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <stdbool.h>

typedef struct{
    bool belt_tightness;
    bool door_lock;
    bool should_activate_abs;
} Actuators_abstraction;

// typedef struct SeatBeltStruct{
//     bool shouldTightenBelt; 
// } SeatBelt; 

typedef enum SeatBeltEnum{
    LOOSE_BELT = 0,
    TIGHT_BELT = 1 
} StateSeatBelt; 

// typedef struct DoorLockStruct{
//     bool shouldUnlockDoor; 
// } DoorLock; 

typedef enum DoorLockEnum{
    UNLOCKED_DOOR = 0,
    LOCKED_DOOR = 1 
} StateDoorLock;

// typedef struct ABSStruct{
//     bool shouldActivateBreak; 
// } ABS; 

typedef enum ABSEnum{
    ABS_DEACTIVATED = 0,
    ABS_ACTIVATED = 1
} StateABS;

#endif