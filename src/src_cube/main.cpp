#ifndef HAL_UART_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#endif

#include "main.h"
#include "pca9685.h"
#include "periph_init.h"
#include "stm32f4xx_hal.h"
#include "servo_control.h"
#include "gestures.h"
#include "emg_control.h"
#include <stdio.h>
#include <cstdio>
#include <fstream>
#include <string>
#include <cstring>

volatile bool data_rdy_f = false;
// uint16_t adc_buffer[ADC_CHANNELS * SAMPLES] = { 0 };
// memory alignment for DMA
__attribute__((aligned(4))) uint16_t adc_buffer[ADC_CHANNELS * SAMPLES] = {0};
PCA9685_HandleTypeDef pca9685;

typedef struct
{
    uint8_t addr[127];
    uint8_t dev_count;
} I2C_devices_list_t;

I2C_devices_list_t i2c_dev_list = { 0 };

I2C_devices_list_t *I2C_CheckBusDevices(void)
{
    static I2C_devices_list_t i2c_devices = { 0 };

    for (uint32_t i = 0; i < 128U; i++)
    {
        uint16_t adress = i << 1;
        if (HAL_I2C_IsDeviceReady(&hi2c1, adress, 1, HAL_MAX_DELAY) == HAL_OK)
        {
            i2c_devices.addr[i2c_devices.dev_count] = adress;
            i2c_devices.dev_count++;
        }
    }
    return &i2c_devices;
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_I2C1_Init();
    EMG_Control_Init();

    printf("=== 3-CHANNEL DMA SENSOR PLOTTER ===\r\n");
    printf("Channels: PA0, PA1, PA2\r\n");
    printf("Sample Rate: ~100 kHz per channel\r\n");
    printf("Total Samples: %d\r\n\r\n", ADC_CHANNELS * SAMPLES);

    printf("=== I2C DEVICE SCAN ===\r\n");

    i2c_dev_list = *(I2C_CheckBusDevices());
    for (uint32_t i = 0; i < i2c_dev_list.dev_count; ++i)
    {
        printf("I2C device found at address: 0x%02X \r\n", i2c_dev_list.addr[i]);
    }

    // Initialize PCA9685 for servo control (60Hz for servos)
    if (PCA9685_Init(&pca9685, &hi2c1, PCA9685_I2C_ADDRESS, 50.0))
    {
        printf("PCA9685 initialized successfully\r\n");
        // TestServo();
    }
    else
    {
        printf("PCA9685 initialization failed!\r\n");
        printf("Check I2C connections and address.\r\n");

        uint8_t alt_addresses[] = { 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47 };
        for (int i = 0; i < 7; i++)
        {
            if (PCA9685_Init(&pca9685, &hi2c1, alt_addresses[i], 50.0))
            {
                printf("PCA9685 found at 0x%02X and initialized!\r\n", alt_addresses[i]);
                // TestServo();
                break;
            }
        }
    }

    // // ADC Calibration
    // #ifdef HAL_ADC_MODULE_ENABLED
    // printf("Calibrating ADC...\r\n");
    // if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK) {
    //     printf("ADC calibration failed or not supported!\r\n");
    // } else {
    //     printf("ADC calibration successful\r\n");
    // }
    // #endif printf("Calibrating ADC...\r\n");
    // if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK) {
    //     printf("ADC calibration failed!\r\n");
    // } else {
    //     printf("ADC calibration successful\r\n");
    // }

    // HAL_Delay(100);

    // Clear buffer before starting
    for (int i = 0; i < ADC_CHANNELS * SAMPLES; i++) {
        adc_buffer[i] = 0;
    }


    // start ADC with DMA for 3 channels
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, ADC_CHANNELS * SAMPLES) != HAL_OK)
    {
        Error_Handler();
    }

    while (!data_rdy_f) {
        HAL_Delay(1);
    }

    // Restart ADC for continuous operation
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, ADC_CHANNELS * SAMPLES);

    while (1)
    {
        if (data_rdy_f)
        {
            // for (int sample_idx = 0; sample_idx < 512; sample_idx++) // Show first 16 samples
            // {
            //     uint32_t base_idx = sample_idx * ADC_CHANNELS;
                
            //     uint16_t ch1 = adc_buffer[base_idx + 0];  // PA0 - should be ~4095
            //     uint16_t ch2 = adc_buffer[base_idx + 1];  // PA1 - should be ~0
            //     uint16_t ch3 = adc_buffer[base_idx + 2];  // PA2 - should be ~4095
                
            //     printf(">CH1:%d,CH2:%d,CH3:%d\r\n", ch1, ch2, ch3);
            // }

            // HAL_Delay(10);
            // HAL_ADC_Stop_DMA(&hadc1);
            // HAL_Delay(5);
            // for (int i = 0; i < ADC_CHANNELS * SAMPLES; i++) {
            //     adc_buffer[i] = 0;
            // }

            EMG_Control_Process();
            data_rdy_f = false;
            // HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, ADC_CHANNELS * SAMPLES);
        }
        // HAL_Delay(100);
        HAL_Delay(1);
    }
}



