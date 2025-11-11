#ifndef GESTURES_H
#define GESTURES_H

#include <stdint.h>

typedef enum {
    GESTURE_OPEN_HAND,
    GESTURE_FIST,
    GESTURE_POINT,
    GESTURE_THUMBS_UP,

    GESTURE_COUNT
} GestureID_t;


void Gesture_Execute(GestureID_t gesture_id);

#endif