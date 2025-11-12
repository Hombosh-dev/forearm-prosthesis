#include "servo_control.h"
#include "pca9685.h"
// #include "main.h"

#define NUM_SERVOS 5
// extern PCA9685_HandleTypeDef pca9685;

void SetServo1Angle(uint8_t angle) {
    PCA9685_SetServoAngle(&pca9685, SERVO_THUMB_CHANNEL, angle);
}

void SetServo2Angle(uint8_t angle) {
    PCA9685_SetServoAngle(&pca9685, SERVO_INDEX_CHANNEL, angle);
}

void SetServo3Angle(uint8_t angle) {
    PCA9685_SetServoAngle(&pca9685, SERVO_MIDDLE_CHANNEL, angle);
}

void SetServo4Angle(uint8_t angle) {
    PCA9685_SetServoAngle(&pca9685, SERVO_RING_CHANNEL, angle);
}

void SetServo5Angle(uint8_t angle) {
    PCA9685_SetServoAngle(&pca9685, SERVO_PINKY_CHANNEL, angle);
}