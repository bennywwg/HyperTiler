#include "ImageUtils.hpp"

#include <png.h>

namespace HyperTiler {
    struct pngMemoryReader {
        uint8_t const* dataStart;
        uint64_t currentPos;
    };

    void pngReadFunc(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead) {
        png_voidp io_ptr = png_get_io_ptr(png_ptr);
        htAssert(io_ptr);

        pngMemoryReader& status = *reinterpret_cast<pngMemoryReader*>(io_ptr);

        memcpy(outBytes, status.dataStart + status.currentPos, byteCountToRead);

        status.currentPos += byteCountToRead;
    }

    ImageData ReadPng(vector<uint8_t> const& data, bool expand) {
        if (data.size() < 8) return ImageData();

        if (png_sig_cmp(data.data(), 0, 8)) {
            return ImageData();
        }

        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

        if (!png_ptr) {
            return ImageData();
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            png_destroy_read_struct(&png_ptr, NULL, NULL);
            return ImageData();
        }

        if (setjmp(png_jmpbuf(png_ptr))) {
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            return ImageData();
        }

        pngMemoryReader memoryReader{ data.data(), 8 };

        png_set_read_fn(png_ptr, &memoryReader, pngReadFunc);
        png_set_sig_bytes(png_ptr, 8);

        png_read_info(png_ptr, info_ptr);

        if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_PALETTE) {
            //png_set_palette_to_rgb(png_ptr);
        }

        ImageData res;

        png_set_interlace_handling(png_ptr);
        if (expand) png_set_expand(png_ptr);
        png_read_update_info(png_ptr, info_ptr);

        res.bitDepth = png_get_bit_depth(png_ptr, info_ptr);
        res.numChannels = png_get_channels(png_ptr, info_ptr);
        res.width = png_get_image_width(png_ptr, info_ptr);
        res.height = png_get_image_height(png_ptr, info_ptr);

        if (res.bitDepth == 0) {
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            return ImageData();
        }

        htAssert(res.bitDepth == 8 || res.bitDepth == 16);

        // set error handler with... jumps? yikes.
        if (setjmp(png_jmpbuf(png_ptr))) {
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            return ImageData();
        }

        res.data.resize(res.width * res.height * res.numChannels * res.bitDepth / 8);

        std::vector<png_bytep> rowPointers;
        for (int row = 0; row < res.height; ++row) {
            rowPointers.push_back(res.data.data() + row * res.numChannels * (res.bitDepth / 8) * res.width);
        }

        png_read_image(png_ptr, rowPointers.data());

        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

        return res;
    }

    bool WritePng(vector<uint8_t>& outputData, uint8_t* inputData, int width, int height, bool swapEndian) {
        if (swapEndian) {
            for (int i = 0; i < width * height; ++i) {
                uint8_t tmp = inputData[i * 2];
                inputData[i * 2] = inputData[i * 2 + 1];
                inputData[i * 2 + 1] = tmp;
            }
        }

        png_image img;
        img.version = PNG_IMAGE_VERSION;
        img.format = PNG_FORMAT_LINEAR_Y;
        img.width = width;
        img.height = height;
        //img.flags = PNG_IMAGE_FLAG_16BIT_sRGB;

        png_alloc_size_t size = width * height * sizeof(uint16_t);

        if (!png_image_write_to_memory(&img, nullptr, &size, 0, inputData, 0, nullptr)) {
            return false;
        }

        outputData.resize(size);

        return png_image_write_to_memory(&img, outputData.data(), &size, 0, inputData, 0, nullptr);
    }

    bool WritePng(vector<uint8_t>& outputData, ImageData const& img) {
        png_image pimg;
        memset(&pimg, 0, sizeof(pimg));
        pimg.version = PNG_IMAGE_VERSION;
        pimg.format = PNG_FORMAT_RGB;
        pimg.width = img.width;
        pimg.height = img.height;

        png_alloc_size_t size = img.width * img.height * (img.bitDepth / 8) * img.numChannels;
        htAssert(size == img.data.size());

        if (!png_image_write_to_memory(&pimg, nullptr, &size, 0, img.data.data(), 0, nullptr)) {
            return false;
        }

        outputData.resize(size);

        return png_image_write_to_memory(&pimg, outputData.data(), &size, 0, img.data.data(), 0, nullptr);
    }
}