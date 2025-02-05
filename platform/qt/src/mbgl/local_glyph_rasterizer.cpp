#include <mbgl/text/local_glyph_rasterizer.hpp>
#include <mbgl/util/i18n.hpp>
#include <mbgl/util/constants.hpp>

#include <QtCore/QFile>
#include <QtGui/QFont>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <qglobal.h>

namespace mbgl {

class LocalGlyphRasterizer::Impl {
public:
    Impl(const std::optional<std::string>& fontFamily_);

    bool isConfigured() const;

    std::optional<std::string> fontFamily;
    QFont font;
    std::optional<QFontMetrics> metrics;
};

LocalGlyphRasterizer::Impl::Impl(const std::optional<std::string>& fontFamily_)
    : fontFamily(fontFamily_) {
    if (isConfigured()) {
        font.setFamily(QString::fromStdString(*fontFamily));
        font.setPixelSize(util::ONE_EM);
        metrics = QFontMetrics(font);
    }
}

bool LocalGlyphRasterizer::Impl::isConfigured() const {
    return fontFamily.operator bool();
}

LocalGlyphRasterizer::LocalGlyphRasterizer(const std::optional<std::string>& fontFamily)
    : impl(std::make_unique<Impl>(fontFamily)) {}

LocalGlyphRasterizer::~LocalGlyphRasterizer() {}

bool LocalGlyphRasterizer::canRasterizeGlyph(const FontStack&, GlyphID glyphID) {
#ifdef MLN_TEXT_SHAPING_HARFBUZZ
    return impl->isConfigured() && impl->metrics->inFont(glyphID.complex.code) &&
           util::i18n::allowsFixedWidthGlyphGeneration(glyphID);
#else
    return impl->isConfigured() && impl->metrics->inFont(glyphID) &&
           util::i18n::allowsFixedWidthGlyphGeneration(glyphID);
#endif
}

Glyph LocalGlyphRasterizer::rasterizeGlyph(const FontStack&, GlyphID glyphID) {
    Glyph glyph;
    glyph.id = glyphID;

    if (!impl->isConfigured()) {
        assert(false);
        return glyph;
    }

#ifdef MLN_TEXT_SHAPING_HARFBUZZ
    glyph.metrics.width = impl->metrics->horizontalAdvance(glyphID.complex.code);
#else
    glyph.metrics.width = impl->metrics->horizontalAdvance(glyphID);
#endif
    glyph.metrics.height = impl->metrics->height();
    glyph.metrics.left = 3;
    glyph.metrics.top = -8;
    glyph.metrics.advance = glyph.metrics.width;

    // Set width of a glyph's backing image to be util::ONE_EM.
    Size size(util::ONE_EM, glyph.metrics.height);
    QImage image(QSize(size.width, size.height), QImage::Format_Alpha8);
    image.fill(qRgba(0, 0, 0, 0));
    QPainter painter(&image);
    painter.setFont(impl->font);
    painter.setRenderHints(QPainter::TextAntialiasing);

    // Render at constant baseline, to align with glyphs that are rendered by node-fontnik.
#ifdef MLN_TEXT_SHAPING_HARFBUZZ
    painter.drawText(QPointF(0, 20), QString(QChar(glyphID.complex.code)));
#else
    painter.drawText(QPointF(0, 20), QString(QChar(glyphID)));
#endif

    auto img = std::make_unique<uint8_t[]>(image.sizeInBytes());
    memcpy(img.get(), image.constBits(), image.sizeInBytes());

    glyph.bitmap = AlphaImage{size, std::move(img)};

    return glyph;
}

} // namespace mbgl
