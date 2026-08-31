
#include "display/P10Driver.h"


void P10Driver::selectRow(uint8_t row) {

    digitalWrite(PIN_A, (row & 0x01) ? HIGH : LOW);
    digitalWrite(PIN_B, (row & 0x02) ? HIGH : LOW);
}


void P10Driver::writeLed(bool on) {

    // P10 Active LOW:
    // true  -> LOW  -> LED ON
    // false -> HIGH -> LED OFF

    digitalWrite(PIN_DR, on ? LOW : HIGH);

    digitalWrite(PIN_CLK, HIGH);
    digitalWrite(PIN_CLK, LOW);
}


void P10Driver::writeRow(uint8_t row, const bool* data) {

    // Disable output while shifting data
    digitalWrite(PIN_OE, HIGH);

    // 64 x 16 panel, 1/4 scan
    // 64 * 16 / 4 = 256 bits
    for (uint i = 0; i < 64 * 16 / 4; i++) {
        writeLed(data[i]);
    }

    // Latch shifted data
    digitalWrite(PIN_LAT, HIGH);
    digitalWrite(PIN_LAT, LOW);

    // Select scan row
    selectRow(row);

    // Dead time
    delayMicroseconds(5);

    // Enable output
    digitalWrite(PIN_OE, LOW);

    // Display time
    delayMicroseconds(brightness);

    // Disable output before next row
    digitalWrite(PIN_OE, HIGH);
}


P10Driver::MatrixPosition
P10Driver::BufferPosToDisplayPos(uint x, uint y) {

    MatrixPosition res;

    res.row = y % 4;

    uint xOffset =
        (((uint)(x / 8)) * 8 * 4)
        + (x % 8);

    uint yOffset =
        (3 - ((uint)(y / 4))) * 8;

    res.pos = xOffset + yOffset;

    return res;
}


void P10Driver::refreshTask(void* pvParameters) {

    P10Driver* instance =
        static_cast<P10Driver*>(pvParameters);

    while (true) {

        // Read current active buffer
        uint8_t readIdx = instance->activeBufferIdx;

        // Refresh all 4 scan rows
        for (uint8_t i = 0; i < 4; i++) {

            instance->writeRow(
                i,
                instance->displayRows[readIdx][i]
            );
        }

        // Yield and feed watchdog
        vTaskDelay(1);
    }
}


void P10Driver::init() {

    if (isInitialized)
        return;

    pinMode(PIN_OE, OUTPUT);
    pinMode(PIN_A, OUTPUT);
    pinMode(PIN_B, OUTPUT);
    pinMode(PIN_CLK, OUTPUT);
    pinMode(PIN_LAT, OUTPUT);
    pinMode(PIN_DR, OUTPUT);

    // Create asynchronous LED refresh task on Core 0
    xTaskCreatePinnedToCore(
        refreshTask,
        "P10RefreshTask",
        2048,
        this,
        1,
        &refreshTaskHandle,
        0
    );

    isInitialized = true;
}


void P10Driver::set(uint x, uint y, bool on) {

    if (x >= 64 || y >= 16)
        return;

    buffer[x][y] = on;
}


void P10Driver::setBrightness(uint value) {

    brightness = value;
}


void P10Driver::flush() {

    uint8_t backBufferIdx =
        1 - activeBufferIdx;

    // Convert logical framebuffer
    // into the physical P10 layout
    for (uint x = 0; x < 64; x++) {

        for (uint y = 0; y < 16; y++) {

            MatrixPosition pos =
                BufferPosToDisplayPos(x, y);

            displayRows[backBufferIdx]
                       [pos.row]
                       [pos.pos]
                = buffer[x][y];
        }
    }

    // Swap active display buffer
    activeBufferIdx = backBufferIdx;
}