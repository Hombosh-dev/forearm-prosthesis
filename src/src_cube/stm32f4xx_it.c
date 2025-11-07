#include "stm32f4xx_it.h"

/* ---- Minimal, safe-default handlers ---- */
void NMI_Handler(void)                { /* optional: add logging */ }
void HardFault_Handler(void)          { while (1) { } }
void MemManage_Handler(void)          { while (1) { } }
void BusFault_Handler(void)           { while (1) { } }
void UsageFault_Handler(void)         { while (1) { } }
void SVC_Handler(void)                { }
void DebugMon_Handler(void)           { }
void PendSV_Handler(void)             { }

/* ---- The important one: 1 ms HAL timebase ---- */
void SysTick_Handler(void)
{
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();   // optional but recommended for HAL callbacks
}
