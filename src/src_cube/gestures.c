#include "gestures.h"
#include "servo_control.h"
#include <stdio.h>

#define NUM_FINGERS 5

typedef struct {
    GestureID_t id; 
    const char* name;
    uint8_t angles[NUM_FINGERS];
} GestureDefinition_t;


static const GestureDefinition_t g_gesture_database[8] = {
    // ID                  Name         {Thumb, Index, Middle, Ring, Pinky}
    {GESTURE_OPEN_HAND,    "OPEN_HAND",      {0,     0,     0,       0,     0}},
    {GESTURE_FIST,         "FIST",           {135,   135,   135,     135,   135}},
    {GESTURE_THUMB_CLOSE,  "THUMB_CLOSE",    {45,    0,     0,       0,     0}},
    {GESTURE_INDEX_CLOSE,  "INDEX_CLOSE",    {0,     45,    0,       0,     0}},
    {GESTURE_MIDDLE_CLOSE, "MIDDLE_CLOSE",   {0,     0,     45,      0,     0}},
    {GESTURE_RING_CLOSE,   "RING_CLOSE",     {0,     0,     0,       45,    0}},
    {GESTURE_PINKY_CLOSE,  "PINKY_CLOSE",    {0,     0,     0,       0,    45}},
    {GESTURE_ALL_OPEN,     "ALL_OPEN",       {0,     0,     0,       0,     0}},
};

void Gesture_Execute(GestureID_t gesture_id) {
    if (gesture_id >= 8) {
        printf("! Error: gesture does not exist (ID: %d)\r\n", gesture_id);
        return;
    }

    const GestureDefinition_t* gesture = &g_gesture_database[gesture_id];

    printf("--- Execute gesture: %s ---\r\n", gesture->name);
    printf("Angles: Thumb=%d°, Index=%d°, Middle=%d°, Ring=%d°, Pinky=%d°\r\n",
           gesture->angles[0], gesture->angles[1], gesture->angles[2], 
           gesture->angles[3], gesture->angles[4]);

    SetServo1Angle(gesture->angles[0]);
    SetServo2Angle(gesture->angles[1]);
    SetServo3Angle(gesture->angles[2]);
    SetServo4Angle(gesture->angles[3]);
    SetServo5Angle(gesture->angles[4]);

    HAL_Delay(1000);
}

void TestFingerSequence(void) {
    printf("\r\n=== STARTING FINGER SEQUENCE TEST ===\r\n");
    
    // Start with open hand
    printf("\r\n1. Opening hand...\r\n");
    Gesture_Execute(GESTURE_OPEN_HAND);
    HAL_Delay(1000);
    
    // Close each finger one by one
    // printf("\r\n2. Closing thumb...\r\n");
    // Gesture_Execute(GESTURE_THUMB_CLOSE);
    // HAL_Delay(1000);
    
    printf("\r\n6. Closing pinky finger...\r\n");
    Gesture_Execute(GESTURE_RING_CLOSE);
    HAL_Delay(1000);

    printf("\r\n3. Closing ring finger...\r\n");
    Gesture_Execute(GESTURE_INDEX_CLOSE);
    HAL_Delay(1000);
    
    printf("\r\n4. Closing middle finger...\r\n");
    Gesture_Execute(GESTURE_MIDDLE_CLOSE);
    HAL_Delay(1000);
    
    printf("\r\n5. Closing index finger...\r\n");
    Gesture_Execute(GESTURE_PINKY_CLOSE);
    HAL_Delay(1000);
    
    // printf("\r\n6. Closing pinky finger...\r\n");
    // Gesture_Execute(GESTURE_RING_CLOSE);
    // HAL_Delay(1000);
    
    // Now we should have a fist
    printf("\r\n7. Full fist (all fingers closed)...\r\n");
    Gesture_Execute(GESTURE_FIST);
    HAL_Delay(2000);
    
    // Open all fingers simultaneously
    printf("\r\n8. Opening all fingers simultaneously...\r\n");
    Gesture_Execute(GESTURE_ALL_OPEN);
    HAL_Delay(2000);
    
    printf("\r\n=== FINGER SEQUENCE TEST COMPLETE ===\r\n");
}