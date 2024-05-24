#include <httplib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

std::vector<uint8_t> convertToRGB565(const uint8_t* image_data, int width, int height, int channels) {
    std::vector<uint8_t> rgb565_data;
    rgb565_data.reserve(width * height * 2); // Each pixel will be split into two bytes

    for (int i = 0; i < width * height; ++i) {
        uint8_t r = image_data[i * channels];
        uint8_t g = image_data[i * channels + 1];
        uint8_t b = image_data[i * channels + 2];

        uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

        uint8_t byte1 = rgb565 & 0xFF;         // Lower byte
        uint8_t byte2 = (rgb565 >> 8) & 0xFF;  // Upper byte

        rgb565_data.push_back(byte2);
        rgb565_data.push_back(byte1);
    }

    return rgb565_data;
}

std::vector<uint8_t> downloadAndConvertToRGB565(const std::string& url) {
    auto response = cpr::Get(cpr::Url{url});

    if (response.status_code != 200) {
        std::cerr << "Failed to download image: " << response.status_code << std::endl;
        return {};
    }

    int width, height, channels;
    unsigned char* image_data = stbi_load_from_memory(
        reinterpret_cast<const unsigned char*>(response.text.c_str()), response.text.size(), &width, &height, &channels, 3);

    if (!image_data) {
        std::cerr << "Failed to load image." << std::endl;
        return {};
    }

    int new_width = 64;
    int new_height = 64;
    std::vector<uint8_t> resized_image_data(new_width * new_height * 3);

    if (!stbir_resize_uint8(image_data, width, height, 0, resized_image_data.data(), new_width, new_height, 0, 3)) {
        std::cerr << "Failed to resize image." << std::endl;
        stbi_image_free(image_data);
        return {};
    }

    // Create a new buffer for the mirrored image
    std::vector<uint8_t> mirrored_image_data(new_width * new_height * 3);

    // Mirror the image along the line y=-x
    for (int y = 0; y < new_height; ++y) {
        for (int x = 0; x < new_width; ++x) {
            int src_index = (y * new_width + x) * 3;
            int dst_index = (x * new_height + y) * 3;

            mirrored_image_data[dst_index] = resized_image_data[src_index];
            mirrored_image_data[dst_index + 1] = resized_image_data[src_index + 1];
            mirrored_image_data[dst_index + 2] = resized_image_data[src_index + 2];
        }
    }

    std::vector<uint8_t> rgb565_data = convertToRGB565(mirrored_image_data.data(), new_width, new_height, 3);

    // Free the loaded image data
    stbi_image_free(image_data);

    return rgb565_data;
}

void writeVectorToFile(const std::vector<uint8_t>& data, const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    for (size_t i = 0; i < data.size(); ++i) {
        uint8_t value = data[i];

        file << static_cast<int>(value);
        if (i != data.size() - 1) {
            file << ",";
        }
    }

    file.close();
    if (!file.good()) {
        std::cerr << "Error occurred while writing to file: " << filename << std::endl;
    }
}