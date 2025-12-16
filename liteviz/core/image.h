#ifndef __LITEVIZ_IMAGE_H__
#define __LITEVIZ_IMAGE_H__

#include <liteviz/core/common.h>

// Forward declare STB functions - implementation in stb_impl.cpp
#include <liteviz/utils/stb_image.h>
#include <liteviz/utils/stb_image_write.h>

namespace liteviz {

struct Image {
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<uint8_t> data;

    Image() = default;
    Image(int w, int h, int c) : width(w), height(h), channels(c), data(w * h * c) {}

    bool empty() const { return data.empty() || width == 0 || height == 0; }
    uint8_t* ptr() { return data.empty() ? nullptr : data.data(); }
    const uint8_t* ptr() const { return data.empty() ? nullptr : data.data(); }
};

struct ImageIO {

static inline bool load(const std::string& path, Image& out, int desired_channels = 0) {
    int w = 0, h = 0, c = 0;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &c, desired_channels);
    if (!data) return false;
    int channels = desired_channels > 0 ? desired_channels : c;
    out.width = w;
    out.height = h;
    out.channels = channels;
    out.data.assign(data, data + (w * h * channels));
    stbi_image_free(data);
    return true;
}

static inline bool save_png(const std::string& path, const Image& img) {
    if (img.empty()) return false;
    int stride_in_bytes = img.width * img.channels;
    return stbi_write_png(path.c_str(), img.width, img.height, img.channels, img.ptr(), stride_in_bytes) != 0;
}

static inline void flip_vertical(Image& img) {
    if (img.empty()) return;
    int row_bytes = img.width * img.channels;
    std::vector<uint8_t> tmp(row_bytes);
    for (int y = 0; y < img.height / 2; ++y) {
        uint8_t* rowA = img.ptr() + y * row_bytes;
        uint8_t* rowB = img.ptr() + (img.height - 1 - y) * row_bytes;
        std::memcpy(tmp.data(), rowA, row_bytes);
        std::memcpy(rowA, rowB, row_bytes);
        std::memcpy(rowB, tmp.data(), row_bytes);
    }
}

static inline bool load_from_memory(const std::vector<uint8_t>& mem, Image& out, int desired_channels = 0) {
    int w = 0, h = 0, c = 0;
    unsigned char* data = stbi_load_from_memory(mem.data(), mem.size(), &w, &h, &c, desired_channels);
    if (!data) return false;
    int channels = desired_channels > 0 ? desired_channels : c;
    out.width = w;
    out.height = h;
    out.channels = channels;
    out.data.assign(data, data + (w * h * channels));
    stbi_image_free(data);
    return true;
}

};

} // namespace liteviz

#endif // __LITEVIZ_IMAGE_H__