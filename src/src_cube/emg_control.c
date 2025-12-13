#include "emg_control.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

extern ADC_HandleTypeDef hadc1;

typedef struct {
    uint16_t buf[EMG_WINDOW_SIZE];
    uint32_t sum;
    uint16_t filtered;
    uint16_t index;
    uint8_t full;
    uint16_t baseline;
    uint16_t threshold;
    uint8_t activated;
    uint8_t activation_count;  
    uint8_t deactivation_count;
    char label;
} EMG_Filter_t;

EMG_Filter_t channels[3];
ServoState_t servo_states[5] = {
    {0, 0, 0, 150},      // thumb
    {0, 0, 0, 180},      // index
    {10, 10, 10, 170},   // middle
    {20, 20, 20, 180},   // ring
    {0, 0, 0, 120}       // pinky
};

static uint8_t current_state = STATE_IDLE;
static uint32_t state_change_time = 0;
static uint32_t last_emg_activity = 0;

static void update_filter(EMG_Filter_t *filter, uint16_t raw_value) {
    uint16_t i = filter->index;
    uint16_t old_value = filter->buf[i];
    filter->buf[i] = raw_value;
    
    if (filter->full) {
        filter->sum += raw_value - old_value;
    } else {
        filter->sum += raw_value;
    }
    
    uint16_t count = filter->full ? EMG_WINDOW_SIZE : (filter->index + 1);
    filter->filtered = filter->sum / count;
    
    filter->index = (filter->index + 1) % EMG_WINDOW_SIZE;
    if (filter->index == 0) filter->full = 1;
}

static void update_baseline(EMG_Filter_t *filter) {
    static uint32_t last_update[3] = {0};
    uint8_t ch_idx = filter->label - 'C';
    
    if (!filter->activated && (HAL_GetTick() - last_update[ch_idx] > 3000)) {
        filter->baseline = (filter->baseline * 15 + filter->filtered) / 16;
        last_update[ch_idx] = HAL_GetTick();
    }
}

static void detect_activation(EMG_Filter_t *filter) {
    int16_t signal_above = (int16_t)filter->filtered - (int16_t)filter->baseline;
    
    if (signal_above > filter->threshold + EMG_HYSTERESIS) {
        filter->activation_count++;
        filter->deactivation_count = 0;
        
        if (filter->activation_count >= 3) {
            filter->activated = 1;
        }
    } 
    else if (signal_above < filter->threshold - EMG_HYSTERESIS) {
        filter->deactivation_count++;
        filter->activation_count = 0;
        
        if (filter->deactivation_count >= 5) {
            filter->activated = 0;
        }
    } else {
        filter->activation_count = 0;
        filter->deactivation_count = 0;
    }
}

static uint8_t calculate_confidence(uint8_t desired_state) {
    uint8_t confidence = 0;
    
    switch(desired_state) {
        case STATE_CLOSE:
            if (channels[0].activated && !channels[2].activated) {
                confidence = 100;
            } 
            else if (channels[0].activated && channels[2].activated) {
                int16_t close_signal = channels[0].filtered - channels[0].baseline;
                int16_t open_signal = channels[2].filtered - channels[2].baseline;
                if (close_signal > open_signal * 1.5) {
                    confidence = 70;
                }
            }
            break;
            
        case STATE_OPEN:
            if (channels[2].activated && !channels[0].activated) {
                confidence = 100;
            }
            else if (channels[2].activated && channels[0].activated) {
                int16_t close_signal = channels[0].filtered - channels[0].baseline;
                int16_t open_signal = channels[2].filtered - channels[2].baseline;
                if (open_signal > close_signal * 1.5) {
                    confidence = 70;
                }
            }
            break;
            
        case STATE_THUMB:
            if (channels[1].activated && !channels[0].activated && !channels[2].activated) {
                confidence = 100;
            }
            break;
    }
    
    return confidence;
}

void EMG_Control_Init(void) {
    // initialize Close channel (CH1)
    channels[0].label = 'C';
    channels[0].threshold = TH_CLOSE_BASE;
    channels[0].baseline = 500;
    
    // initialize Thumb channel (CH2)
    channels[1].label = 'T';
    channels[1].threshold = TH_THUMB_BASE;
    channels[1].baseline = 450;
    
    // initialize Open channel (CH3)
    channels[2].label = 'O';
    channels[2].threshold = TH_OPEN_BASE;
    channels[2].baseline = 550;
    
    // initialize all filters
    for (int i = 0; i < 3; i++) {
        memset(channels[i].buf, 0, sizeof(channels[i].buf));
        channels[i].sum = 0;
        channels[i].filtered = 0;
        channels[i].index = 0;
        channels[i].full = 0;
        channels[i].baseline = 500 + (i * 50);
        channels[i].activated = 0;
        channels[i].activation_count = 0;
        channels[i].deactivation_count = 0;
    }
    
    current_state = STATE_IDLE;
    state_change_time = HAL_GetTick();
    last_emg_activity = HAL_GetTick();
    
    printf("=== EMG CONTROL ===\r\n");
    printf("Thresholds: C=%d T=%d O=%d\r\n", TH_CLOSE_BASE, TH_THUMB_BASE, TH_OPEN_BASE);
    printf("Hysteresis: %d, Debounce: %dms\r\n", EMG_HYSTERESIS, STATE_DEBOUNCE_MS);
}

