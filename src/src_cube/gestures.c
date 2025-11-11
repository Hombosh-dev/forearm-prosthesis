#include "gestures.h"
#include "servo_control.h"
#include <stdio.h>

#define NUM_FINGERS 5

typedef struct {
    GestureID_t id; 
    const char* name;
    uint8_t angles[NUM_FINGERS];
} GestureDefinition_t;

/**
 * Це єдине місце, яке редагуємо для додавання/зміни жестів.
 * Порядок кутів: {Мізинець, Безіменний, Середній, Вказівний, Великий}
 * ПРИМІТКА: Порядок елементів у цьому масиві МАЄ
 * співпадати з порядком в 'enum GestureID_t' у gestures.h!
 */
static const GestureDefinition_t g_gesture_database[GESTURE_COUNT] = {
    // ID                  Name            {Мізинець, Безім., Серед., Вказ., Великий}
    {GESTURE_OPEN_HAND, "OPEN_HAND",    {0,     0,     0,     0,     0}},
    {GESTURE_FIST,      "FIST",         {180, 180, 180, 180, 180}},
    {GESTURE_POINT,     "POINT",        {180, 180, 180,   0, 180}},
    {GESTURE_THUMBS_UP, "THUMBS_UP",    {180, 180, 180, 180,   0}},
};

void Gesture_Execute(GestureID_t gesture_id) {
    if (gesture_id >= GESTURE_COUNT) {
        printf("! Error: gesture does not exist (ID: %d)\r\n", gesture_id);
        return;
    }

    const GestureDefinition_t* gesture = &g_gesture_database[gesture_id];

    printf("--- Execute gesture: %s ---\r\n", gesture->name);

    SetServo1Angle(gesture->angles[0]);
    SetServo2Angle(gesture->angles[1]);
    SetServo3Angle(gesture->angles[2]);
    SetServo4Angle(gesture->angles[3]);
    SetServo5Angle(gesture->angles[4]);
}