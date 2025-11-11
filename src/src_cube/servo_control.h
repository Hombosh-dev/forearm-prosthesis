#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include "stm32f4xx_hal.h"

// pinky (little) finger
void SetServo1Angle(uint8_t angle);

// ring finger
void SetServo2Angle(uint8_t angle);

// middle finger
void SetServo3Angle(uint8_t angle);

// index finger
void SetServo4Angle(uint8_t angle);

// thumb
void SetServo5Angle(uint8_t angle);

#endif
