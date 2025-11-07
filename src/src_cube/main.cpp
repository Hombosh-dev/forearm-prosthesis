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

    printf("=== SENSOR PLOTTER READY ===\r\n");
    printf("Sampling PA0...\r\n\r\n");

    uint32_t sample_count = 0;

    while (1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) 
        {
            uint16_t adc_value = HAL_ADC_GetValue(&hadc1);
            printf("%d\n", adc_value);  
            sample_count++;
        }
        HAL_ADC_Stop(&hadc1);
        
        HAL_Delay(10);  
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
    // void DMA2_Stream0_IRQHandler(void) {
    //     HAL_DMA_IRQHandler(&hdma_adc1);
    // }
}