#include "pca9685.h"

// Private function prototypes
static bool PCA9685_WriteRegister(PCA9685_HandleTypeDef *pca, uint8_t reg, uint8_t value);
static bool PCA9685_ReadRegister(PCA9685_HandleTypeDef *pca, uint8_t reg, uint8_t *value);

bool PCA9685_Init(PCA9685_HandleTypeDef *pca, I2C_HandleTypeDef *hi2c, uint8_t address, float freq) {
    pca->hi2c = hi2c;
    pca->address = address << 1; // Shift address for HAL
    pca->frequency = freq;
    
    // Reset device
    if (!PCA9685_Reset(pca)) {
        return false;
    }
    
    // Set PWM frequency
    uint8_t prescale = (uint8_t)((25000000.0 / (4096.0 * freq)) - 1.0);
    
    // Put to sleep to set prescale
    if (!PCA9685_Sleep(pca, true)) {
        return false;
    }
    
    if (!PCA9685_WriteRegister(pca, PCA9685_PRESCALE_REG, prescale)) {
        return false;
    }
    
    // Wake up
    if (!PCA9685_Sleep(pca, false)) {
        return false;
    }
    
    // HAL_Delay(5);
    // uint8_t mode1;
    // if (!PCA9685_ReadRegister(pca, PCA9685_MODE1_REG, &mode1)) {
    //     return false;
    // }
    // mode1 |= PCA9685_RESTART;
    // if (!PCA9685_WriteRegister(pca, PCA9685_MODE1_REG, mode1)) {
    //     return false;
    // }

    HAL_Delay(1);
    
    // Set mode register with auto-increment
    return PCA9685_WriteRegister(pca, PCA9685_MODE1_REG, PCA9685_AI);
}

bool PCA9685_SetPWM(PCA9685_HandleTypeDef *pca, uint8_t channel, uint16_t on, uint16_t off) {
    uint8_t reg = PCA9685_LED0_ON_L + (4 * channel);
    
    uint8_t data[4] = {
        on & 0xFF,           // LED_ON_L
        (on >> 8) & 0x0F,    // LED_ON_H
        off & 0xFF,          // LED_OFF_L
        (off >> 8) & 0x0F    // LED_OFF_H
    };
    
    if (HAL_I2C_Mem_Write(pca->hi2c, pca->address, reg, I2C_MEMADD_SIZE_8BIT, data, 4, 100) != HAL_OK) {
        return false;
    }
    
    return true;
}

bool PCA9685_SetServoAngle(PCA9685_HandleTypeDef *pca, uint8_t channel, uint8_t angle) {
    // Constrain angle to 0-180
    if (angle > 180) angle = 180;
    
    // Map angle to pulse width
    uint16_t pulse = SERVO_MIN_PULSE + ((SERVO_MAX_PULSE - SERVO_MIN_PULSE) * angle) / 180;
    // printf("Servo %d: Angle=%d° -> Pulse=%dμs\r\n", channel, angle, pulse);
    // Set PWM (always start at 0, end at pulse value)
    return PCA9685_SetPWM(pca, channel, 0, pulse);
}

bool PCA9685_Sleep(PCA9685_HandleTypeDef *pca, bool sleep) {
    uint8_t mode1;
    
    if (!PCA9685_ReadRegister(pca, PCA9685_MODE1_REG, &mode1)) {
        return false;
    }
    
    if (sleep) {
        mode1 |= PCA9685_SLEEP;
    } else {
        mode1 &= ~PCA9685_SLEEP;
    }
    
    return PCA9685_WriteRegister(pca, PCA9685_MODE1_REG, mode1);
}

bool PCA9685_Reset(PCA9685_HandleTypeDef *pca) {
    // Software reset - write 0x06 to mode1 register (not the standard I2C general call)
    return PCA9685_WriteRegister(pca, PCA9685_MODE1_REG, 0x06);
}

// Private functions
static bool PCA9685_WriteRegister(PCA9685_HandleTypeDef *pca, uint8_t reg, uint8_t value) {
    if (HAL_I2C_Mem_Write(pca->hi2c, pca->address, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 100) != HAL_OK) {
        return false;
    }
    return true;
}

static bool PCA9685_ReadRegister(PCA9685_HandleTypeDef *pca, uint8_t reg, uint8_t *value) {
    if (HAL_I2C_Mem_Read(pca->hi2c, pca->address, reg, I2C_MEMADD_SIZE_8BIT, value, 1, 100) != HAL_OK) {
        return false;
    }
    return true;
}

// Add to pca9685.c
bool PCA9685_SetServoPulse(PCA9685_HandleTypeDef *pca, uint8_t channel, uint16_t pulse_us) {
    // Convert microseconds to 12-bit PWM value
    // PCA9685 resolution: 4096 steps @ 50Hz = 20ms period
    // pulse_us / 20000 * 4096 = pulse_us * 0.2048
    uint16_t pwm_value = (pulse_us * 4096) / 20000;
    
    if (pwm_value > 4095) pwm_value = 4095;
    
    return PCA9685_SetPWM(pca, channel, 0, pwm_value);
}