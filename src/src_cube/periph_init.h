#ifndef PERIPH_INIT_H
#define PERIPH_INIT_H

#include "stm32f4xx_hal.h"

extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_tim2_ch2_ch4;
extern TIM_HandleTypeDef htim2;

void PeriphCommonClock_Config(void);
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_ADC1_Init(void);
void MX_DMA_Init(void);
void MX_USART1_UART_Init(void);

void MX_TIM2_Init(void);
#endif // PERIPH_INIT_H
