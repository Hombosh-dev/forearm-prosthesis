#ifndef EMG_CONTROL_H
#define EMG_CONTROL_H

#include "servo_control.h"

#define EMG_WINDOW_SIZE     50      // MA-50 filter you liked

#define EMG_THRESHOLD       500     // Base threshold for all channels
#define EMG_UPDATE_RATE     200      // 100Hz update (10ms)
#define EMG_HYSTERESIS      50

#define TH_CLOSE_BASE       400     // Increase threshold
#define TH_THUMB_BASE       350
#define TH_OPEN_BASE        450

// Channel mappings
#define CH_CLOSE  0  // A0 - Fingers closing
#define CH_THUMB  1  // A1 - Thumb opening
#define CH_OPEN   2  // A2 - Fingers opening

#define STATE_IDLE          0
#define STATE_CLOSE         1
#define STATE_OPEN          2
#define STATE_THUMB         3

extern volatile bool data_rdy_f;
extern uint16_t adc_buffer[];

void EMG_Control_Init(void);
void EMG_Control_Process(void);
void TestServoSequence(void);  // Add this

#endif