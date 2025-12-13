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
    angle = CLAMP_ANGLE(angle, SERVO4_MIN, SERVO4_MAX);
    PCA9685_SetServoAngle(&pca9685, SERVO_RING_CHANNEL, angle);
    // printf("Servo4(Ring): Set to %d°\r\n", angle);
}

void SetServo5Angle(uint8_t angle) {
    angle = CLAMP_ANGLE(angle, SERVO5_MIN, SERVO5_MAX);
    PCA9685_SetServoAngle(&pca9685, SERVO_PINKY_CHANNEL, angle);
    // printf("Servo5(Pinky): Set to %d° (clamped: %d°)\r\n", angle, angle);
}

void SetServo1Normalized(uint8_t normalized_angle) {
    if (normalized_angle <= 90) {
        uint8_t angle = SERVO1_OPEN + (normalized_angle * (SERVO1_HALF - SERVO1_OPEN) / 90);
        SetServo1Angle(angle);
    } else {
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


void OpenHand(void) {
    printf("Gesture: Open Hand\r\n");
    SetServo1Angle(SERVO1_OPEN);    
    SetServo2Angle(SERVO2_OPEN); 
    SetServo3Angle(SERVO3_OPEN);  
    SetServo4Angle(SERVO4_OPEN);  
    SetServo5Angle(SERVO5_OPEN);   
}

void HalfGrip(void) {
    printf("Gesture: Half Grip\r\n");
    SetServo2Angle(SERVO2_HALF);     
    SetServo1Angle(SERVO1_HALF);     
    SetServo3Angle(SERVO3_HALF);     
    SetServo4Angle(SERVO4_HALF);     
    SetServo5Angle(SERVO5_HALF);     
}

void CloseHand(void) {
    printf("Gesture: Close Hand/Fist\r\n");
    SetServo1Angle(SERVO1_CLOSED);   
    SetServo2Angle(SERVO2_CLOSED);  
    SetServo3Angle(SERVO3_CLOSED);   
    SetServo4Angle(SERVO4_CLOSED);  
    SetServo5Angle(SERVO5_CLOSED);  
}

void FourClosedThumbOpen(void) {
    printf("Gesture: 4 Fingers Closed, Thumb Open\r\n");
    SetServo1Angle(SERVO1_OPEN);     
    SetServo2Angle(SERVO2_CLOSED);   
    SetServo3Angle(SERVO3_CLOSED);   
    SetServo4Angle(SERVO4_CLOSED);   
    SetServo5Angle(SERVO5_CLOSED);   
}

void PointGesture(void) {
    printf("Gesture: Point (Index extended)\r\n");
    SetServo1Angle(SERVO1_CLOSED);   
    SetServo2Angle(SERVO2_OPEN);    
    SetServo3Angle(SERVO3_CLOSED);   
    SetServo4Angle(SERVO4_CLOSED);   
    SetServo5Angle(SERVO5_CLOSED);   
}

void OKGesture(void) {
    printf("Gesture: OK (Thumb and index making a circle)\r\n");
    SetServo1Angle(60);             
    SetServo2Angle(60);             
    SetServo3Angle(SERVO3_CLOSED);  
    SetServo4Angle(SERVO4_CLOSED);   
    SetServo5Angle(SERVO5_CLOSED);  
}

void TestServoSequence(void) {
    printf("\r\n=== SERVO TEST SEQUENCE ===\r\n");
    
    printf("1. Open hand...\r\n");
    SetServo1Angle(0);     
    HAL_Delay(100);
    SetServo2Angle(0);   
    HAL_Delay(100);
    SetServo3Angle(10);    
    HAL_Delay(100);
    SetServo4Angle(20);    
    HAL_Delay(100);
    SetServo5Angle(0);   
    HAL_Delay(2000);
    
    printf("2. Half grip...\r\n");
    SetServo1Angle(90);   
    HAL_Delay(100);
    SetServo2Angle(120);   
    HAL_Delay(100);
    SetServo3Angle(120);  
    HAL_Delay(100);
    SetServo4Angle(130);  
    HAL_Delay(100);
    SetServo5Angle(90);   
    HAL_Delay(2000);
    
    printf("3. Full fist...\r\n");
    SetServo1Angle(150);   
    HAL_Delay(100);
    SetServo2Angle(180);   
    HAL_Delay(100);
    SetServo3Angle(170);   
    HAL_Delay(100);
    SetServo4Angle(180);   
    HAL_Delay(100);
    SetServo5Angle(120);
    HAL_Delay(2000);
    
    printf("4. Return to open hand...\r\n");
    OpenHand();
    HAL_Delay(1000);
    
    printf("\r\n=== TEST COMPLETE ===\r\n");
}