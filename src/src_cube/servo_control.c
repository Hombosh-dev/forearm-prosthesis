#include "servo_control.h"
#include "pca9685.h"
#include "main.h"

// imports from main.h
extern PCA9685_HandleTypeDef pca9685;

const uint8_t const NUM_SERVOS = 5;
static const uint8_t servo_channels[NUM_SERVOS] = { 0, 1, 2, 3, 4 };

void SetServo1Angle(uint8_t angle) {
    PCA9685_SetServoAngle(&pca9685, 0, angle);
}

void SetServo2Angle(uint8_t angle) {
    PCA9685_SetServoAngle(&pca9685, 1, angle);
}

void SetServo3Angle(uint8_t angle) {
    PCA9685_SetServoAngle(&pca9685, 2, angle);
}

void SetServo4Angle(uint8_t angle) {
    PCA9685_SetServoAngle(&pca9685, 3, angle);
}

void SetServo5Angle(uint8_t angle) {
    PCA9685_SetServoAngle(&pca9685, 4, angle);
}