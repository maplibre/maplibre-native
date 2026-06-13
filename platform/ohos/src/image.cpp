#include <mbgl/util/image.hpp>
#include <mbgl/util/premultiply.hpp>

#include <multimedia/image_framework/image/image_source_native.h>
#include <multimedia/image_framework/image/pixelmap_native.h>

#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace mbgl {
namespace {

const char* imageErrorCodeName(Image_ErrorCode code) {
    switch (code) {
        case IMAGE_SUCCESS:
            return "IMAGE_SUCCESS";
        case IMAGE_BAD_PARAMETER:
            return "IMAGE_BAD_PARAMETER";
        case IMAGE_UNSUPPORTED_MIME_TYPE:
            return "IMAGE_UNSUPPORTED_MIME_TYPE";
        case IMAGE_UNKNOWN_MIME_TYPE:
            return "IMAGE_UNKNOWN_MIME_TYPE";
        case IMAGE_TOO_LARGE:
            return "IMAGE_TOO_LARGE";
        case IMAGE_DMA_NOT_EXIST:
            return "IMAGE_DMA_NOT_EXIST";
        case IMAGE_DMA_OPERATION_FAILED:
            return "IMAGE_DMA_OPERATION_FAILED";
        case IMAGE_UNSUPPORTED_OPERATION:
            return "IMAGE_UNSUPPORTED_OPERATION";
        case IMAGE_UNSUPPORTED_METADATA:
            return "IMAGE_UNSUPPORTED_METADATA";
        case IMAGE_UNSUPPORTED_CONVERSION:
            return "IMAGE_UNSUPPORTED_CONVERSION";
        case IMAGE_INVALID_REGION:
            return "IMAGE_INVALID_REGION";
        case IMAGE_UNSUPPORTED_MEMORY_FORMAT:
            return "IMAGE_UNSUPPORTED_MEMORY_FORMAT";
        case IMAGE_INVALID_PARAMETER:
            return "IMAGE_INVALID_PARAMETER";
        case IMAGE_ALLOC_FAILED:
            return "IMAGE_ALLOC_FAILED";
        case IMAGE_COPY_FAILED:
            return "IMAGE_COPY_FAILED";
        case IMAGE_LOCK_UNLOCK_FAILED:
            return "IMAGE_LOCK_UNLOCK_FAILED";
        case IMAGE_ALLOCATOR_MODE_UNSUPPORTED:
            return "IMAGE_ALLOCATOR_MODE_UNSUPPORTED";
        case IMAGE_UNKNOWN_ERROR:
            return "IMAGE_UNKNOWN_ERROR";
        case IMAGE_BAD_SOURCE:
            return "IMAGE_BAD_SOURCE";
        case IMAGE_SOURCE_UNSUPPORTED_MIME_TYPE:
            return "IMAGE_SOURCE_UNSUPPORTED_MIME_TYPE";
        case IMAGE_SOURCE_TOO_LARGE:
            return "IMAGE_SOURCE_TOO_LARGE";
        case IMAGE_SOURCE_UNSUPPORTED_ALLOCATOR_TYPE:
            return "IMAGE_SOURCE_UNSUPPORTED_ALLOCATOR_TYPE";
        case IMAGE_SOURCE_UNSUPPORTED_OPTIONS:
            return "IMAGE_SOURCE_UNSUPPORTED_OPTIONS";
        case IMAGE_SOURCE_INVALID_PARAMETER:
            return "IMAGE_SOURCE_INVALID_PARAMETER";
        case IMAGE_DECODE_FAILED:
            return "IMAGE_DECODE_FAILED";
        case IMAGE_SOURCE_ALLOC_FAILED:
            return "IMAGE_SOURCE_ALLOC_FAILED";
        case IMAGE_PACKER_INVALID_PARAMETER:
            return "IMAGE_PACKER_INVALID_PARAMETER";
        case IMAGE_ENCODE_FAILED:
            return "IMAGE_ENCODE_FAILED";
        case IMAGE_RECEIVER_INVALID_PARAMETER:
            return "IMAGE_RECEIVER_INVALID_PARAMETER";
        default:
            return nullptr;
    }
}

std::string imageErrorCodeMessage(Image_ErrorCode code) {
    if (const char* name = imageErrorCodeName(code)) {
        return std::string{name} + " (" + std::to_string(code) + ")";
    }
    return "Image_ErrorCode " + std::to_string(code);
}

void checkImageResult(Image_ErrorCode code, const char* operation) {
    if (code != IMAGE_SUCCESS) {
        throw std::runtime_error(std::string(operation) + " failed with " + imageErrorCodeMessage(code));
    }
}

const char* pixelFormatName(int32_t format) {
    switch (format) {
        case PIXEL_FORMAT_UNKNOWN:
            return "PIXEL_FORMAT_UNKNOWN";
        case PIXEL_FORMAT_RGB_565:
            return "PIXEL_FORMAT_RGB_565";
        case PIXEL_FORMAT_RGBA_8888:
            return "PIXEL_FORMAT_RGBA_8888";
        case PIXEL_FORMAT_BGRA_8888:
            return "PIXEL_FORMAT_BGRA_8888";
        case PIXEL_FORMAT_RGB_888:
            return "PIXEL_FORMAT_RGB_888";
        case PIXEL_FORMAT_ALPHA_8:
            return "PIXEL_FORMAT_ALPHA_8";
        case PIXEL_FORMAT_RGBA_F16:
            return "PIXEL_FORMAT_RGBA_F16";
        case PIXEL_FORMAT_NV21:
            return "PIXEL_FORMAT_NV21";
        case PIXEL_FORMAT_NV12:
            return "PIXEL_FORMAT_NV12";
        case PIXEL_FORMAT_RGBA_1010102:
            return "PIXEL_FORMAT_RGBA_1010102";
        case PIXEL_FORMAT_YCBCR_P010:
            return "PIXEL_FORMAT_YCBCR_P010";
        case PIXEL_FORMAT_YCRCB_P010:
            return "PIXEL_FORMAT_YCRCB_P010";
        default:
            return nullptr;
    }
}

const char* alphaTypeName(int32_t alphaType) {
    switch (alphaType) {
        case PIXELMAP_ALPHA_TYPE_UNKNOWN:
            return "PIXELMAP_ALPHA_TYPE_UNKNOWN";
        case PIXELMAP_ALPHA_TYPE_OPAQUE:
            return "PIXELMAP_ALPHA_TYPE_OPAQUE";
        case PIXELMAP_ALPHA_TYPE_PREMULTIPLIED:
            return "PIXELMAP_ALPHA_TYPE_PREMULTIPLIED";
        case PIXELMAP_ALPHA_TYPE_UNPREMULTIPLIED:
            return "PIXELMAP_ALPHA_TYPE_UNPREMULTIPLIED";
        default:
            return nullptr;
    }
}

std::string namedValue(const char* name, int32_t value) {
    if (name) {
        return std::string{name} + " (" + std::to_string(value) + ")";
    }
    return std::to_string(value);
}

std::string pixelmapDescription(
    uint32_t width, uint32_t height, uint32_t rowStride, int32_t format, int32_t alphaType) {
    return "width=" + std::to_string(width) + ", height=" + std::to_string(height) +
           ", rowStride=" + std::to_string(rowStride) + ", pixelFormat=" + namedValue(pixelFormatName(format), format) +
           ", alphaType=" + namedValue(alphaTypeName(alphaType), alphaType);
}

struct ImageSourceDeleter {
    void operator()(OH_ImageSourceNative* source) const {
        if (source) {
            OH_ImageSourceNative_Release(source);
        }
    }
};

struct DecodingOptionsDeleter {
    void operator()(OH_DecodingOptions* options) const {
        if (options) {
            OH_DecodingOptions_Release(options);
        }
    }
};

struct PixelmapDeleter {
    void operator()(OH_PixelmapNative* pixelmap) const {
        if (pixelmap) {
            OH_PixelmapNative_Release(pixelmap);
        }
    }
};

struct PixelmapInfoDeleter {
    void operator()(OH_Pixelmap_ImageInfo* info) const {
        if (info) {
            OH_PixelmapImageInfo_Release(info);
        }
    }
};

using ImageSourcePtr = std::unique_ptr<OH_ImageSourceNative, ImageSourceDeleter>;
using DecodingOptionsPtr = std::unique_ptr<OH_DecodingOptions, DecodingOptionsDeleter>;
using PixelmapPtr = std::unique_ptr<OH_PixelmapNative, PixelmapDeleter>;
using PixelmapInfoPtr = std::unique_ptr<OH_Pixelmap_ImageInfo, PixelmapInfoDeleter>;

std::size_t checkedByteCount(std::size_t width, std::size_t height, const char* description) {
    if (height != 0 && width > std::numeric_limits<std::size_t>::max() / height) {
        throw std::runtime_error(std::string("OHOS image decoder returned overflowing ") + description);
    }
    return width * height;
}

void copyRows(
    uint8_t* dst, const uint8_t* src, uint32_t width, uint32_t height, std::size_t rowStride, int32_t format) {
    const auto tightStride = checkedByteCount(static_cast<std::size_t>(width), 4, "row stride");

    for (uint32_t y = 0; y < height; ++y) {
        const auto* srcRow = src + y * rowStride;
        auto* dstRow = dst + y * tightStride;

        if (format == PIXEL_FORMAT_BGRA_8888) {
            for (uint32_t x = 0; x < width; ++x) {
                dstRow[4 * x + 0] = srcRow[4 * x + 2];
                dstRow[4 * x + 1] = srcRow[4 * x + 1];
                dstRow[4 * x + 2] = srcRow[4 * x + 0];
                dstRow[4 * x + 3] = srcRow[4 * x + 3];
            }
        } else {
            std::memcpy(dstRow, srcRow, tightStride);
        }
    }
}

} // namespace

