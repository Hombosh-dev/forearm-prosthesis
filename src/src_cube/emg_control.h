#ifndef EMG_CONTROL_H
#define EMG_CONTROL_H

#include "servo_control.h"

#define EMG_WINDOW_SIZE     100
#define FIXED_ADC_CHANNELS  3

#define SERVO_REST_ANGLE    20
#define SERVO_ACTIVE_ANGLE  160

#define EMG_THRESHOLD       1000     
#define EMG_UPDATE_RATE     10       // 100 Hz

extern volatile bool data_rdy_f;
extern uint16_t adc_buffer[];

void EMG_Control_Init(void);
void EMG_Control_Process(void);

#endif
