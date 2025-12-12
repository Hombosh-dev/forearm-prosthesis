// servo_control.c - Updated with your exact angles
#include "servo_control.h"
#include <stdio.h>
#include <math.h>

// Individual servo angle setting with your optimized clamping
void SetServo1Angle(uint8_t angle) {
    angle = CLAMP_ANGLE(angle, SERVO1_MIN, SERVO1_MAX);
    PCA9685_SetServoAngle(&pca9685, SERVO_THUMB_CHANNEL, angle);
    // printf("Servo1(Thumb): Set to %d° (clamped: %d°)\r\n", angle, angle);
}

void SetServo2Angle(uint8_t angle) {
    angle = CLAMP_ANGLE(angle, SERVO2_MIN, SERVO2_MAX);
    PCA9685_SetServoAngle(&pca9685, SERVO_INDEX_CHANNEL, angle);
    // printf("Servo2(Index): Set to %d° (clamped: %d°)\r\n", angle, angle);
}

void SetServo3Angle(uint8_t angle) {
    angle = CLAMP_ANGLE(angle, SERVO3_MIN, SERVO3_MAX);
    PCA9685_SetServoAngle(&pca9685, SERVO_MIDDLE_CHANNEL, angle);
    // printf("Servo3(Middle): Set to %d° (clamped: %d°)\r\n", angle, angle);
}

void SetServo4Angle(uint8_t angle) {
    // Special handling for extended range servo (up to 200°)
    if (angle > 180) {
        // Map 181-200 to extended pulse width
        uint16_t extended_pulse = 500 + ((angle - 180) * 5); // Extended pulse calculation
        PCA9685_SetServoPulse(&pca9685, SERVO_RING_CHANNEL, extended_pulse);
    } else {
        angle = CLAMP_ANGLE(angle, SERVO4_MIN, SERVO4_MAX);
        PCA9685_SetServoAngle(&pca9685, SERVO_RING_CHANNEL, angle);
    }
    // printf("Servo4(Ring): Set to %d°\r\n", angle);
}

void SetServo5Angle(uint8_t angle) {
    angle = CLAMP_ANGLE(angle, SERVO5_MIN, SERVO5_MAX);
    PCA9685_SetServoAngle(&pca9685, SERVO_PINKY_CHANNEL, angle);
    // printf("Servo5(Pinky): Set to %d° (clamped: %d°)\r\n", angle, angle);
}

// Enhanced normalized mapping functions
// These map 0-180 normalized to your specific servo ranges
void SetServo1Normalized(uint8_t normalized_angle) {
    if (normalized_angle > 180) normalized_angle = 180;
    
    if (normalized_angle <= 90) {
        // 0-90 normalized maps to SERVO1_OPEN to SERVO1_HALF
        uint8_t angle = SERVO1_OPEN + (normalized_angle * (SERVO1_HALF - SERVO1_OPEN) / 90);
        SetServo1Angle(angle);
    } else {
        // 90-180 normalized maps to SERVO1_HALF to SERVO1_CLOSED
        uint8_t angle = SERVO1_HALF + ((normalized_angle - 90) * (SERVO1_CLOSED - SERVO1_HALF) / 90);
        SetServo1Angle(angle);
    }
}

void SetServo2Normalized(uint8_t normalized_angle) {
    if (normalized_angle > 180) normalized_angle = 180;
    
    if (normalized_angle <= 90) {
        uint8_t angle = SERVO2_OPEN + (normalized_angle * (SERVO2_HALF - SERVO2_OPEN) / 90);
        SetServo2Angle(angle);
    } else {
        uint8_t angle = SERVO2_HALF + ((normalized_angle - 90) * (SERVO2_CLOSED - SERVO2_HALF) / 90);
        SetServo2Angle(angle);
    }
}

void SetServo3Normalized(uint8_t normalized_angle) {
    if (normalized_angle > 180) normalized_angle = 180;
    
    if (normalized_angle <= 90) {
        uint8_t angle = SERVO3_OPEN + (normalized_angle * (SERVO3_HALF - SERVO3_OPEN) / 90);
        SetServo3Angle(angle);
    } else {
        uint8_t angle = SERVO3_HALF + ((normalized_angle - 90) * (SERVO3_CLOSED - SERVO3_HALF) / 90);
        SetServo3Angle(angle);
    }
}

void SetServo4Normalized(uint8_t normalized_angle) {
    if (normalized_angle > 180) normalized_angle = 180;
    
    if (normalized_angle <= 90) {
        uint8_t angle = SERVO4_OPEN + (normalized_angle * (SERVO4_HALF - SERVO4_OPEN) / 90);
        SetServo4Angle(angle);
    } else {
        uint8_t angle = SERVO4_HALF + ((normalized_angle - 90) * (SERVO4_CLOSED - SERVO4_HALF) / 90);
        SetServo4Angle(angle);
    }
}

void SetServo5Normalized(uint8_t normalized_angle) {
    if (normalized_angle > 180) normalized_angle = 180;
    
    if (normalized_angle <= 90) {
        uint8_t angle = SERVO5_OPEN + (normalized_angle * (SERVO5_HALF - SERVO5_OPEN) / 90);
        SetServo5Angle(angle);
    } else {
        uint8_t angle = SERVO5_HALF + ((normalized_angle - 90) * (SERVO5_CLOSED - SERVO5_HALF) / 90);
        SetServo5Angle(angle);
    }
}

