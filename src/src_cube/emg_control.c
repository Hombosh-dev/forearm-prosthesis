#include "emg_control.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    uint16_t buf[EMG_WINDOW_SIZE];
    uint32_t sum;
    uint16_t filtered;

    uint16_t index;
    uint8_t full;
    uint8_t servoAngle; 
    
    uint16_t lastRawValue;       
    uint16_t lastFilteredValue; 
} EMG_Channel_t;

typedef struct {
    EMG_Channel_t close;  
    EMG_Channel_t thumb;  
    EMG_Channel_t open;   
    
    uint8_t servoAngle;
    
    enum {
        STATE_IDLE,
        STATE_FINGERS_CLOSING,
        STATE_FINGERS_OPENING,
        STATE_THUMB_ACTIVE
    } currentState;
    
    uint32_t stateStartTime;
    uint32_t lastGestureTime;
    
} EMG_System_t;

static EMG_System_t emg = {0};

static void init_channel(EMG_Channel_t *ch) {
    memset(ch->buf, 0, sizeof(ch->buf));
    ch->sum = 0;
    ch->filtered = 0;
    ch->index = 0;
    ch->full = 0;
    ch->lastRawValue = 0;
    ch->lastFilteredValue = 0;
}

void EMG_Control_Init(void)
{
    init_channel(&emg.close);
    init_channel(&emg.thumb);
    init_channel(&emg.open);
    
    emg.servoAngle = SERVO_REST_ANGLE;
    emg.currentState = STATE_IDLE;
    emg.stateStartTime = 0;
    emg.lastGestureTime = 0;
    
    printf("3-Channel EMG Control initialized\r\n");
    printf("Channels: Close(A0), Thumb(A1), Open(A2)\r\n");
    printf("Thresholds: Close=%d, Thumb=%d, Open=%d\r\n", 
           EMG_THRESHOLD_CLOSE, EMG_THRESHOLD_THUMB, EMG_THRESHOLD_OPEN);
    printf("Angle range: %d-%d degrees\r\n", SERVO_REST_ANGLE, SERVO_ACTIVE_ANGLE);
}


static void update_channel_filter(EMG_Channel_t *ch, uint16_t rawValue) {
    ch->lastRawValue = rawValue;
    
    uint16_t i = ch->index;
    uint16_t old = ch->buf[i];
    
    ch->buf[i] = rawValue;
    
    if(ch->full) {
        ch->sum += rawValue - old;
    } else {
        ch->sum += rawValue;
    }
    
    uint16_t count = ch->full ? EMG_WINDOW_SIZE : (i + 1);
    ch->filtered = ch->sum / count;
    ch->lastFilteredValue = ch->filtered;
    
    ch->index++;
    if (ch->index >= EMG_WINDOW_SIZE) {
        ch->index = 0;
        ch->full = 1;
    }
}

static void process_fingers_close(void) {
    uint16_t closeSignal = emg.close.filtered;
    uint16_t openSignal = emg.open.filtered;
    
    if (closeSignal > EMG_THRESHOLD_CLOSE && openSignal < EMG_THRESHOLD_OPEN - 100) {
        if (closeSignal > EMG_THRESHOLD_CLOSE + 200) {
            if (emg.servoAngle > 40) emg.servoAngle -= 4;
            else if (emg.servoAngle > 20) emg.servoAngle -= 2;
        } else if (closeSignal > EMG_THRESHOLD_CLOSE) {
            if (emg.servoAngle > 20) emg.servoAngle -= 1;
        }
    }
}

static void process_fingers_open(void) {
    uint16_t openSignal = emg.open.filtered;
    uint16_t closeSignal = emg.close.filtered;
    
    if (openSignal > EMG_THRESHOLD_OPEN && closeSignal < EMG_THRESHOLD_CLOSE - 100) {
        if (openSignal > EMG_THRESHOLD_OPEN + 200) {
            if (emg.servoAngle < 140) emg.servoAngle += 4;
            else if (emg.servoAngle < 160) emg.servoAngle += 2;
        } else if (openSignal > EMG_THRESHOLD_OPEN) {
            if (emg.servoAngle < 160) emg.servoAngle += 1;
        }
    }
}

static void process_thumb(void) {
    uint16_t thumbSignal = emg.thumb.filtered;
    
    if (thumbSignal > EMG_THRESHOLD_THUMB) {
        uint8_t thumbAngle;
        if (thumbSignal > EMG_THRESHOLD_THUMB + 300) {
            thumbAngle = SERVO_ACTIVE_ANGLE; 
        } else if (thumbSignal > EMG_THRESHOLD_THUMB + 150) {
            thumbAngle = (SERVO_ACTIVE_ANGLE + SERVO_REST_ANGLE) / 2; 
        } else {
            thumbAngle = SERVO_REST_ANGLE;  
        }
        SetServo1Angle(thumbAngle);
    } else {
        SetServo1Angle(SERVO_REST_ANGLE);
    }
}