void EMG_AutoCalibrate(void) {
    printf("\r\n=== AUTO-CALIBRATION ===\r\n");
    printf("Please relax your arm for 3 seconds...\r\n");
    
    uint32_t start_time = HAL_GetTick();
    uint32_t sums[3] = {0};
    uint16_t count = 0;
    
    while (HAL_GetTick() - start_time < 3000) {
        if (data_rdy_f) {
            int last_idx = (SAMPLES - 1) * ADC_CHANNELS;
            sums[0] += adc_buffer[last_idx + CH_CLOSE];
            sums[1] += adc_buffer[last_idx + CH_THUMB];
            sums[2] += adc_buffer[last_idx + CH_OPEN];
            count++;
            data_rdy_f = false;
        }
        HAL_Delay(1);
    }
    
    if (count > 0) {
        channels[0].baseline = sums[0] / count;
        channels[1].baseline = sums[1] / count;
        channels[2].baseline = sums[2] / count;
        
        channels[0].threshold = channels[0].baseline + 300;
        channels[1].threshold = channels[1].baseline + 250;
        channels[2].threshold = channels[2].baseline + 350;
        
        printf("Calibrated: C=%d(+%d), T=%d(+%d), O=%d(+%d)\r\n",
               channels[0].baseline, channels[0].threshold - channels[0].baseline,
               channels[1].baseline, channels[1].threshold - channels[1].baseline,
               channels[2].baseline, channels[2].threshold - channels[2].baseline);
    }
}

