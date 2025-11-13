#ifndef HAL_UART_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#endif

#include "main.h"
#include "pca9685.h"
#include "periph_init.h"
#include "stm32f4xx_hal.h"
#include "servo_control.h"
#include "gestures.h"
#include <stdio.h>
#include <cstdio>
#include <fstream>
#include <string>

volatile bool data_rdy_f = false;
uint16_t adc_buffer[ADC_CHANNELS * SAMPLES] = { 0 };
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


void LogADCDataToFile()
{
    static std::ofstream adcfile("adc_data.csv");
    if (!adcfile.is_open()) return;
    
    for (int i = 0; i < SAMPLES; i += 8)
    {
        adcfile << adc_buffer[i * ADC_CHANNELS + 0] << ","
                << adc_buffer[i * ADC_CHANNELS + 1] << ","
                << adc_buffer[i * ADC_CHANNELS + 2] << ","
                << adc_buffer[i * ADC_CHANNELS + 3] << std::endl;
    }
    adcfile.flush();
}

// Викликайте цю функцію після виводу в UART:
// printf(">CH1:%d,CH2:%d,CH3:%d\r\n", ...);
// LogADCDataToFile();


int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_I2C1_Init();

    printf("=== 4-CHANNEL DMA SENSOR PLOTTER ===\r\n");
    printf("Channels: PA0, PA1, PA2, PA3\r\n");
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


    // start ADC with DMA for 4 channels
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

            for (int i = 0; i < SAMPLES; i += 8)
            {
            // printf("1 %d, 2 %d, 3 %d, 4 %d,\n",
                // printf(">CH1:%d,CH2:%d,CH3:%d,CH4:%d\r\n",
                printf(">CH1:%d,CH2:%d,CH3:%d\r\n",
                    adc_buffer[i * ADC_CHANNELS + 0],  // Channel 0 (PA0)
                    adc_buffer[i * ADC_CHANNELS + 1],  // Channel 1 (PA1)
                    adc_buffer[i * ADC_CHANNELS + 2]);  // Channel 2 (PA2)
                    // adc_buffer[i * ADC_CHANNELS + 3]); // Channel 3 (PA3)

                    HAL_Delay(10);
            }

            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            data_rdy_f = false;
            HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, ADC_CHANNELS * SAMPLES);
        }
        HAL_Delay(10);
    }
}



void TestServo(void)
{
    printf("\r\n=== SERVO GESTURE TEST ===\r\n");

    HAL_Delay(2000);
    Gesture_Execute(GESTURE_OPEN_HAND);
    HAL_Delay(1000);

    Gesture_Execute(GESTURE_FIST);
    HAL_Delay(1000);

    // Gesture_Execute(GESTURE_OPEN_HAND);
    // HAL_Delay(2000);

    TestFingerSequence();
    // HAL_Delay(6000);
    Gesture_Execute(GESTURE_OPEN_HAND);

    printf("\r\n=== GESTURE TESTING COMPLETE ===\r\n");
    printf("Entering main loop with ADC data...\r\n");

    // SetServo1Angle(90);
    // HAL_Delay(1000);
    // SetServo1Angle(90); // 1

    // // SetServo2Angle(135);
    // HAL_Delay(500);
    // SetServo2Angle(0); // 135 closed 2

    // // SetServo3Angle(90);
    // HAL_Delay(500);
    // SetServo3Angle(180); // 90 closed 3

    // // SetServo4Angle(10);
    // HAL_Delay(500);
    // SetServo4Angle(70); // 0 calm 70 5

    // // SetServo5Angle(90);
    // HAL_Delay(500); // 0 calm
    // SetServo5Angle(0); // 135 4

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
        data_rdy_f = true;
    }
}

void Error_Handler(void)
{
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(100);
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