#pragma once

#include <Arduino.h>

#define BUZZER_PIN 26
#define SOUND_DURATION 1000 // Total duration in ms for 3-note sequence

// Configurable pitch frequencies (Hz)
constexpr uint32_t NOTE_LOW  = 262; // C4
constexpr uint32_t NOTE_MID  = 523; // C5
constexpr uint32_t NOTE_HIGH = 1046; // C6

class Buzzer {
private:
    uint8_t  _pin;
    uint32_t _frequencies[3];
    uint8_t  _currentStep;
    bool     _isPlaying;
    uint32_t _stepDuration;
    uint32_t _lastStepTime;

    void stop();

public:
    Buzzer(uint8_t pin = BUZZER_PIN);

    void init();
    void startIntro(); // High -> Mid -> Low (Deepest last)
    void startOutro(); // Low -> Mid -> High (Deepest first)
    
    // Call this repeatedly in main loop() to handle non-blocking sound progression
    void update();
};