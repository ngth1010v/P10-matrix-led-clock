#include "buzzer/Buzzer.h"

Buzzer::Buzzer(uint8_t pin) 
    : _pin(pin), _currentStep(0), _isPlaying(false), _lastStepTime(0) {
    _stepDuration = SOUND_DURATION / 3;
}

void Buzzer::init() {
    // Configure PWM pin using ESP32 Arduino Core v3+ API
    ledcAttach(_pin, 2000, 8); // Attach pin with initial frequency and 8-bit resolution
    ledcWrite(_pin, 0);        // Keep silent initially
}

void Buzzer::startIntro() {
    // Pitch gets lower / deeper: High -> Mid -> Low
    _frequencies[0] = NOTE_HIGH;
    _frequencies[1] = NOTE_MID;
    _frequencies[2] = NOTE_LOW;

    _currentStep = 0;
    _isPlaying = true;
    _lastStepTime = millis();

    ledcWriteTone(_pin, _frequencies[0]);
}

void Buzzer::startOutro() {
    // Pitch gets higher / less deep: Low -> Mid -> High
    _frequencies[0] = NOTE_LOW;
    _frequencies[1] = NOTE_MID;
    _frequencies[2] = NOTE_HIGH;

    _currentStep = 0;
    _isPlaying = true;
    _lastStepTime = millis();

    ledcWriteTone(_pin, _frequencies[0]);
}

void Buzzer::update() {
    if (!_isPlaying) return;

    if (millis() - _lastStepTime >= _stepDuration) {
        _lastStepTime = millis();
        _currentStep++;

        if (_currentStep < 3) {
            ledcWriteTone(_pin, _frequencies[_currentStep]);
        } else {
            stop(); // Sequence complete
        }
    }
}

void Buzzer::stop() {
    _isPlaying = false;
    _currentStep = 0;
    ledcWriteTone(_pin, 0); // Mute
}