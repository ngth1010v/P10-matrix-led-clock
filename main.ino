#define PIN_OE   22
#define PIN_A    19
#define PIN_B    21
#define PIN_CLK  18
#define PIN_LAT  5
#define PIN_DR   23

#include <Arduino.h>

// 2 tấm P10 ghép ngang = 64 cột (x) x 16 hàng (y)
class MatrixLed {
  struct MatrixPosition {
    uint8_t row;
    uint pos;
  };

  //===================================================================
  // PRIVATE
  //===================================================================
  private:
    bool buffer[64][16] = {};
    uint brightness = 1000;

    // Double buffering for thread-safe display updates without locking
    bool displayRows[2][4][64 * 16 / 4] = {};
    volatile uint8_t activeBufferIdx = 0;

    bool isInitialized = false;
    TaskHandle_t refreshTaskHandle = NULL;

    // Helper
    void selectRow(uint8_t row) {
      digitalWrite(PIN_A, (row & 0x01) ? HIGH : LOW);
      digitalWrite(PIN_B, (row & 0x02) ? HIGH : LOW);
    }
    
    void writeLed(bool on) {
      // P10 Active LOW: true -> LOW (Sáng), false -> HIGH (Tắt)
      digitalWrite(PIN_DR, on ? LOW : HIGH);
      digitalWrite(PIN_CLK, HIGH);
      digitalWrite(PIN_CLK, LOW);
    }   
    
    void writeRow(uint8_t row, const bool* data) {
        digitalWrite(PIN_OE, HIGH);

        for (uint i = 0; i < 64 * 16 / 4; i++) {
          writeLed(data[i]);
        }

        // Latch
        digitalWrite(PIN_LAT, HIGH);
        digitalWrite(PIN_LAT, LOW);

        // Select row
        selectRow(row);

        // Dead-time
        delayMicroseconds(5);

        // Enable output
        digitalWrite(PIN_OE, LOW);

        // Display time
        delayMicroseconds(brightness);

        // Disable output before next row
        digitalWrite(PIN_OE, HIGH);
    }

    // Mapper
    MatrixPosition BufferPosToDisplayPos(uint x, uint y) {
      MatrixPosition res;

      res.row = y % 4;

      uint xOffset = (((uint)(x / 8)) * 8 * 4) + (x % 8);
      uint yOffset = (3 - ((uint)(y / 4))) * 8;
      res.pos = xOffset + yOffset;

      return res;
    }

    // FreeRTOS Task Static Runner
    static void refreshTask(void* pvParameters) {
      MatrixLed* instance = static_cast<MatrixLed*>(pvParameters);
      while (true) {
        // Read from current active buffer safely
        uint8_t readIdx = instance->activeBufferIdx;
        for (uint8_t i = 0; i < 4; i++) {
          instance->writeRow(i, instance->displayRows[readIdx][i]);
        }
        // Feeds the Watchdog Timer properly while yielding control
        vTaskDelay(1);
      }
    }

  //===================================================================
  // PUBLIC
  //===================================================================
  public:

    // Initialize pins and create background refresh task
    void init() {
      if (isInitialized) return;

      pinMode(PIN_OE, OUTPUT);
      pinMode(PIN_A, OUTPUT);
      pinMode(PIN_B, OUTPUT);
      pinMode(PIN_CLK, OUTPUT);
      pinMode(PIN_LAT, OUTPUT);
      pinMode(PIN_DR, OUTPUT);

      // Create asynchronous LED refresh task on Core 0
      xTaskCreatePinnedToCore(
        refreshTask,
        "MatrixRefreshTask",
        2048,
        this,
        1,
        &refreshTaskHandle,
        0
      );

      isInitialized = true;
    }

    // Editor
    void set(uint x, uint y, bool on){
      if ((x >= 64 || y >= 16)) return;
      buffer[x][y] = on;
    }
    
    void setBrightness(uint value){
      brightness = value;
    }

    // Prepares display buffer and updates display pointer atomically
    void flush(){
      uint8_t backBufferIdx = 1 - activeBufferIdx;

      for (uint x = 0; x < 64; x += 1){
        for (uint y = 0; y < 16; y += 1){
          MatrixPosition pos = BufferPosToDisplayPos(x, y);
          displayRows[backBufferIdx][pos.row][pos.pos] = buffer[x][y];
        }
      }

      // Atomic pointer swap to display the newly built frame
      activeBufferIdx = backBufferIdx;
    }
};

MatrixLed matrixLed; 

void setup() {
  Serial.begin(115200);
  delay(500);

  matrixLed.init(); // Auto setups GPIOs & starts async core 0 task

  matrixLed.set(5, 5, true);
  matrixLed.set(2, 2, true);
  matrixLed.set(50, 15, true);

  matrixLed.flush(); // Render initial buffer state once
}

void loop() {
  delay(1000);
}