void SetAllServosNormalized(uint8_t normalized_angle) {
    SetServo1Normalized(normalized_angle);
    SetServo2Normalized(normalized_angle);
    SetServo3Normalized(normalized_angle);
    SetServo4Normalized(normalized_angle);
    SetServo5Normalized(normalized_angle);
}

// ================= YOUR OPTIMIZED GESTURES =================

void OpenHand(void) {
    printf("Gesture: Open Hand (Your optimized angles)\r\n");
    SetServo1Angle(SERVO1_OPEN);     // Your: 0 OPEN
    SetServo2Angle(SERVO2_OPEN);     // Your: 0 open
    SetServo3Angle(SERVO3_OPEN);     // Your: 10 open
    SetServo4Angle(SERVO4_OPEN);     // Your: 20 open
    SetServo5Angle(SERVO5_OPEN);     // Your: 0 OPEN
}

void HalfGrip(void) {
    printf("Gesture: Half Grip (Your optimized angles)\r\n");
    SetServo1Angle(SERVO1_HALF);     // Your: 90 half
    SetServo2Angle(SERVO2_HALF);     // Your: 120 half
    SetServo3Angle(SERVO3_HALF);     // Your: 120 half
    SetServo4Angle(SERVO4_HALF);     // Your: 130 half
    SetServo5Angle(SERVO5_HALF);     // Your: 90 half
}

void CloseHand(void) {
    printf("Gesture: Close Hand/Fist (Your optimized angles)\r\n");
    SetServo1Angle(SERVO1_CLOSED);   // Your: 150 closed
    SetServo2Angle(SERVO2_CLOSED);   // Your: 180 closed
    SetServo3Angle(SERVO3_CLOSED);   // Your: 170 closed
    SetServo4Angle(SERVO4_CLOSED);   // Your: 200 closed better
    SetServo5Angle(SERVO5_CLOSED);   // Your: 100 closed (adjusted)
}

void FourClosedThumbOpen(void) {
    printf("Gesture: 4 Fingers Closed, Thumb Open\r\n");
    SetServo1Angle(SERVO1_OPEN);     // Thumb open
    SetServo2Angle(SERVO2_CLOSED);   // Index closed
    SetServo3Angle(SERVO3_CLOSED);   // Middle closed
    SetServo4Angle(SERVO4_CLOSED);   // Ring closed
    SetServo5Angle(SERVO5_CLOSED);   // Pinky closed
}

void PointGesture(void) {
    printf("Gesture: Point (Index extended)\r\n");
    SetServo1Angle(SERVO1_CLOSED);   // Thumb closed
    SetServo2Angle(SERVO2_OPEN);     // Index open
    SetServo3Angle(SERVO3_CLOSED);   // Middle closed
    SetServo4Angle(SERVO4_CLOSED);   // Ring closed
    SetServo5Angle(SERVO5_CLOSED);   // Pinky closed
}

void OKGesture(void) {
    printf("Gesture: OK (Thumb and index making a circle)\r\n");
    SetServo1Angle(60);              // Thumb half-bent
    SetServo2Angle(60);              // Index half-bent
    SetServo3Angle(SERVO3_CLOSED);   // Middle closed
    SetServo4Angle(SERVO4_CLOSED);   // Ring closed
    SetServo5Angle(SERVO5_CLOSED);   // Pinky closed
}

// Your exact test sequence from the comment
void TestYourServoSequence(void) {
    printf("\r\n=== YOUR SERVO TEST SEQUENCE ===\r\n");
    
    printf("1. Open hand (your optimized angles)...\r\n");
    SetServo1Angle(0);     // 0 OPEN
    HAL_Delay(100);
    SetServo2Angle(0);     // 0 open
    HAL_Delay(100);
    SetServo3Angle(10);    // 10 open
    HAL_Delay(100);
    SetServo4Angle(20);    // 20 open
    HAL_Delay(100);
    SetServo5Angle(0);     // 0 OPEN
    HAL_Delay(2000);
    
    printf("2. Half grip (your midpoints)...\r\n");
    SetServo1Angle(90);    // 90 half
    HAL_Delay(100);
    SetServo2Angle(120);   // 120 half
    HAL_Delay(100);
    SetServo3Angle(120);   // 120 half
    HAL_Delay(100);
    SetServo4Angle(130);   // 130 half
    HAL_Delay(100);
    SetServo5Angle(90);    // 90 half
    HAL_Delay(2000);
    
    printf("3. Full fist (your optimized closed)...\r\n");
    SetServo1Angle(150);   // 150 closed
    HAL_Delay(100);
    SetServo2Angle(180);   // 180 closed
    HAL_Delay(100);
    SetServo3Angle(170);   // 170 closed
    HAL_Delay(100);
    SetServo4Angle(200);   // 200 closed better (extended)
    HAL_Delay(100);
    SetServo5Angle(100);   // 120 closed (better -20) -> using 100
    HAL_Delay(2000);
    
    printf("4. Return to open hand...\r\n");
    OpenHand();
    HAL_Delay(1000);
    
    printf("\r\n=== YOUR TEST COMPLETE ===\r\n");
}