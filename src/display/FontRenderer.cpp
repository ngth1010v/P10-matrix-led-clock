#include "display/FontRenderer.h"

void FontRenderer::init() {
    auto addChar = [this](char32_t c, const std::vector<std::vector<bool>>& grid) {
        Bitmap temp;
        temp.w = 8;
        temp.h = 16;
        temp.data = grid;
        this->set(c, temp);
    };


    // ==========================================
    // UPPERCASE LETTERS (A - Z)
    // ==========================================
    addChar('A', {
        {0,0,0,1,1,0,0,0},
        {0,0,1,0,0,1,0,0},
        {0,1,0,0,0,0,1,0},
        {1,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,1}
    });

    addChar('B', {
        {1,1,1,1,1,1,0,0},
        {1,0,0,0,0,0,1,0},
        {1,0,0,0,0,0,1,0},
        {1,1,1,1,1,1,0,0},
        {1,0,0,0,0,0,1,0},
        {1,0,0,0,0,0,1,0},
        {1,0,0,0,0,0,1,0},
        {1,1,1,1,1,1,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0}
    });

}

void FontRenderer::set(char32_t c, const Bitmap& bmp) {
    InternalBitmap internal_bmp;
    internal_bmp.w = bmp.w;
    internal_bmp.h = bmp.h;
    internal_bmp.compressed_data = compressBitmap(bmp.data, bmp.w, bmp.h);

    font_cache_[c] = std::move(internal_bmp);
}

FontRenderer::Bitmap FontRenderer::get(char32_t c, unsigned int target_w, unsigned int target_h) const {
    auto it = font_cache_.find(c);
    if (it == font_cache_.end()) {
        // Return empty bitmap (w = 0, h = 0) for unset character
        return Bitmap{};
    }

    Bitmap original;
    original.w = it->second.w;
    original.h = it->second.h;
    original.data = decompressBitmap(it->second.compressed_data, original.w, original.h);

    return convertResolution(original, target_w, target_h);
}

FontRenderer::Bitmap FontRenderer::convertResolution(const Bitmap& origin, unsigned int target_w, unsigned int target_h) const {
    // Placeholder resolution converter (returns original)
    (void)target_w;
    (void)target_h;
    return origin;
}

std::vector<uint8_t> FontRenderer::compressBitmap(const std::vector<std::vector<bool>>& data, unsigned int w, unsigned int h) {
    size_t total_bits = static_cast<size_t>(w) * h;
    size_t total_bytes = (total_bits + 7) / 8;

    std::vector<uint8_t> compressed(total_bytes, 0);

    for (unsigned int y = 0; y < h; ++y) {
        for (unsigned int x = 0; x < w; ++x) {
            if (y < data.size() && x < data[y].size() && data[y][x]) {
                size_t bit_index = static_cast<size_t>(y) * w + x;
                compressed[bit_index / 8] |= (1 << (7 - (bit_index % 8)));
            }
        }
    }

    return compressed;
}

std::vector<std::vector<bool>> FontRenderer::decompressBitmap(const std::vector<uint8_t>& compressed, unsigned int w, unsigned int h) {
    std::vector<std::vector<bool>> data(h, std::vector<bool>(w, false));

    for (unsigned int y = 0; y < h; ++y) {
        for (unsigned int x = 0; x < w; ++x) {
            size_t bit_index = static_cast<size_t>(y) * w + x;
            if ((compressed[bit_index / 8] & (1 << (7 - (bit_index % 8)))) != 0) {
                data[y][x] = true;
            }
        }
    }

    return data;
}