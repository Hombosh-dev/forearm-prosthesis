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

    printf("=== 3-CHANNEL DMA SENSOR PLOTTER ===\r\n");
    printf("Channels: PA0, PA1, PA2\r\n");
    printf("Sample Rate: ~100 kHz per channel\r\n");
    printf("Total Samples: %d\r\n\r\n", ADC_CHANNELS * SAMPLES);

    EMG_Control_Init();
    EMG_AutoCalibrate();

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

    // clear buffer before starting
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

    // restart ADC for continuous operation
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, ADC_CHANNELS * SAMPLES);

    while (1)
    {
        if (data_rdy_f)
        {
            EMG_Control_Process();
            data_rdy_f = false;
        }
        
        static uint32_t last_raw_print = 0;
        uint32_t current_time = HAL_GetTick();
        
        if (current_time - last_raw_print >= 100) {
            int last_sample_index = (SAMPLES - 1) * ADC_CHANNELS;
            
            printf(">CH1:%d,CH2:%d,CH3:%d\r\n", 
                   adc_buffer[last_sample_index + 0],  // CH1 (PA0)
                   adc_buffer[last_sample_index + 1],  // CH2 (PA1) 
                   adc_buffer[last_sample_index + 2]); // CH3 (PA2)
            
            last_raw_print = current_time;
        }
        
    }
}



void TestServo(void) {
    printf("\r\n=== SERVO TEST ===\r\n");  // primary open okay, half okay, closed
    
    printf("Open hand (0 degrees)...\r\n");
    SetServo1Angle(0);     // 0 OPEN  | 90 half  | 150 closed 
    HAL_Delay(100);
    SetServo2Angle(0);     // 0 open  | 120 half | 180 closed 
    HAL_Delay(100);
    SetServo3Angle(10);    // 10 open | 120 half | 170 closed  
    HAL_Delay(100);
    SetServo4Angle(20);    // 20 open | 130 half | 180 closed
    HAL_Delay(100);
    SetServo5Angle(0);     // 0 OPEN  | 90 half  | 120 closed
    HAL_Delay(2000);

    printf("Half fist (90 degrees)...\r\n");
    SetServo1Angle(120); 
    HAL_Delay(100);
    SetServo2Angle(120);
    HAL_Delay(100);
    SetServo3Angle(110); 
    HAL_Delay(100);
    SetServo4Angle(130);
    HAL_Delay(100);
    SetServo5Angle(130);
    HAL_Delay(2000);
    
    printf("Full fist (180 degrees)...\r\n");
    SetServo1Angle(180);
    HAL_Delay(100);
    SetServo2Angle(180);
    HAL_Delay(100);
    SetServo3Angle(150);
    HAL_Delay(100);
    SetServo4Angle(180);
    HAL_Delay(100);
    SetServo5Angle(180);
    HAL_Delay(2000);
    
    printf("Return to open hand...\r\n");
    SetServo1Angle(0);
    SetServo2Angle(0);
    SetServo3Angle(10);
    SetServo4Angle(50);
    SetServo5Angle(0);
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