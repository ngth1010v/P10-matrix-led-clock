#define PIN_OE   22
#define PIN_A    19
#define PIN_B    21
#define PIN_CLK  18
#define PIN_LAT  5
#define PIN_DR   23

#define BRIGHTNESS 1000

#include <array>

// 2 tấm P10 ghép ngang = 64 cột (x) x 16 hàng (y)
struct MatrixPosition {
  uint8_t row;
  uint pos;
};

class MatrixLed {

  private:
  bool buffer[64][16] = {};

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
  void writeRow(uint8_t row, bool* data) {

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
      delayMicroseconds(BRIGHTNESS);

      // Disable output before next row
      digitalWrite(PIN_OE, HIGH);
  }

  // Mapper
  // (x,y)   = (row,pos)
  // (2,2)   = (2,24 + 2)
  // (5,5)   = (1,16 + 5)
  // (20,15) = (3,64 + 4)
  // about:
  // - buffer: có góc tòa độ 0,0 ở gốc trái trên
  // - matrix: tính từ gốc trái, dưới. được group theo dạng zitzac:
  // [1]: group 1
  // 3 7 11 ...
  // 2 6 10 ...
  // 1 5 9 ...
  // 0 4 8 ...
MatrixPosition BufferPosToDisplayPos(uint x, uint y) {
    MatrixPosition res;

    res.row = y % 4;

    uint xOffset = (((uint)(x / 8)) * 8 * 4) + (x % 8);
    uint yOffset = (3 - ((uint)(y / 4))) * 8;
    res.pos = xOffset + yOffset;

    return res;
}

  public:
  void set(uint x, uint y, bool on){
    if ((x >= 64 || y >= 16)) return;
    buffer[x][y] = on;
  }
  void flush(){

    bool rows[4][64*16/4] = {};
    for (uint x = 0; x < 64; x += 1){
      for (uint y = 0; y < 16; y += 1){
        MatrixPosition pos = BufferPosToDisplayPos(x,y);
        rows[pos.row][pos.pos] = buffer[x][y];
      }
    }

    for (uint8_t i = 0; i < 4; i += 1){
      writeRow(i, rows[i]);
    }
  }
  void debugBuffer() {

    for (uint x = 0; x < 64; x++) {
        for (uint y = 0; y < 16; y++) {

            if (buffer[x][y]) {
                Serial.printf(
                    "BUFFER: [%u][%u] = 1\n",
                    x, y
                );
            }
        }
    }
}
};

MatrixLed matrixLed; 

void setup() {
  pinMode(PIN_OE, OUTPUT);
  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_CLK, OUTPUT);
  pinMode(PIN_LAT, OUTPUT);
  pinMode(PIN_DR, OUTPUT);

  Serial.begin(115200);
  delay(500);

  matrixLed.set(5, 5, true);
  matrixLed.set(2, 2, true);
  matrixLed.set(20, 15, true);

  matrixLed.debugBuffer();
}

void loop() {
  matrixLed.flush();
  // sleep(5);
}