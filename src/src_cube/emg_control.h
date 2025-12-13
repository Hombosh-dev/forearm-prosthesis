#ifndef EMG_CONTROL_H
#define EMG_CONTROL_H

#include "servo_control.h"

#define EMG_WINDOW_SIZE     50      // MA-50 filter 

#define EMG_THRESHOLD       500     // base threshold for all channels
#define EMG_UPDATE_RATE     200     // 100Hz update (10ms)
#define EMG_HYSTERESIS      70
#define STATE_DEBOUNCE_MS   200

#define TH_CLOSE_BASE       400  
#define TH_THUMB_BASE       350
#define TH_OPEN_BASE        450

#define CH_CLOSE  0         // A0 - 4 fingers closing
#define CH_THUMB  1         // A1 - thumb opening
#define CH_OPEN   2         // A2 - 4 fingers opening

#define STATE_IDLE          0
#define STATE_CLOSE         1
#define STATE_OPEN          2
#define STATE_THUMB         3
#define STATE_HOLD          4 

typedef struct {
    uint8_t current_angle;
    uint8_t target_angle;
    uint8_t min_angle;
    uint8_t max_angle;
} ServoState_t;

extern volatile bool data_rdy_f;
extern uint16_t adc_buffer[];

void EMG_Control_Init(void);
void EMG_Control_Process(void);
void TestServoSequence(void); 
void EMG_AutoCalibrate(void);

#endif