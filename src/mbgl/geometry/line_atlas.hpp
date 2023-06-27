#pragma once

#include <mbgl/gfx/texture.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/util/image.hpp>

#if MLN_DRAWABLE_RENDERER
#include <mbgl/gfx/texture2d.hpp>
#include <variant>
#else
#include <mbgl/util/variant.hpp>
#include <optional>
#endif

#include <map>
#include <memory>
#include <vector>

namespace mbgl {

namespace gfx {
class UploadPass;
} // namespace gfx

class LinePatternPos {
public:
    float width;
    float height;
    float y;
};

enum class LinePatternCap : bool {
    Square = false,
    Round = true,
};

struct DashRange {
    float left;
    float right;
    bool isDash;
    bool isZeroLength;
};

class DashPatternTexture {
public:
    DashPatternTexture(const std::vector<float>& from, const std::vector<float>& to, LinePatternCap);

    // Uploads the texture to the GPU to be available when we need it. This is a
    // lazy operation; the texture is only bound when the data is uploaded for
    // the first time.
    void upload(gfx::UploadPass&);

    // Binds the atlas texture to the GPU, and uploads data if it is out of date.
    gfx::TextureBinding textureBinding() const;
#if MLN_DRAWABLE_RENDERER
    const std::shared_ptr<gfx::Texture2D>& getTexture() const;
#endif
    
    // Returns the size of the texture image.
    Size getSize() const;

    const LinePatternPos& getFrom() const { return from; }
    const LinePatternPos& getTo() const { return to; }

private:
    LinePatternPos from, to;

#if MLN_DRAWABLE_RENDERER
    std::variant<AlphaImage, gfx::Texture2DPtr> texture;
#else
    variant<AlphaImage, gfx::Texture> texture;
#endif
};

class LineAtlas {
public:
    LineAtlas();
    ~LineAtlas();

    // Obtains or creates a texture that has both line patterns in it
    DashPatternTexture& getDashPatternTexture(const std::vector<float>& from,
                                              const std::vector<float>& to,
                                              LinePatternCap);

    // Uploads the textures to the GPU to be available when we need it.
    void upload(gfx::UploadPass&);

    bool isEmpty() const { return textures.empty(); }

private:
    std::map<size_t, DashPatternTexture> textures;

    // Stores a list of hashes of texture objects that need uploading.
    std::vector<size_t> needsUpload;
};

} // namespace mbgl
