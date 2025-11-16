#ifndef EMG_CONTROL_H
#define EMG_CONTROL_H

#include "servo_control.h"

// EMG Control Configuration
#define EMG_WINDOW_SIZE 20        // 20 samples ≈ 200ms at 100Hz
#define EMG_THRESHOLD 300         // Activation threshold (0-4095 for 12-bit ADC)
#define SERVO_MIN_ANGLE 20        // Rest position
#define SERVO_MAX_ANGLE 160       // Maximum flexion position
#define EMG_UPDATE_RATE 10        // Update rate in ms (100Hz)
#define FIXED_ADC_CHANNELS 3

// External declarations for main.cpp variables
extern volatile bool data_rdy_f;
extern uint16_t adc_buffer[];

// Function prototypes
void EMG_Control_Init(void);
void EMG_Control_Process(void);
uint16_t EMG_GetFilteredValue(uint8_t channel);
void EMG_SetThreshold(uint16_t threshold);
uint16_t EMG_GetThreshold(void);
void EMG_SetControlChannel(uint8_t servo_ch, uint8_t adc_ch);

#endif // EMG_CONTROL_H