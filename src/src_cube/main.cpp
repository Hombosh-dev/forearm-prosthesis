#include "main.h"
#include "periph_init.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>

volatile bool data_rdy_f = false;
uint16_t adc_buffer[ADC_CHANNELS * SAMPLES] = {0}; 

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_DMA_Init();
    MX_ADC1_Init();

    printf("=== 4-CHANNEL DMA SENSOR PLOTTER ===\r\n");
    printf("Channels: PA0, PA1, PA2, PA3\r\n");
    printf("Sample Rate: ~100 kHz per channel\r\n");
    printf("Total Samples: %d\r\n\r\n", ADC_CHANNELS * SAMPLES);

    // start ADC with DMA for 4 channels
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, ADC_CHANNELS * SAMPLES) != HAL_OK)
    {
        Error_Handler();
    }

    while (1)
    {
        if (data_rdy_f)
        {
            printf("PLOT_DATA:");
            for (int i = 0; i < SAMPLES; i += 8) 
            {
                printf("1 %d, 2 %d, 3 %d, 4 %d,\n",
                // printf("%d,%d,%d,%d\r\n",
                    adc_buffer[i * ADC_CHANNELS + 0],  // Channel 0 (PA0)
                    adc_buffer[i * ADC_CHANNELS + 1],  // Channel 1 (PA1)
                    adc_buffer[i * ADC_CHANNELS + 2],  // Channel 2 (PA2)
                    adc_buffer[i * ADC_CHANNELS + 3]); // Channel 3 (PA3)
                
                HAL_Delay(10);
            }
            // printf("END\r\n");
            
            // amplitudes
            // uint16_t min_val[4] = {65535, 65535, 65535, 65535};
            // uint16_t max_val[4] = {0, 0, 0, 0};
            
            // for (int i = 0; i < SAMPLES * ADC_CHANNELS; i += ADC_CHANNELS)
            // {
            //     for (int ch = 0; ch < ADC_CHANNELS; ch++)
            //     {
            //         uint16_t val = adc_buffer[i + ch];
            //         if (val < min_val[ch]) min_val[ch] = val;
            //         if (val > max_val[ch]) max_val[ch] = val;
            //     }
            // }
            
            // printf("AMPLITUDES: %d,%d,%d,%d\r\n",
            //     max_val[0] - min_val[0],
            //     max_val[1] - min_val[1], 
            //     max_val[2] - min_val[2],
            //     max_val[3] - min_val[3]);
            
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            data_rdy_f = false;
            HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, ADC_CHANNELS * SAMPLES);
        }
        HAL_Delay(5);
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
        data_rdy_f = true;
    }
}

void Error_Handler(void)
{
    while(1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(100);
    }
}

extern "C" {
    int _write(int file, char *ptr, int len) {
        HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
        return len;
    }
    void DMA2_Stream0_IRQHandler(void) {
        HAL_DMA_IRQHandler(&hdma_adc1);
    }
}