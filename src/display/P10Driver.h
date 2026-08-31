
#ifndef P10_DRIVER_H
#define P10_DRIVER_H

#include <Arduino.h>

#define PIN_OE   22
#define PIN_A    19
#define PIN_B    21
#define PIN_CLK  18
#define PIN_LAT  5
#define PIN_DR   23

class P10Driver {

private:

    struct MatrixPosition {
        uint8_t row;
        uint pos;
    };

    // Logical framebuffer
    bool buffer[64][16] = {};

    // Brightness / display time
    uint brightness = 1000;

    // Double buffering
    bool displayRows[2][4][64 * 16 / 4] = {};

    volatile uint8_t activeBufferIdx = 0;

    bool isInitialized = false;

    TaskHandle_t refreshTaskHandle = NULL;

    // Hardware control
    void selectRow(uint8_t row);
    void writeLed(bool on);
    void writeRow(uint8_t row, const bool* data);

    // Coordinate mapper
    MatrixPosition BufferPosToDisplayPos(uint x, uint y);

    // FreeRTOS task entry point
    static void refreshTask(void* pvParameters);

public:

    P10Driver() = default;

    // Initialize P10 driver
    void init();

    // Edit logical framebuffer
    void set(uint x, uint y, bool on);

    // Set display brightness
    void setBrightness(uint value);

    // Commit framebuffer to display
    void flush();
};

#endif // P10_DRIVER_H