#pragma once

#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cuchar>

class FontRenderer {
public:
    struct Bitmap {
        std::vector<std::vector<bool>> data;
        unsigned int w = 0;
        unsigned int h = 0;
    };

    FontRenderer() = default;
    ~FontRenderer() = default;

    /**
     * @brief Initializes the renderer with default 8x16 characters 
     *        (A-Z, a-z, 0-9) using set().
     */
    void init();

    /**
     * @brief Gets the bitmap for a given character, scaled/converted to target resolution.
     * @param c UTF-32 character code point.
     * @param w Target width.
     * @param h Target height.
     * @return Bitmap structure (w=0, h=0, data empty if unset).
     */
    Bitmap get(char32_t c, unsigned int w, unsigned int h) const;

    /**
     * @brief Stores a char bitmap, compressing the 2D bool vector into bit-packed uint8_t.
     * @param c UTF-32 character code point.
     * @param bmp The input 2D bool bitmap.
     */
    void set(char32_t c, const Bitmap& bmp);

private:
    struct InternalBitmap {
        std::vector<uint8_t> compressed_data; // Bit-packed pixel buffer
        unsigned int w = 0;
        unsigned int h = 0;
    };

    std::unordered_map<char32_t, InternalBitmap> font_cache_;

    // Resolution converter placeholder logic (currently returns original input)
    Bitmap convertResolution(const Bitmap& origin, unsigned int target_w, unsigned int target_h) const;

    // Helper functions for packing/unpacking pixels into bytes
    static std::vector<uint8_t> compressBitmap(const std::vector<std::vector<bool>>& data, unsigned int w, unsigned int h);
    static std::vector<std::vector<bool>> decompressBitmap(const std::vector<uint8_t>& compressed, unsigned int w, unsigned int h);
};
