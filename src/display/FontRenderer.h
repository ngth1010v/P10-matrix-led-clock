#pragma once

#include <cstdint>
#include <map>
#include <utility>
#include <vector>

class FontRenderer {
public:
    // User-facing Bitmap representation
    struct Bitmap {
        uint8_t w{0};
        uint8_t h{0};
        std::vector<std::vector<bool>> pixels;
    };

private:
    // Internal compact bit-packed representation (Row-major, 1 byte per row)
    struct InternalBitmap {
        uint8_t w{0};
        uint8_t h{0};
        std::vector<uint8_t> packed_data; // Size = h bytes, lower 'w' bits represent pixel columns
    };

    std::map<std::pair<char, bool>, InternalBitmap> bitmapMap;
    bool is_initialized{false};

    // Private helper to store character bit data
    void set(char c, bool mini, const Bitmap& data);

public:
    FontRenderer() = default;
    ~FontRenderer() = default;

    // Initializes the renderer state
    void init();

    // Returns character bitmap; returns w=0, h=0 if not found or uninitialized
    Bitmap get(char c, bool mini);
};
