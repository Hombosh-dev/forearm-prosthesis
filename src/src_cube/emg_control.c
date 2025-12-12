// emg_control.c - Implementing your exact MA-50 filter
#include "emg_control.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

extern ADC_HandleTypeDef hadc1;

// EXACT replica of your Python EMG_Filter struct
typedef struct {
    uint16_t buf[EMG_WINDOW_SIZE];  // Circular buffer
    uint32_t sum;                    // Running sum
    uint16_t filtered;               // Current filtered value
    uint16_t index;                  // Current buffer index
    uint8_t full;                    // Buffer full flag
    uint16_t last_raw_value;         // Last raw sample
    uint16_t threshold;              // Activation threshold
    uint8_t activated;               // Activation state
    char label;                      // Channel label ('C', 'T', 'O')
    uint16_t baseline;               // Resting baseline
} EMG_Filter_t;

// Three EMG channels (one for each muscle group)
static EMG_Filter_t channels[3];
static uint8_t current_state = STATE_IDLE;
static uint32_t state_change_time = 0;

// === YOUR EXACT FILTER IMPLEMENTATION ===
// This matches your Python EMGFilter.update_filter() exactly
static void update_filter(EMG_Filter_t *filter, uint16_t raw_value) {
    filter->last_raw_value = raw_value;
    
    uint16_t i = filter->index;
    uint16_t old_value = filter->buf[i];
    filter->buf[i] = raw_value;
    
    if (filter->full) {
        // Buffer is full: subtract old, add new
        filter->sum += raw_value - old_value;
    } else {
        // Buffer still filling: just add new value
        filter->sum += raw_value;
    }
    
    // Calculate average - EXACTLY as in Python
    uint16_t count = filter->full ? EMG_WINDOW_SIZE : (filter->index + 1);
    filter->filtered = filter->sum / count;  // Integer division
    
    // Update circular buffer index
    filter->index++;
    if (filter->index >= EMG_WINDOW_SIZE) {
        filter->index = 0;
        filter->full = 1;  // Buffer is now full
    }
}

// Dynamic baseline tracking (automatic calibration)
static void update_baseline(EMG_Filter_t *filter) {
    static uint32_t last_baseline_update[3] = {0};
    uint8_t ch_idx = filter->label - 'C';  // 0=C, 1=T, 2=O
    
    // Update baseline every 2 seconds
    if (HAL_GetTick() - last_baseline_update[ch_idx] > 2000) {
        // Simple low-pass filter for baseline
        filter->baseline = (filter->baseline * 7 + filter->filtered) / 8;
        last_baseline_update[ch_idx] = HAL_GetTick();
    }
}

// Activation detection with hysteresis (like your Python code)
static void detect_activation(EMG_Filter_t *filter) {
    int16_t signal_above = (int16_t)filter->filtered - (int16_t)filter->baseline;
    
    // Your activation logic from Python
    if (signal_above > filter->threshold + EMG_HYSTERESIS) {
        filter->activated = 1;
    } else if (signal_above < filter->threshold - EMG_HYSTERESIS) {
        filter->activated = 0;
    }
    
    // Optional: Debug activation
    static uint32_t last_debug[3] = {0};
    if (HAL_GetTick() - last_debug[filter->label - 'C'] > 2000) {
        printf("CH%c: Base=%d, Filt=%d, Above=%d, Act=%d\r\n",
               filter->label, filter->baseline, filter->filtered, 
               signal_above, filter->activated);
        last_debug[filter->label - 'C'] = HAL_GetTick();
    }
}

