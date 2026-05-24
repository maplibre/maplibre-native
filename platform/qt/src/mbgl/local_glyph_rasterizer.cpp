#include <mbgl/text/local_glyph_rasterizer.hpp>
#include <mbgl/util/i18n.hpp>
#include <mbgl/util/constants.hpp>

#include <QtCore/QFile>
#include <QtGui/QFont>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <qglobal.h>
namespace mbgl {

// 2x bitmap with 1x logical metrics.
constexpr int kLocalGlyphTextureScale = 2;
constexpr uint32_t kLocalGlyphLogicalSize = 30;
constexpr uint32_t kLocalGlyphBitmapSize = kLocalGlyphLogicalSize * kLocalGlyphTextureScale;
constexpr int kLocalGlyphRasterBuffer = Glyph::borderSize * kLocalGlyphTextureScale;
constexpr double kLocalGlyphTopAdjustment = 26.0;

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
        font.setPixelSize(util::ONE_EM * kLocalGlyphTextureScale);
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
    return impl->isConfigured() && impl->metrics->inFont(glyphID.complex.code) &&
           util::i18n::allowsFixedWidthGlyphGeneration(glyphID);
}

Glyph LocalGlyphRasterizer::rasterizeGlyph(const FontStack&, GlyphID glyphID) {
    Glyph glyph;
    glyph.id = glyphID;

    if (!impl->isConfigured()) {
        assert(false);
        return glyph;
    }

    // QFontMetrics returns measurements at the 2x rasterization pixel size; the
    // glyph's metrics are reported in 1x logical units.
    const QString text(QChar(glyphID.complex.code));
    const int advance2x = impl->metrics->horizontalAdvance(glyphID.complex.code);
    const int height2x = impl->metrics->height();
    const QRect glyphBounds = impl->metrics->tightBoundingRect(text);
    const int glyphTop2x = std::max(0, -glyphBounds.top());
    glyph.metrics.width = advance2x / kLocalGlyphTextureScale;
    glyph.metrics.height = height2x / kLocalGlyphTextureScale;
    glyph.metrics.left = 3.5f;
    glyph.metrics.top = glyphTop2x / static_cast<double>(kLocalGlyphTextureScale) - kLocalGlyphTopAdjustment;
    glyph.metrics.advance = glyph.metrics.width;
    glyph.metrics.isDoubleResolution = true;

    // The backing image is drawn at 2x texture resolution.
    Size size(kLocalGlyphBitmapSize, kLocalGlyphBitmapSize);
    QImage image(QSize(size.width, size.height), QImage::Format_Alpha8);
    image.fill(qRgba(0, 0, 0, 0));
    QPainter painter(&image);
    painter.setFont(impl->font);
    painter.setRenderHints(QPainter::TextAntialiasing);

    // Align the glyph using its measured top, matching the local TinySDF path
    // in GL JS rather than relying on a platform-wide fixed top value.
    const double baselineY = kLocalGlyphRasterBuffer + glyphTop2x;
    painter.drawText(QPointF(0, baselineY), text);

    // QImage::Format_Alpha8 may pad each row for alignment. Copy row-by-row
    // using the unpadded width so AlphaImage's tight layout matches.
    auto img = std::make_unique<uint8_t[]>(static_cast<size_t>(size.width) * size.height);
    for (uint32_t y = 0; y < size.height; ++y) {
        std::memcpy(img.get() + static_cast<size_t>(y) * size.width,
                    image.constScanLine(y),
                    size.width);
    }

    glyph.bitmap = AlphaImage{size, std::move(img)};

    return glyph;
}

} // namespace mbgl
