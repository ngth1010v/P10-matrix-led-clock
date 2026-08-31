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

void P10Driver::writeRow(uint8_t row, const uint8_t* data) {
    // Disable output while shifting data
    digitalWrite(PIN_OE, HIGH);

    // 256 bits total per row = 32 bytes (MSB first)
    for (uint16_t byteIdx = 0; byteIdx < 32; byteIdx++) {
        uint8_t b = data[byteIdx];
        for (int8_t bitIdx = 7; bitIdx >= 0; bitIdx--) {
            bool bitVal = (b >> bitIdx) & 0x01;
            writeLed(bitVal);
        }
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

P10Driver::MatrixPosition P10Driver::BufferPosToDisplayPos(uint x, uint y) {
    MatrixPosition res;

    res.row = y % 4;

    uint xOffset = (((uint)(x / 8)) * 8 * 4) + (x % 8);
    uint yOffset = (3 - ((uint)(y / 4))) * 8;

    res.pos = xOffset + yOffset;

    return res;
}

void P10Driver::refreshTask(void* pvParameters) {
    P10Driver* instance = static_cast<P10Driver*>(pvParameters);

    while (true) {
        // Read current active buffer index atomically
        uint8_t readIdx = instance->activeBufferIdx.load(std::memory_order_relaxed);

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

    // Row-major byte layout: 8 pixels per byte horizontally
    uint8_t byteIdx = x / 8;
    uint8_t bitMask = 1 << (7 - (x % 8)); // MSB-first bit order

    if (on) {
        buffer[y][byteIdx].fetch_or(bitMask, std::memory_order_relaxed);
    } else {
        buffer[y][byteIdx].fetch_and(~bitMask, std::memory_order_relaxed);
    }
}

void P10Driver::setBrightness(uint value) {
    brightness = value;
}

void P10Driver::flush() {
    uint8_t backBufferIdx = 1 - activeBufferIdx.load(std::memory_order_relaxed);

    // Clear back buffer row storage before mapping
    memset(displayRows[backBufferIdx], 0, sizeof(displayRows[backBufferIdx]));

    // Convert logical framebuffer into the physical P10 packed layout
    for (uint x = 0; x < 64; x++) {
        for (uint y = 0; y < 16; y++) {
            uint8_t byteIdx = x / 8;
            uint8_t bitMask = 1 << (7 - (x % 8));

            // Extract pixel bit atomically from logical buffer
            bool pixelOn = (buffer[y][byteIdx].load(std::memory_order_relaxed) & bitMask) != 0;

            if (pixelOn) {
                MatrixPosition pos = BufferPosToDisplayPos(x, y);

                uint16_t targetByte = pos.pos / 8;
                uint8_t targetBitMask = 1 << (7 - (pos.pos % 8));

                displayRows[backBufferIdx][pos.row][targetByte] |= targetBitMask;
            }
        }
    }

    // Swap active display buffer atomically
    activeBufferIdx.store(backBufferIdx, std::memory_order_release);
}