void EMG_Control_Init(void) {
    // Initialize Close channel (CH1)
    channels[0].label = 'C';
    channels[0].threshold = TH_CLOSE_BASE;
    channels[0].baseline = 500;  // Initial guess
    
    // Initialize Thumb channel (CH2)
    channels[1].label = 'T';
    channels[1].threshold = TH_THUMB_BASE;
    channels[1].baseline = 450;
    
    // Initialize Open channel (CH3)
    channels[2].label = 'O';
    channels[2].threshold = TH_OPEN_BASE;
    channels[2].baseline = 550;
    
    // Initialize all filters
    for (int i = 0; i < 3; i++) {
        memset(channels[i].buf, 0, sizeof(channels[i].buf));
        channels[i].sum = 0;
        channels[i].filtered = 0;
        channels[i].index = 0;
        channels[i].full = 0;
        channels[i].last_raw_value = 0;
        channels[i].activated = 0;
    }
    
    current_state = STATE_IDLE;
    state_change_time = HAL_GetTick();
    
    printf("=== MA-50 EMG FILTER SYSTEM ===\r\n");
    printf("Window: %d samples, Thresholds: C=%d T=%d O=%d\r\n", 
           EMG_WINDOW_SIZE, TH_CLOSE_BASE, TH_THUMB_BASE, TH_OPEN_BASE);
    printf("Hysteresis: %d\r\n", EMG_HYSTERESIS);
}

void EMG_Calibrate(void) {
    printf("\r\n=== EMG CALIBRATION ===\r\n");
    printf("Keep arm relaxed for 3 seconds...\r\n");
    
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
        
        // Set thresholds slightly above baseline
        channels[0].threshold = channels[0].baseline + 150;
        channels[1].threshold = channels[1].baseline + 120;
        channels[2].threshold = channels[2].baseline + 180;
        
        printf("Calibrated baselines: C=%d, T=%d, O=%d\r\n",
               channels[0].baseline, channels[1].baseline, channels[2].baseline);
        printf("Calibrated thresholds: C=%d, T=%d, O=%d\r\n",
               channels[0].threshold, channels[1].threshold, channels[2].threshold);
    }
}