static void update_gesture_state(void) {
    uint16_t closeSignal = emg.close.filtered;
    uint16_t thumbSignal = emg.thumb.filtered;
    uint16_t openSignal = emg.open.filtered;
    
    uint32_t currentTime = HAL_GetTick();
    
    bool closingActive = (closeSignal > EMG_THRESHOLD_CLOSE) && (openSignal < EMG_THRESHOLD_OPEN - 50);
    bool openingActive = (openSignal > EMG_THRESHOLD_OPEN) && (closeSignal < EMG_THRESHOLD_CLOSE - 50);
    bool thumbActive = thumbSignal > EMG_THRESHOLD_THUMB;
    
    if (closingActive && !openingActive && !thumbActive) {
        emg.currentState = STATE_FINGERS_CLOSING;
    } else if (openingActive && !closingActive && !thumbActive) {
        emg.currentState = STATE_FINGERS_OPENING;
    } else if (thumbActive && !closingActive && !openingActive) {
        emg.currentState = STATE_THUMB_ACTIVE;
    } else {
        emg.currentState = STATE_IDLE;
    }
    
    switch (emg.currentState) {
        case STATE_FINGERS_CLOSING:
            process_fingers_close();
            SetServo2Angle(emg.servoAngle);
            SetServo3Angle(emg.servoAngle);
            SetServo4Angle(emg.servoAngle);
            SetServo5Angle(emg.servoAngle);
            break;
            
        case STATE_FINGERS_OPENING:
            process_fingers_open();
            SetServo2Angle(emg.servoAngle);
            SetServo3Angle(emg.servoAngle);
            SetServo4Angle(emg.servoAngle);
            SetServo5Angle(emg.servoAngle);
            break;
            
        case STATE_THUMB_ACTIVE:
            process_thumb();
            SetServo2Angle(emg.servoAngle);
            SetServo3Angle(emg.servoAngle);
            SetServo4Angle(emg.servoAngle);
            SetServo5Angle(emg.servoAngle);
            break;
            
        case STATE_IDLE:
        default:
            if (emg.servoAngle > SERVO_REST_ANGLE) emg.servoAngle--;
            else if (emg.servoAngle < SERVO_REST_ANGLE) emg.servoAngle++;
            
            SetServo1Angle(SERVO_REST_ANGLE);
            SetServo2Angle(emg.servoAngle);
            SetServo3Angle(emg.servoAngle);
            SetServo4Angle(emg.servoAngle);
            SetServo5Angle(emg.servoAngle);
            break;
    }
}

static void send_servo_commands(void) {
    printf("SERVO_CMD: ");
    printf("CLOSE=%u, ", emg.close.lastFilteredValue);
    printf("THUMB=%u, ", emg.thumb.lastFilteredValue);
    printf("OPEN=%u, ", emg.open.lastFilteredValue);
    printf("ANGLE=%d, ", emg.servoAngle);
    
    printf(">CH1:%d,CH2:%d,CH3:%d\r\n", 
           emg.close.lastRawValue, 
           emg.thumb.lastRawValue, 
           emg.open.lastRawValue);
    
    if (emg.close.lastFilteredValue > EMG_THRESHOLD_CLOSE + 100) {
        printf("STATUS=FINGERS_CLOSING");
    } else if (emg.open.lastFilteredValue > EMG_THRESHOLD_OPEN + 100) {
        printf("STATUS=FINGERS_OPENING");
    } else if (emg.thumb.lastFilteredValue > EMG_THRESHOLD_THUMB + 100) {
        printf("STATUS=THUMB_ACTIVE");
    } else {
        printf("STATUS=INACTIVE");
    }
    
    printf("\r\n");
}

void EMG_Control_Process(void) {
    static uint32_t lastUpdate = 0;
    uint32_t now = HAL_GetTick();
    
    if (now - lastUpdate < EMG_UPDATE_RATE)
        return;
    lastUpdate = now;
    
    if (!data_rdy_f)
        return;
    
    int lastSampleIndex = (SAMPLES - 1) * ADC_CHANNELS;
    
    update_channel_filter(&emg.close, adc_buffer[lastSampleIndex + CH_CLOSE]);
    update_channel_filter(&emg.thumb, adc_buffer[lastSampleIndex + CH_THUMB]);
    update_channel_filter(&emg.open, adc_buffer[lastSampleIndex + CH_OPEN]);
    
    printf("EMG_DATA: ");
    printf("CLOSE_RAW=%u, CLOSE_FILT=%u, ", 
           emg.close.lastRawValue, emg.close.lastFilteredValue);
    printf("THUMB_RAW=%u, THUMB_FILT=%u, ",
           emg.thumb.lastRawValue, emg.thumb.lastFilteredValue);
    printf("OPEN_RAW=%u, OPEN_FILT=%u\r\n",
           emg.open.lastRawValue, emg.open.lastFilteredValue);
    
    update_gesture_state();
    
    send_servo_commands();
}