PremultipliedImage decodeImage(const std::string& string) {
    auto data = std::vector<uint8_t>(string.begin(), string.end());

    OH_ImageSourceNative* sourceRaw = nullptr;
    checkImageResult(OH_ImageSourceNative_CreateFromData(data.data(), data.size(), &sourceRaw),
                     "OH_ImageSourceNative_CreateFromData");
    ImageSourcePtr source(sourceRaw);

    OH_DecodingOptions* optionsRaw = nullptr;
    checkImageResult(OH_DecodingOptions_Create(&optionsRaw), "OH_DecodingOptions_Create");
    DecodingOptionsPtr options(optionsRaw);
    checkImageResult(OH_DecodingOptions_SetPixelFormat(options.get(), PIXEL_FORMAT_RGBA_8888),
                     "OH_DecodingOptions_SetPixelFormat");

    OH_PixelmapNative* pixelmapRaw = nullptr;
    checkImageResult(OH_ImageSourceNative_CreatePixelmap(source.get(), options.get(), &pixelmapRaw),
                     "OH_ImageSourceNative_CreatePixelmap");
    PixelmapPtr pixelmap(pixelmapRaw);

    OH_Pixelmap_ImageInfo* infoRaw = nullptr;
    checkImageResult(OH_PixelmapImageInfo_Create(&infoRaw), "OH_PixelmapImageInfo_Create");
    PixelmapInfoPtr info(infoRaw);
    checkImageResult(OH_PixelmapNative_GetImageInfo(pixelmap.get(), info.get()), "OH_PixelmapNative_GetImageInfo");

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t rowStride = 0;
    int32_t pixelFormat = PIXEL_FORMAT_UNKNOWN;
    int32_t alphaType = PIXELMAP_ALPHA_TYPE_UNKNOWN;
    checkImageResult(OH_PixelmapImageInfo_GetWidth(info.get(), &width), "OH_PixelmapImageInfo_GetWidth");
    checkImageResult(OH_PixelmapImageInfo_GetHeight(info.get(), &height), "OH_PixelmapImageInfo_GetHeight");
    checkImageResult(OH_PixelmapImageInfo_GetRowStride(info.get(), &rowStride), "OH_PixelmapImageInfo_GetRowStride");
    checkImageResult(OH_PixelmapImageInfo_GetPixelFormat(info.get(), &pixelFormat),
                     "OH_PixelmapImageInfo_GetPixelFormat");
    checkImageResult(OH_PixelmapImageInfo_GetAlphaType(info.get(), &alphaType), "OH_PixelmapImageInfo_GetAlphaType");

    const auto description = pixelmapDescription(width, height, rowStride, pixelFormat, alphaType);
    if (width == 0 || height == 0) {
        throw std::runtime_error("OHOS image decoder returned empty image dimensions: " + description);
    }
    if (pixelFormat != PIXEL_FORMAT_RGBA_8888 && pixelFormat != PIXEL_FORMAT_BGRA_8888) {
        throw std::runtime_error("OHOS image decoder returned unsupported pixel format: " + description);
    }

    const auto sourceRowStride = checkedByteCount(static_cast<std::size_t>(width), 4, "row stride");
    const auto bytesRequired = checkedByteCount(sourceRowStride, height, "pixel buffer size");
    std::vector<uint8_t> pixels(bytesRequired);
    size_t bytesRead = pixels.size();
    checkImageResult(OH_PixelmapNative_ReadPixels(pixelmap.get(), pixels.data(), &bytesRead),
                     "OH_PixelmapNative_ReadPixels");
    if (bytesRead < bytesRequired) {
        throw std::runtime_error("OHOS image decoder returned fewer pixel bytes than expected: read " +
                                 std::to_string(bytesRead) + " of " + std::to_string(bytesRequired) + " bytes, " +
                                 description);
    }

    const Size imageSize{width, height};
    if (alphaType == PIXELMAP_ALPHA_TYPE_UNPREMULTIPLIED) {
        UnassociatedImage image(imageSize);
        copyRows(image.data.get(), pixels.data(), width, height, sourceRowStride, pixelFormat);
        return util::premultiply(std::move(image));
    }

    PremultipliedImage image(imageSize);
    copyRows(image.data.get(), pixels.data(), width, height, sourceRowStride, pixelFormat);
    return image;
}

} // namespace mbgl
