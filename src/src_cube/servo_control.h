#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H
#ifdef __cplusplus
extern "C" {
#endif

#include "pca9685.h"
// #include "main.h"
#include "stm32f4xx_hal.h"

#define SERVO_THUMB_CHANNEL   0
#define SERVO_INDEX_CHANNEL   1
#define SERVO_MIDDLE_CHANNEL  2
#define SERVO_RING_CHANNEL    3
#define SERVO_PINKY_CHANNEL   4

void SetServo1Angle(uint8_t angle);
void SetServo2Angle(uint8_t angle);
void SetServo3Angle(uint8_t angle);
void SetServo4Angle(uint8_t angle);
void SetServo5Angle(uint8_t angle);

extern PCA9685_HandleTypeDef pca9685;

#ifdef __cplusplus
}
#endif

#endif
