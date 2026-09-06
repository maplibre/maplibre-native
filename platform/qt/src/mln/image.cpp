#include <mln/util/image.hpp>
#include <mln/util/constants.hpp>

#include <QBuffer>
#include <QByteArray>
#include <QImage>
#include <QImageReader>

namespace mln {

std::string encodePNG(const PremultipliedImage& pre) {
    QImage image(pre.data.get(), pre.size.width, pre.size.height, QImage::Format_ARGB32_Premultiplied);

    QByteArray array;
    QBuffer buffer(&array);

    buffer.open(QIODevice::WriteOnly);
    image.rgbSwapped().save(&buffer, "PNG");

    return std::string(array.constData(), array.size());
}

#if !defined(QT_IMAGE_DECODERS)
PremultipliedImage decodeJPEG(const uint8_t*, size_t);
#endif

PremultipliedImage decodeImage(const std::string& string) {
    const uint8_t* data = reinterpret_cast<const uint8_t*>(string.data());
    const size_t size = string.size();

#if !defined(QT_IMAGE_DECODERS)
    if (size >= 2) {
        uint16_t magic = ((data[0] << 8) | data[1]) & 0xffff;
        if (magic == 0xFFD8) {
            return decodeJPEG(data, size);
        }
    }
#endif

    QImage image = QImage::fromData(data, static_cast<int>(size))
                       .rgbSwapped()
                       .convertToFormat(QImage::Format_ARGB32_Premultiplied);

    if (image.isNull()) {
        throw std::runtime_error("Unsupported image type");
    }

    auto img = std::make_unique<uint8_t[]>(image.sizeInBytes());
    memcpy(img.get(), image.constBits(), image.sizeInBytes());

    return {{static_cast<uint32_t>(image.width()), static_cast<uint32_t>(image.height())}, std::move(img)};
}

std::string_view rasterAcceptHeader() {
    static const bool supportsWebP = QImageReader::supportedImageFormats().contains("webp");
    return supportsWebP ? util::MIME_TYPE_RASTER : util::MIME_TYPE_RASTER_NO_WEBP;
}

} // namespace mln
