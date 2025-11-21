#ifndef PCA9685_H
#define PCA9685_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// PCA9685 I2C Address
#define PCA9685_I2C_ADDRESS 0x40

// Register addresses
#define PCA9685_MODE1_REG       0x00
#define PCA9685_MODE2_REG       0x01
#define PCA9685_PRESCALE_REG    0xFE
#define PCA9685_LED0_ON_L       0x06
#define PCA9685_LED0_ON_H       0x07
#define PCA9685_LED0_OFF_L      0x08
#define PCA9685_LED0_OFF_H      0x09

// Mode1 register bits
#define PCA9685_RESTART         0x80
#define PCA9685_EXTCLK          0x40
#define PCA9685_AI              0x20
#define PCA9685_SLEEP           0x10
#define PCA9685_SUB1            0x08
#define PCA9685_SUB2            0x04
#define PCA9685_SUB3            0x02
#define PCA9685_ALLCALL         0x01

// Servo pulse parameters (adjust these for your servo)
#define SERVO_MIN_PULSE  125  //125 // Minimum pulse length (0 degrees)
#define SERVO_MAX_PULSE  500  //500 // Maximum pulse length (180 degrees)

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t address;
    float frequency;
} PCA9685_HandleTypeDef;

// Function prototypes
bool PCA9685_Init(PCA9685_HandleTypeDef *pca, I2C_HandleTypeDef *hi2c, uint8_t address, float freq);
bool PCA9685_SetPWM(PCA9685_HandleTypeDef *pca, uint8_t channel, uint16_t on, uint16_t off);
bool PCA9685_SetServoAngle(PCA9685_HandleTypeDef *pca, uint8_t channel, uint8_t angle);
bool PCA9685_Sleep(PCA9685_HandleTypeDef *pca, bool sleep);
bool PCA9685_Reset(PCA9685_HandleTypeDef *pca);


#ifdef __cplusplus
}
#endif

#endif // PCA9685_H