void EMG_Control_Process(void) {
    static uint32_t last_process = 0;
    static uint32_t last_raw_print = 0;
    static uint32_t last_filter_print = 0;
    uint32_t now = HAL_GetTick();
    
    // Process at 100Hz (10ms) - matches your sample rate
    if (now - last_process < 10) {
        return;
    }
    last_process = now;
    
    if (!data_rdy_f) {
        return;
    }
    
    // Get latest sample from DMA buffer
    int last_idx = (SAMPLES - 1) * ADC_CHANNELS;
    
    // Update each channel with your MA-50 filter
    update_filter(&channels[0], adc_buffer[last_idx + CH_CLOSE]);
    update_filter(&channels[1], adc_buffer[last_idx + CH_THUMB]);
    update_filter(&channels[2], adc_buffer[last_idx + CH_OPEN]);
    
    // Update baselines
    for (int i = 0; i < 3; i++) {
        update_baseline(&channels[i]);
    }
    
    // Detect activations
    for (int i = 0; i < 3; i++) {
        detect_activation(&channels[i]);
    }
    
    // === STATE MACHINE ===
    uint8_t new_state = STATE_IDLE;
    
    // Priority-based state machine (like your Python logic)
    if (channels[0].activated && !channels[2].activated) {
        // Close state - channel C active, O not active
        new_state = STATE_CLOSE;
    } 
    else if (channels[2].activated && !channels[0].activated) {
        // Open state - channel O active, C not active
        new_state = STATE_OPEN;
    }
    else if (channels[1].activated && !channels[0].activated && !channels[2].activated) {
        // Thumb state - only T active
        new_state = STATE_THUMB;
    }
    else {
        // Idle state
        new_state = STATE_IDLE;
    }
    
    // State change with debouncing (100ms)
    if (new_state != current_state) {
        if (now - state_change_time > 100) {
            current_state = new_state;
            state_change_time = now;
            printf("STATE: %s\r\n", 
                   current_state == STATE_CLOSE ? "CLOSE" :
                   current_state == STATE_OPEN ? "OPEN" :
                   current_state == STATE_THUMB ? "THUMB" : "IDLE");
        }
    }
    
    // === SERVO CONTROL ===
    uint8_t servo_angle = 0;
    
    switch(current_state) {
        case STATE_CLOSE: {
            // Map filtered value above baseline to angle
            int16_t signal = channels[0].filtered - channels[0].baseline;
        if (signal < 0) signal = 0;
        
        // Normalize to 0-180 (adjust scale factor based on your EMG range)
        uint8_t normalized = (signal * 180) / 600;  // Adjust 600 based on your max EMG
        
        if (normalized > 180) normalized = 180;
        
        // Use your optimized servo mappings
        SetServo1Normalized(normalized);  // Thumb - uses your mapping
        SetServo2Normalized(normalized);  // Index - uses your mapping
        SetServo3Normalized(normalized);  // Middle - uses your mapping
        SetServo4Normalized(normalized);  // Ring - uses your extended mapping
        SetServo5Normalized(normalized);  // Pinky - uses your adjusted mapping
        break;
    }
    
    case STATE_OPEN: {
        // For opening, we want inverse control
        int16_t signal = channels[2].filtered - channels[2].baseline;
        if (signal < 0) signal = 0;
        
        // Inverse: more signal = more open (0-180, where 180 is fully open)
        uint8_t normalized = 180 - ((signal * 180) / 600);
        if (normalized < 0) normalized = 0;
        
        SetServo1Normalized(normalized);
        SetServo2Normalized(normalized);
        SetServo3Normalized(normalized);
        SetServo4Normalized(normalized);
        SetServo5Normalized(normalized);
        break;
    }
    
    case STATE_THUMB: {
        // Thumb-only control
        int16_t signal = channels[1].filtered - channels[1].baseline;
        if (signal < 0) signal = 0;
        
        uint8_t normalized = (signal * 180) / 600;
        if (normalized > 180) normalized = 180;
        
        // Only move thumb, keep others where they are
        SetServo1Normalized(normalized);
        
        // Optional: you might want to slowly open other fingers
        static uint8_t other_fingers = 0;
        if (other_fingers > 0) other_fingers -= 1;
        
        SetServo2Normalized(other_fingers);
        SetServo3Normalized(other_fingers);
        SetServo4Normalized(other_fingers);
        SetServo5Normalized(other_fingers);
        break;
    }
    
    case STATE_IDLE:
    default: {
        // Slowly open hand completely
        static uint8_t idle_pos = 0;
        if (idle_pos > 0) idle_pos -= 1;
        
        OpenHand();  // Or use SetAllServosNormalized(0) for your optimized open
        break;
    }
}
    
    // === DEBUG OUTPUT ===
    // Print raw data (100Hz)
    if (now - last_raw_print >= 10) {
        printf(">CH1:%d,CH2:%d,CH3:%d\r\n",
               adc_buffer[last_idx + CH_CLOSE],
               adc_buffer[last_idx + CH_THUMB],
               adc_buffer[last_idx + CH_OPEN]);
        last_raw_print = now;
    }
    
    // Print filtered data and state (20Hz)
    if (now - last_filter_print >= 50) {
        printf("EMG_FILT: C=%d(%c) T=%d(%c) O=%d(%c) | ",
               channels[0].filtered, channels[0].activated ? 'A' : 'I',
               channels[1].filtered, channels[1].activated ? 'A' : 'I',
               channels[2].filtered, channels[2].activated ? 'A' : 'I');
        
        // Get current servo angles (simplified)
        uint8_t s1, s2, s3, s4, s5;
        // You might want to track actual servo angles separately
        
        printf("ANGLES: S1=%d S2=%d S3=%d S4=%d S5=%d | ",
               current_state == STATE_THUMB ? servo_angle : 0,
               current_state == STATE_CLOSE ? servo_angle : 0,
               current_state == STATE_CLOSE ? servo_angle : 0,
               current_state == STATE_CLOSE ? servo_angle : 0,
               current_state == STATE_CLOSE ? servo_angle : 0);
        
        printf("STATE=");
        switch(current_state) {
            case STATE_CLOSE: printf("CLOSE"); break;
            case STATE_OPEN: printf("OPEN"); break;
            case STATE_THUMB: printf("THUMB"); break;
            default: printf("IDLE"); break;
        }
        
        printf(" TH=%d\r\n", channels[0].threshold);
        
        last_filter_print = now;
    }
    
    data_rdy_f = false;
}