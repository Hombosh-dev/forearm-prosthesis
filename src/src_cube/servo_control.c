#include "servo_control.h"
#include "pca9685.h"
// #include "main.h"

#define NUM_SERVOS 5
#define THUMB_CONST 0
#define INDEX_CONST 0
#define MIDDLE_CONST 0
#define RING_CONST 0
#define PINKY_CONST 0
// extern PCA9685_HandleTypeDef pca9685;

void SetServo1Angle(uint8_t angle) {
    PCA9685_SetServoAngle(&pca9685, SERVO_THUMB_CHANNEL, angle+THUMB_CONST);
}

void SetServo2Angle(uint8_t angle) {
    PCA9685_SetServoAngle(&pca9685, SERVO_INDEX_CHANNEL, angle+INDEX_CONST);
}

void SetServo3Angle(uint8_t angle) {
    PCA9685_SetServoAngle(&pca9685, SERVO_MIDDLE_CHANNEL, angle+MIDDLE_CONST);
}

void SetServo4Angle(uint8_t angle) {
    PCA9685_SetServoAngle(&pca9685, SERVO_RING_CHANNEL, angle+RING_CONST);
}

void SetServo5Angle(uint8_t angle) {
    PCA9685_SetServoAngle(&pca9685, SERVO_PINKY_CHANNEL, angle+PINKY_CONST);
}