void EMG_Control_Process(void) {
    static uint32_t last_process = 0;
    static uint32_t last_print = 0;
    uint32_t now = HAL_GetTick();
    
    // process at 50Hz (20ms)
    if (now - last_process < 20) {
        return;
    }
    last_process = now;
    
    if (!data_rdy_f) {
        return;
    }
    
    int last_idx = (SAMPLES - 1) * ADC_CHANNELS;
    
    update_filter(&channels[0], adc_buffer[last_idx + CH_CLOSE]);
    update_filter(&channels[1], adc_buffer[last_idx + CH_THUMB]);
    update_filter(&channels[2], adc_buffer[last_idx + CH_OPEN]);
    
    // update baselines
    for (int i = 0; i < 3; i++) {
        update_baseline(&channels[i]);
    }
    
    // detect activations with persistence
    for (int i = 0; i < 3; i++) {
        detect_activation(&channels[i]);
    }
    
    uint8_t new_state = STATE_IDLE;
    uint8_t close_confidence = calculate_confidence(STATE_CLOSE);
    uint8_t open_confidence = calculate_confidence(STATE_OPEN);
    uint8_t thumb_confidence = calculate_confidence(STATE_THUMB);
    
    // state selection with confidence thresholds
    if (close_confidence >= 70) {
        new_state = STATE_CLOSE;
    } else if (open_confidence >= 70) {
        new_state = STATE_OPEN;
    } else if (thumb_confidence >= 80) { 
        new_state = STATE_THUMB;
    } else {
        new_state = STATE_IDLE;
    }
    
    // check for EMG activity timeout
    uint8_t any_activity = channels[0].activated || channels[1].activated || channels[2].activated;
    if (any_activity) {
        last_emg_activity = now;
    } else if (now - last_emg_activity > 2000) {
        // no activity for 2 seconds - force idle
        new_state = STATE_IDLE;
    }
    
    // state change with debounce
    if (new_state != current_state) {
        if (now - state_change_time > STATE_DEBOUNCE_MS) {
            printf("STATE: %s (C:%d%%, O:%d%%, T:%d%%)\r\n",
                   new_state == STATE_CLOSE ? "CLOSE" :
                   new_state == STATE_OPEN ? "OPEN" :
                   new_state == STATE_THUMB ? "THUMB" : "IDLE",
                   close_confidence, open_confidence, thumb_confidence);
            
            current_state = new_state;
            state_change_time = now;
        }
    }
    
    switch(current_state) {
        case STATE_CLOSE: {
            int16_t signal = channels[0].filtered - channels[0].baseline;
            if (signal < 0) signal = 0;
            
            // map to angle with smoothing
            uint8_t target = (signal * 180) / 800;
            if (target > 180) target = 180;
            
            // smooth movement toward target
            for (int i = 0; i < 5; i++) {
                if (servo_states[i].target_angle < target) {
                    servo_states[i].target_angle += 2;
                } else if (servo_states[i].target_angle > target) {
                    servo_states[i].target_angle -= 2;
                }
                
                if (servo_states[i].target_angle > servo_states[i].max_angle) {
                    servo_states[i].target_angle = servo_states[i].max_angle;
                }
            }
            break;
        }
        
        case STATE_OPEN: {
            int16_t signal = channels[2].filtered - channels[2].baseline;
            if (signal < 0) signal = 0;
            
            // inverse mapping for opening
            uint8_t target = 180 - ((signal * 180) / 800);
            if (target > 180) target = 0;
            
            // smooth movement toward target
            for (int i = 0; i < 5; i++) {
                if (servo_states[i].target_angle > target) {
                    servo_states[i].target_angle -= 3;
                } else if (servo_states[i].target_angle < target) {
                    servo_states[i].target_angle += 1;
                }
                
                if (servo_states[i].target_angle < servo_states[i].min_angle) {
                    servo_states[i].target_angle = servo_states[i].min_angle;
                }
            }
            break;
        }
        
        case STATE_THUMB: {
            int16_t signal = channels[1].filtered - channels[1].baseline;
            if (signal < 0) signal = 0;
            
            uint8_t thumb_target = (signal * 150) / 800;
            if (thumb_target > 150) thumb_target = 150;
            
            // smooth thumb movement
            if (servo_states[0].target_angle < thumb_target) {
                servo_states[0].target_angle += 2;
            } else if (servo_states[0].target_angle > thumb_target) {
                servo_states[0].target_angle -= 2;
            }
            
            // slowly open other fingers
            for (int i = 1; i < 5; i++) {
                if (servo_states[i].target_angle > 0) {
                    servo_states[i].target_angle -= 1;
                }
            }
            break;
        }
        
        case STATE_IDLE:
        default: {
            // slowly open hand completely when idle
            for (int i = 0; i < 5; i++) {
                if (servo_states[i].target_angle > servo_states[i].min_angle) {
                    servo_states[i].target_angle -= 1;
                }
                // never go below minimum
                if (servo_states[i].target_angle < servo_states[i].min_angle) {
                    servo_states[i].target_angle = servo_states[i].min_angle;
                }
            }
            break;
        }
    }
    
    SetServo1Angle(servo_states[0].target_angle);
    SetServo2Angle(servo_states[1].target_angle);
    SetServo3Angle(servo_states[2].target_angle);
    SetServo4Angle(servo_states[3].target_angle);
    SetServo5Angle(servo_states[4].target_angle);
    
    servo_states[0].current_angle = servo_states[0].target_angle;
    servo_states[1].current_angle = servo_states[1].target_angle;
    servo_states[2].current_angle = servo_states[2].target_angle;
    servo_states[3].current_angle = servo_states[3].target_angle;
    servo_states[4].current_angle = servo_states[4].target_angle;
    
    if (now - last_print >= 100) {  // every 100ms
        printf(">CH1:%d,CH2:%d,CH3:%d\r\n",
               adc_buffer[last_idx + CH_CLOSE],
               adc_buffer[last_idx + CH_THUMB],
               adc_buffer[last_idx + CH_OPEN]);
        
        printf("EMG_FILT: C=%d(%c) T=%d(%c) O=%d(%c) | ",
               channels[0].filtered, channels[0].activated ? 'A' : 'I',
               channels[1].filtered, channels[1].activated ? 'A' : 'I',
               channels[2].filtered, channels[2].activated ? 'A' : 'I');
        
        printf("ANGLES: S1=%d S2=%d S3=%d S4=%d S5=%d | ",
               servo_states[0].current_angle,
               servo_states[1].current_angle,
               servo_states[2].current_angle,
               servo_states[3].current_angle,
               servo_states[4].current_angle);
        
        printf("STATE=");
        switch(current_state) {
            case STATE_CLOSE: printf("CLOSE"); break;
            case STATE_OPEN: printf("OPEN"); break;
            case STATE_THUMB: printf("THUMB"); break;
            default: printf("IDLE"); break;
        }
        
        printf(" TH=%d\r\n", TH_CLOSE_BASE);
        
        last_print = now;
    }
    
    data_rdy_f = false;
}