#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H
#ifdef __cplusplus
extern "C" {
#endif

#include "pca9685.h"

#define SERVO_THUMB_CHANNEL   0
#define SERVO_INDEX_CHANNEL   1
#define SERVO_MIDDLE_CHANNEL  2
#define SERVO_RING_CHANNEL    3
#define SERVO_PINKY_CHANNEL   4

#define SERVO1_OPEN     0
#define SERVO1_HALF     90
#define SERVO1_CLOSED   150
#define SERVO1_MIN      0
#define SERVO1_MAX      150

#define SERVO2_OPEN     0
#define SERVO2_HALF     120
#define SERVO2_CLOSED   180
#define SERVO2_MIN      0
#define SERVO2_MAX      180

#define SERVO3_OPEN     10
#define SERVO3_HALF     120
#define SERVO3_CLOSED   170
#define SERVO3_MIN      10
#define SERVO3_MAX      170

#define SERVO4_OPEN     20
#define SERVO4_HALF     130
#define SERVO4_CLOSED   180
#define SERVO4_MIN      20
#define SERVO4_MAX      180

#define SERVO5_OPEN     0
#define SERVO5_HALF     90
#define SERVO5_CLOSED   120
#define SERVO5_MIN      0
#define SERVO5_MAX      120

#define CLAMP_ANGLE(angle, min, max) \
    ((angle) < (min) ? (min) : ((angle) > (max) ? (max) : (angle)))

void SetServo1Angle(uint8_t angle);
void SetServo2Angle(uint8_t angle);
void SetServo3Angle(uint8_t angle);
void SetServo4Angle(uint8_t angle);
void SetServo5Angle(uint8_t angle);

void SetServo1Normalized(uint8_t normalized_angle); 
void SetServo2Normalized(uint8_t normalized_angle);
void SetServo3Normalized(uint8_t normalized_angle);
void SetServo4Normalized(uint8_t normalized_angle);
void SetServo5Normalized(uint8_t normalized_angle);
void SetAllServosNormalized(uint8_t normalized_angle);  

void OpenHand(void);
void CloseHand(void);
void HalfGrip(void);
void FourClosedThumbOpen(void);
void PointGesture(void);
void OKGesture(void);

void TestServoSequence(void);

extern PCA9685_HandleTypeDef pca9685;

#ifdef __cplusplus
}
#endif

#endif