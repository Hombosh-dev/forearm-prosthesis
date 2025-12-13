# Forearm prosthEsis foR tRaining And self-expeRIence (FERRARI)

## Project Overview
FERRARI is a functional, educational myoelectric prosthesis designed for non-amputee users to experience EMG-based prosthetic control. The system captures forearm muscle signals, processes them in real-time, and actuates a 3D-printed prosthetic hand, demonstrating the complete pipeline from biosignal acquisition to mechanical movement.

## Team
**Ukrainian Catholic University (UCU) Students:**
- Yelyzaveta Bugir
- Oleh Hombosh  
- Yuliia Sova
- Olena Yakovenko

**Mentors:**
- Prof. Johannes Martinek (Technikum Wien)
- Prof. Iris Nemec (Technikum Wien)
- Vladyslav Khmelevskoi (Renesas Electronics)

## System Architecture
### Signal Chain
```
EMG Sensors (3× MyoWare 2.0) -> STM32 Microcontroller -> PCA9685 Servo Controller -> 5× Servo Motors -> 3D-Printed Hand
```

### Key Components
- **Sensors:** 3 MyoWare 2.0 EMG sensors placed on specific forearm muscles
- **Microcontroller:** STM32 microcontroller for real-time signal processing
- **Actuation:** PCA9685 controlling five servo motors (one per finger)
- **Power:** Split power system (6V for motors, 3.3V for electronics) to minimize interference

## Technical Implementation

### EMG Signal Processing Pipeline
```
Raw EMG (1000 Hz) -> MA-50 Filter -> Adaptive Baseline -> Activation Detection -> Cross-talk Resolution -> State Selection -> Servo Control
```

#### Core Algorithms:
1. **Moving Average Filter (MA-50):** Removes high-frequency noise using a 50-sample window buffer optimization
2. **Adaptive Baseline Tracking:** Automatically adjusts to user's resting muscle potential
3. **Hysteresis-based Activation:** Prevents signal jitter with dual thresholds and debouncing
4. **Cross-talk Resolution:** Compares signal amplitudes to distinguish antagonistic muscle activations

### Anatomical Sensor Placement
Based on forearm muscle anatomy for optimal signal differentiation:
- **Sensor 1 (Flexion):** *m. flexor digitorum superficialis* - controls finger closing
- **Sensor 2 (Extension):** *m. extensor digitorum* - controls finger opening  
- **Sensor 3 (Thumb):** *m. abductor pollicis longus* - controls thumb movement

### Control States
The system recognizes four operational states:
- **STATE_IDLE:** Hand relaxed/open
- **STATE_CLOSE:** Hand closing/gripping
- **STATE_OPEN:** Hand opening
- **STATE_THUMB:** Thumb opposition

## Results
- Complete functional prototype from sensors to mechanical hand
- Validated anatomical approach to electrode placement
- Implemented robust filtering and gesture recognition algorithms
- Solved ground interference issues with Common Ground Reference (CGR) strategy

## Project Status
**Completed:**
- Hardware integration (sensors, microcontroller, servos, power)
- Basic gesture recognition (open/close/thumb)
- Mechanical assembly of 3D-printed hand
- Initial signal filtering and processing

## Acknowledgments
Special thanks to Vladyslav Khmelevskoi (Renesas Electronics) and Prof. Johannes Martinek, Prof. Iris Nemec (Technikum Wien) for their invaluable mentorship, feedback, and technical support throughout the project.