void TestServo(void)
{
    printf("\r\n=== SERVO TEST ===\r\n");
    
    printf("Open hand (0 degrees)...\r\n");
    SetServo1Angle(145); // ring
    HAL_Delay(100);
    SetServo2Angle(160);
    HAL_Delay(100);
    SetServo3Angle(170); // max
    HAL_Delay(100);
    SetServo4Angle(160); // max
    HAL_Delay(100);
    // SetServo5Angle(90);
    // HAL_Delay(2000);
    
    printf("Half fist (90 degrees)...\r\n");
    SetServo1Angle(0);
    HAL_Delay(100);
    SetServo2Angle(0);
    HAL_Delay(100);
    SetServo3Angle(0);
    HAL_Delay(100);
    SetServo4Angle(0);
    HAL_Delay(100);
    // SetServo5Angle(90);
    HAL_Delay(2000);

    printf("Open hand (0 degrees)...\r\n");
    SetServo1Angle(145); // ring
    HAL_Delay(100);
    SetServo2Angle(90);
    HAL_Delay(100);
    SetServo3Angle(160); // max
    HAL_Delay(100);
    SetServo4Angle(140); // max 160
    HAL_Delay(100);
    
    // printf("Full fist (180 degrees)...\r\n");
    // SetServo1Angle(180);
    // HAL_Delay(100);
    // SetServo2Angle(180);
    // HAL_Delay(100);
    // SetServo3Angle(180);
    // HAL_Delay(100);
    // SetServo4Angle(180);
    // HAL_Delay(100);
    // SetServo5Angle(180);
    // HAL_Delay(2000);
    
    // printf("Return to open hand...\r\n");
    // SetServo1Angle(0);
    // SetServo2Angle(0);
    // SetServo3Angle(0);
    // SetServo4Angle(0);
    // SetServo5Angle(0);
    
    printf("\r\n=== SERVO TEST COMPLETE ===\r\n");
}

void TestIndividualFingers(void) {
    printf("\r\n--- Testing Individual Fingers ---\r\n");
    
    printf("Testing thumb...\r\n");
    SetServo1Angle(0);
    HAL_Delay(1000);
    SetServo1Angle(90);
    HAL_Delay(1000);
    SetServo1Angle(90);
    HAL_Delay(500);
    
    printf("Testing index finger...\r\n");
    SetServo2Angle(0);
    HAL_Delay(1000);
    SetServo2Angle(90);
    HAL_Delay(1000);
    SetServo2Angle(90);
    HAL_Delay(500);
    
    printf("Testing middle finger...\r\n");
    SetServo3Angle(0);
    HAL_Delay(1000);
    SetServo3Angle(90);
    HAL_Delay(1000);
    SetServo3Angle(90);
    HAL_Delay(500);
    
    printf("Testing ring finger...\r\n");
    SetServo4Angle(0);
    HAL_Delay(1000);
    SetServo4Angle(90);
    HAL_Delay(1000);
    SetServo4Angle(90);
    HAL_Delay(500);
    
    printf("Testing pinky finger...\r\n");
    SetServo5Angle(0);
    HAL_Delay(1000);
    SetServo5Angle(90);
    HAL_Delay(1000);
    SetServo5Angle(90);
    HAL_Delay(500);
    
    Gesture_Execute(GESTURE_OPEN_HAND);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        __DSB();
        data_rdy_f = true;
    }
}

void Error_Handler(void)
{
    while (1)
    {
        // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        // HAL_Delay(100);
    }
}

extern "C"
{
    int _write(int file, char *ptr, int len)
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
        return len;
    }
    void DMA2_Stream0_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_adc1); }
}