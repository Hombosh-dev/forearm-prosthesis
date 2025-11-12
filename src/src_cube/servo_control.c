#include "servo_control.h"
#include "pca9685.h"
#include "main.h"

// imports from main.h
extern PCA9685_HandleTypeDef pca9685;

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