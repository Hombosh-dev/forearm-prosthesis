#ifndef HAL_UART_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#endif

#include "main.h"
#include "pca9685.h"
#include "periph_init.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>

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

    // ScanI2CDevices();
    // ScanPCA9685Addresses();
    // TestI2CBasic();

    // Initialize PCA9685 for servo control (60Hz for servos)
    if (PCA9685_Init(&pca9685, &hi2c1, PCA9685_I2C_ADDRESS, 60.0))
    {
        printf("PCA9685 initialized successfully\r\n");

        // Test servo on channel 0
        TestServo();
    }
    else
    {
        printf("PCA9685 initialization failed!\r\n");
        printf("Check I2C connections and address.\r\n");

        uint8_t alt_addresses[] = { 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47 };
        for (int i = 0; i < 7; i++)
        {
            if (PCA9685_Init(&pca9685, &hi2c1, alt_addresses[i], 60.0))
            {
                printf("PCA9685 found at 0x%02X and initialized!\r\n", alt_addresses[i]);
                TestServo();
                break;
            }
        }
    }

    // start ADC with DMA for 4 channels
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, ADC_CHANNELS * SAMPLES) != HAL_OK)
    {
        Error_Handler();
    }

    while (1)
    {
        if (data_rdy_f)
        {
            printf("PLOT_DATA:");
            // for (int i = 0; i < SAMPLES; i += 8)
            // {
            // printf("1 %d, 2 %d, 3 %d, 4 %d,\n",
            // printf(">CH1:%d,CH2:%d,CH3:%d,CH4:%d\r\n",
            //     adc_buffer[i * ADC_CHANNELS + 0],  // Channel 0 (PA0)
            //     adc_buffer[i * ADC_CHANNELS + 1],  // Channel 1 (PA1)
            //     adc_buffer[i * ADC_CHANNELS + 2],  // Channel 2 (PA2)
            //     adc_buffer[i * ADC_CHANNELS + 3]); // Channel 3 (PA3)

            // HAL_Delay(10);
            // }

            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            data_rdy_f = false;
            HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, ADC_CHANNELS * SAMPLES);
        }
        HAL_Delay(10);
    }
}

void ScanI2CDevices(void)
{
    printf("=== I2C BUS SCAN ===\r\n");

    HAL_StatusTypeDef status;
    uint8_t found_devices = 0;

    printf("Scanning I2C addresses from 0x08 to 0x77...\r\n");

    for (uint8_t address = 8; address < 120; address++)
    {
        status = HAL_I2C_IsDeviceReady(&hi2c1, address << 1, 3, 10);

        if (status == HAL_OK)
        {
            printf("I2C device found at address: 0x%02X", address);
            found_devices++;

            // Check if it's our PCA9685
            if (address == PCA9685_I2C_ADDRESS)
            {
                printf(" <- PCA9685 DETECTED!");
            }
            printf("\r\n");
        }
    }

    if (found_devices == 0)
    {
        printf("No I2C devices found!\r\n");
        printf("Troubleshooting:\r\n");
        printf("1. Check SDA (PB7) and SCL (PB6) connections\r\n");
        printf("2. Verify PCA9685 power (3.3V/5V)\r\n");
        printf("3. Check common GND between STM32 and PCA9685\r\n");
        printf("4. Verify I2C pull-up resistors (4.7kΩ to 3.3V)\r\n");
    }
    else
    {
        printf("Total I2C devices found: %d\r\n", found_devices);
    }
    printf("\r\n");
}
void ScanPCA9685Addresses(void)
{
    printf("=== PCA9685 ADDRESS SCAN ===\r\n");

    // PCA9685 base address is 0x40, but can be 0x40-0x47 depending on A0-A5 pins
    uint8_t pca_addresses[] = { 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47 };

    for (int i = 0; i < 8; i++)
    {
        HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&hi2c1, pca_addresses[i] << 1, 3, 10);
        if (status == HAL_OK)
        {
            printf("PCA9685 found at alternative address: 0x%02X\r\n", pca_addresses[i]);
        }
    }
    printf("\r\n");
}
void TestI2CBasic(void)
{
    printf("=== BASIC I2C TEST ===\r\n");

    uint8_t test_data = 0;
    uint8_t mode1_reg = 0x00;

    // Try to read MODE1 register from PCA9685
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1, PCA9685_I2C_ADDRESS << 1, mode1_reg,
                                                I2C_MEMADD_SIZE_8BIT, &test_data, 1, 100);

    if (status == HAL_OK)
    {
        printf("Successfully read PCA9685 register 0x%02X: 0x%02X\r\n", mode1_reg, test_data);
    }
    else
    {
        printf("Failed to read PCA9685 register (Error: %d)\r\n", status);
        printf("Possible causes:\r\n");
        printf("- Wrong I2C address (try 0x40-0x47)\r\n");
        printf("- Missing pull-up resistors\r\n");
        printf("- PCA9685 not powered\r\n");
        printf("- SDA/SCL lines swapped\r\n");
    }
    printf("\r\n");
}

void TestServo(void)
{
    printf("Testing servo on channel 0...\r\n");

    // Move to 0 degrees
    PCA9685_SetServoAngle(&pca9685, 0, 0);
    printf("Servo at 0 degrees\r\n");
    HAL_Delay(1000);

    // Move to 90 degrees
    PCA9685_SetServoAngle(&pca9685, 0, 90);
    printf("Servo at 90 degrees\r\n");
    HAL_Delay(1000);

    // Move to 180 degrees
    PCA9685_SetServoAngle(&pca9685, 0, 180);
    printf("Servo at 180 degrees\r\n");
    HAL_Delay(1000);

    // Back to 90 degrees
    PCA9685_SetServoAngle(&pca9685, 0, 90);
    printf("Servo at 90 degrees - Test complete\r\n");
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