#ifndef GESTURES_H
#define GESTURES_H

#include <stdint.h>

typedef enum {
    GESTURE_OPEN_HAND,
    GESTURE_FIST,
    GESTURE_THUMB_CLOSE,
    GESTURE_INDEX_CLOSE,
    GESTURE_MIDDLE_CLOSE,
    GESTURE_RING_CLOSE,
    GESTURE_PINKY_CLOSE,
    GESTURE_ALL_OPEN,

    GESTURE_COUNT
} GestureID_t;


void Gesture_Execute(GestureID_t gesture_id);
void TestFingerSequence(void);

#endif