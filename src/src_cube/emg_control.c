#include "emg_control.h"
#include "main.h"
#include <stdio.h>

typedef struct {
    // uint16_t buf[EMG_WINDOW_SIZE][FIXED_ADC_CHANNELS];
    // uint32_t sum[FIXED_ADC_CHANNELS];
    // uint16_t filtered[FIXED_ADC_CHANNELS];

    uint16_t buf[EMG_WINDOW_SIZE];
    uint32_t sum;
    uint16_t filtered;

    uint16_t index;
    uint8_t full;

    // uint8_t adcChannel;    
    // uint8_t servoChannel;    
    uint8_t servoAngle; 
    uint16_t lastRawValue;     
    uint16_t lastFilteredValue;     
} EMG_t;

static EMG_t emg = {0};


void EMG_Control_Init(void)
{
    emg.index = 0;
    emg.full  = 0;

    emg.adcChannel   = 0;     // CH1
    emg.servoChannel = 0;     // Servo1
    emg.servoAngle   = SERVO_REST_ANGLE;

    printf("EMG Control ready. Threshold = %d\n", EMG_THRESHOLD);
}


static void update_filter(uint16_t* raw)
{
    uint16_t i = emg.index;

    for (int ch = 0; ch < FIXED_ADC_CHANNELS; ch++)
    {
        uint16_t old = emg.buf[i][ch];
        emg.buf[i][ch] = raw[ch];

        emg.sum[ch] += raw[ch] - old;

        uint16_t count = emg.full ? EMG_WINDOW_SIZE : (i + 1);
        emg.filtered[ch] = emg.sum[ch] / count;
    }

    emg.index++;
    if (emg.index >= EMG_WINDOW_SIZE) {
        emg.index = 0;
        emg.full = 1;
    }
}

static void update_servo(uint16_t emgValue)
{
    // -------- HYSTERESIS VALUES --------
    const uint16_t TH_ON  = EMG_THRESHOLD + 150;   // 1150
    const uint16_t TH_OFF = EMG_THRESHOLD - 150;   // 850
    static bool activated = false;

    // -------- STATE MACHINE --------
    if (!activated && emgValue > TH_ON)
        activated = true;

    if (activated && emgValue < TH_OFF)
        activated = false;

    // -------- TARGET ANGLE --------
    uint8_t target;

    if (!activated)
    {
        // Below lower threshold → relax
        target = SERVO_REST_ANGLE;
    }
    else
    {
        // Above upper threshold → active
        uint32_t tmp = (uint32_t)(emgValue - TH_ON) *
                       (SERVO_ACTIVE_ANGLE - SERVO_REST_ANGLE);

        tmp /= (4095 - TH_ON);

        target = SERVO_REST_ANGLE + tmp;
        if (target > SERVO_ACTIVE_ANGLE)
            target = SERVO_ACTIVE_ANGLE;
    }

    // -------- SMOOTH RAMPING --------
    if (emg.servoAngle < target) emg.servoAngle++;
    else if (emg.servoAngle > target) emg.servoAngle--;

    // -------- WRITE SERVO --------
    switch (emg.servoChannel) {
        case 0: SetServo1Angle(emg.servoAngle); break;
        case 1: SetServo2Angle(emg.servoAngle); break;
        case 2: SetServo3Angle(emg.servoAngle); break;
        case 3: SetServo4Angle(emg.servoAngle); break;
        case 4: SetServo5Angle(emg.servoAngle); break;
    }
}

void EMG_Control_Process(void)
{
    static uint32_t last = 0;
    uint32_t now = HAL_GetTick();

    if (now - last < EMG_UPDATE_RATE)
        return;
    last = now;

    if (!data_rdy_f)
        return;

    // ---------- Read latest ADC ----------
    uint16_t raw[3];
    int i = (SAMPLES - 1) * ADC_CHANNELS;

    raw[0] = adc_buffer[i + 0];
    raw[1] = adc_buffer[i + 1];
    raw[2] = adc_buffer[i + 2];

    // ---------- Update filter ----------
    update_filter(raw);

    // ---------- Output for plotting ----------
    printf(">CH1:%u,CH2:%u,CH3:%u\n",
           emg.filtered[0],
           emg.filtered[1],
           emg.filtered[2]);

    // ---------- Smooth servo control ----------
    update_servo(emg.filtered[emg.adcChannel]);
}
