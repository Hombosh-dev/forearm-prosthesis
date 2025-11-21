#include "emg_control.h"
#include "main.h"
#include <stdio.h>

typedef struct {
    uint16_t buf[EMG_WINDOW_SIZE];
    uint32_t sum;
    uint16_t filtered;

    uint16_t index;
    uint8_t full;
    uint8_t servoAngle; 
    
    uint16_t lastRawValue;       
    uint16_t lastFilteredValue; 
} EMG_t;

static EMG_t emg = {0};


void EMG_Control_Init(void)
{
    emg.index = 0;
    emg.full  = 0;
    emg.sum = 0;
    emg.filtered = 0;
    emg.servoAngle = SERVO_REST_ANGLE;
    
    printf("Single EMG Control initialized\r\n");
    printf("Window=%d, Threshold=%d\r\n", 
           EMG_WINDOW_SIZE, EMG_THRESHOLD);
    printf("Angle range: %d-%d degrees\r\n", SERVO_REST_ANGLE, SERVO_ACTIVE_ANGLE);;
}


static void update_filter(uint16_t rawValue)
{
    emg.lastRawValue = rawValue;
    
    uint16_t i = emg.index;
    uint16_t old = emg.buf[i];
    
    emg.buf[i] = rawValue;
    
    if(emg.full) {
        emg.sum += rawValue - old;
    } else {
        emg.sum += rawValue;
    }
    
    uint16_t count = emg.full ? EMG_WINDOW_SIZE : (i + 1);
    emg.filtered = emg.sum / count;
    emg.lastFilteredValue = emg.filtered;
    
    emg.index++;
    if (emg.index >= EMG_WINDOW_SIZE) {
        emg.index = 0;
        emg.full = 1;
    }
}

static void update_servo_from_emg(void)
{
    const uint16_t TH_ON  = EMG_THRESHOLD + 100;   // top
    const uint16_t TH_OFF = EMG_THRESHOLD - 50;    // bottom with hysteresis
    
    static bool activated = false;
    uint16_t emgValue = emg.filtered;
    
    // hysteresis
    if (!activated && emgValue > TH_ON)
        activated = true;
    if (activated && emgValue < TH_OFF)
        activated = false;
    
    uint8_t targetAngle;
    
    if (!activated) {
        targetAngle = SERVO_REST_ANGLE; 
    } else {
        uint32_t angleRange = SERVO_ACTIVE_ANGLE - SERVO_REST_ANGLE;
        uint32_t signalRange = 4095 - TH_ON; 
        
        if(signalRange > 0 && emgValue > TH_ON) {
            uint32_t tmp = (uint32_t)(emgValue - TH_ON) * angleRange / signalRange;
            targetAngle = SERVO_REST_ANGLE + tmp;
            
            if (targetAngle > SERVO_ACTIVE_ANGLE)
                targetAngle = SERVO_ACTIVE_ANGLE;
        } else {
            targetAngle = SERVO_REST_ANGLE;
        }
    }
    
    if (emg.servoAngle < targetAngle) 
        emg.servoAngle++;
    else if (emg.servoAngle > targetAngle) 
        emg.servoAngle--;
    
    SetServo1Angle(emg.servoAngle); 
    SetServo2Angle(emg.servoAngle);  
    SetServo3Angle(emg.servoAngle); 
    SetServo4Angle(emg.servoAngle);  
    SetServo5Angle(emg.servoAngle); 
}

static void send_servo_commands(void)
{
    printf("SERVO_CMD: ");
    printf("EMG_RAW=%u, ", emg.lastRawValue);
    printf("EMG_FILT=%u, ", emg.lastFilteredValue);
    printf("ANGLE=%d, ", emg.servoAngle);

    printf(">CH1:%d,CH2:%d,CH3:%d\r\n", emg.lastRawValue, 100, 110);
    
    if (emg.lastFilteredValue > EMG_THRESHOLD + 100) {
        printf("STATUS=ACTIVE");
    } else if (emg.lastFilteredValue > EMG_THRESHOLD) {
        printf("STATUS=THRESHOLD");
    } else {
        printf("STATUS=INACTIVE");
    }
    
    printf("\r\n");
}

void EMG_Control_Process(void)
{
    static uint32_t lastUpdate = 0;
    uint32_t now = HAL_GetTick();

    // update freq
    if (now - lastUpdate < EMG_UPDATE_RATE)
        return;
    lastUpdate = now;

    if (!data_rdy_f)
        return;

    // read ch0
    uint16_t rawValue;
    int lastSampleIndex = (SAMPLES - 1) * ADC_CHANNELS;
    rawValue = adc_buffer[lastSampleIndex + ACTIVE_EMG_CHANNEL];

    update_filter(rawValue);
    printf("EMG_DATA: RAW=%u, FILTERED=%u\r\n", 
           emg.lastRawValue, emg.lastFilteredValue);

    update_servo_from_emg();
    send_servo_commands();
}
