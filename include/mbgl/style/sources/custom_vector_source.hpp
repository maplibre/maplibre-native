#pragma once

#include <mbgl/style/source.hpp>
#include <mbgl/util/range.hpp>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace mbgl {

class OverscaledTileID;
class CanonicalTileID;
template <class T>
class Actor;

namespace style {

using TileFunction = std::function<void(const CanonicalTileID&)>;

class CustomVectorTileLoader;

enum class TileDataFormat : uint8_t {
    MVT = 0,
};

class CustomVectorSource final : public Source {
public:
    struct Options {
        TileFunction fetchTileFunction;
        TileFunction cancelTileFunction;
        Range<uint8_t> zoomRange = {0, 18};
    };

    CustomVectorSource(std::string id, const Options& options);
    ~CustomVectorSource() final;

    void loadDescription(FileSource&) final;
    void setTileData(const CanonicalTileID&,
                     const std::shared_ptr<const std::string>& data,
                     TileDataFormat format = TileDataFormat::MVT);
    void setTileError(const CanonicalTileID&, std::exception_ptr error);
    void invalidateTile(const CanonicalTileID&);

    class Impl;
    const Impl& impl() const;
    bool supportsLayerType(const mbgl::style::LayerTypeInfo*) const override;
    mapbox::base::WeakPtr<Source> makeWeakPtr() override { return weakFactory.makeWeakPtr(); }

protected:
    Mutable<Source::Impl> createMutable() const noexcept final;

private:
    std::unique_ptr<Actor<CustomVectorTileLoader>> loader;
    mapbox::base::WeakPtrFactory<Source> weakFactory{this};
};

template <>
inline bool Source::is<CustomVectorSource>() const {
    return getType() == SourceType::CustomMVTVector;
}

} // namespace style
} // namespace mbgl
