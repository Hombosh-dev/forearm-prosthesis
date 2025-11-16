#include "emg_control.h"
#include "main.h"
#include <stdio.h>

// EMG Control structure with separate filters for each channel
typedef struct {
    uint16_t windowBuf[EMG_WINDOW_SIZE][FIXED_ADC_CHANNELS];  // Circular buffer for moving average
    uint32_t windowSum[FIXED_ADC_CHANNELS];                   // Sum of values in window
    uint16_t windowIndex;                                     // Current buffer index
    uint8_t windowFull;                                       // Flag indicating buffer is full
    uint16_t filteredValue[FIXED_ADC_CHANNELS];               // Current filtered EMG values
    uint8_t servoChannel;                                     // Which servo to control (0-4)
    uint8_t adcChannel;                                       // Which ADC channel to read (0-2)
} EMG_Control_t;

static EMG_Control_t emgControl = {0};
static uint16_t emgThreshold = EMG_THRESHOLD;

void EMG_Control_Init(void) {
    // Initialize EMG control structure
    for (int i = 0; i < EMG_WINDOW_SIZE; i++) {
        for (int ch = 0; ch < FIXED_ADC_CHANNELS; ch++) {
            emgControl.windowBuf[i][ch] = 0;
        }
    }
    for (int ch = 0; ch < FIXED_ADC_CHANNELS; ch++) {
        emgControl.windowSum[ch] = 0;
        emgControl.filteredValue[ch] = 0;
    }
    emgControl.windowIndex = 0;
    emgControl.windowFull = 0;
    emgControl.servoChannel = 0;  // Control thumb servo by default
    emgControl.adcChannel = 0;    // Use ADC channel 0 (PA0) by default
    
    printf("EMG Control initialized\r\n");
    printf("Window size: %d, Threshold: %d\r\n", EMG_WINDOW_SIZE, emgThreshold);
}

void EMG_Control_Process(void) {
    static uint32_t lastUpdate = 0;
    static uint32_t sampleCounter = 0;
    uint32_t currentTime = HAL_GetTick();
    
    // Process at specified rate
    if (currentTime - lastUpdate < EMG_UPDATE_RATE) {
        return;
    }
    lastUpdate = currentTime;
    
    // 1. Get raw values from ADC buffer (use latest sample)
    uint16_t ch1_raw = 0, ch2_raw = 0, ch3_raw = 0;
    
    if (data_rdy_f) {
        // Use the most recent sample (last in buffer)
        int sampleIndex = (SAMPLES - 1) * ADC_CHANNELS;
        if (sampleIndex < ADC_CHANNELS * SAMPLES) {
            ch1_raw = adc_buffer[sampleIndex + 0];  // Channel 1 (PA0)
            ch2_raw = adc_buffer[sampleIndex + 1];  // Channel 2 (PA1) 
            ch3_raw = adc_buffer[sampleIndex + 2];  // Channel 3 (PA2)
        }
    } else {
        return; // No new data available
    }
    
    // 2. Update moving average filters for all channels
    uint16_t raw_values[FIXED_ADC_CHANNELS];
    raw_values[0] = ch1_raw;
    raw_values[1] = ch2_raw;
    raw_values[2] = ch3_raw;
    
    for (int ch = 0; ch < FIXED_ADC_CHANNELS; ch++) {
        // Subtract the value that will be overwritten
        emgControl.windowSum[ch] -= emgControl.windowBuf[emgControl.windowIndex][ch];
        
        // Store new value in buffer
        emgControl.windowBuf[emgControl.windowIndex][ch] = raw_values[ch];
        
        // Add new value to sum
        emgControl.windowSum[ch] += raw_values[ch];
        
        // Calculate filtered value
        if (emgControl.windowFull) {
            emgControl.filteredValue[ch] = emgControl.windowSum[ch] / EMG_WINDOW_SIZE;
        } else {
            uint16_t count = emgControl.windowIndex;
            if (count == 0) count = 1;
            emgControl.filteredValue[ch] = emgControl.windowSum[ch] / count;
        }
    }
    
    // Move to next position in circular buffer
    emgControl.windowIndex++;
    if (emgControl.windowIndex >= EMG_WINDOW_SIZE) {
        emgControl.windowIndex = 0;
        emgControl.windowFull = 1;
    }
    
    // 3. Output FILTERED values in serial plotter format
    printf(">CH1:%d,CH2:%d,CH3:%d\r\n", 
           emgControl.filteredValue[0], 
           emgControl.filteredValue[1], 
           emgControl.filteredValue[2]);
    
    // 4. Control servo based on filtered EMG signal from selected channel
    sampleCounter++;
    if (sampleCounter % 10 == 0) {
        uint8_t servoPosition = SERVO_MIN_ANGLE; // Default rest position
        uint16_t emgValue = emgControl.filteredValue[emgControl.adcChannel];
        
        if (emgValue >= emgThreshold) {
            // Map filtered value from [threshold..4095] to [SERVO_MIN_ANGLE..SERVO_MAX_ANGLE]
            if (emgThreshold < 4095) {
                servoPosition = SERVO_MIN_ANGLE + 
                               ((emgValue - emgThreshold) * 
                               (SERVO_MAX_ANGLE - SERVO_MIN_ANGLE)) / 
                               (4095 - emgThreshold);
            }
            
            // Constrain to valid range
            if (servoPosition < SERVO_MIN_ANGLE) servoPosition = SERVO_MIN_ANGLE;
            if (servoPosition > SERVO_MAX_ANGLE) servoPosition = SERVO_MAX_ANGLE;
        }
        
        // Update servo position based on configured channel
        switch (emgControl.servoChannel) {
            case 0: SetServo1Angle(servoPosition); break;
            case 1: SetServo2Angle(servoPosition); break;
            case 2: SetServo3Angle(servoPosition); break;
            case 3: SetServo4Angle(servoPosition); break;
            case 4: SetServo5Angle(servoPosition); break;
            default: SetServo1Angle(servoPosition); break;
        }
        
        // Optional: Print EMG debug info occasionally
        if (sampleCounter % 50 == 0) {
            printf("// EMG: raw=%4d, avg=%4d, servo=%3d, thr=%d\r\n", 
                   raw_values[emgControl.adcChannel], emgValue, servoPosition, emgThreshold);
        }
    }
}

uint16_t EMG_GetFilteredValue(uint8_t channel) {
    if (channel < FIXED_ADC_CHANNELS) {
        return emgControl.filteredValue[channel];
    }
    return 0;
}

void EMG_SetThreshold(uint16_t threshold) {
    emgThreshold = threshold;
    if (emgThreshold > 4095) emgThreshold = 4095;
    printf("// EMG threshold set to: %d\r\n", emgThreshold);
}

uint16_t EMG_GetThreshold(void) {
    return emgThreshold;
}

void EMG_SetControlChannel(uint8_t servo_ch, uint8_t adc_ch) {
    if (servo_ch < 5) emgControl.servoChannel = servo_ch;
    if (adc_ch < FIXED_ADC_CHANNELS) emgControl.adcChannel = adc_ch;
    printf("// EMG control: Servo=%d, ADC=%d\r\n", emgControl.servoChannel, emgControl.adcChannel);
}