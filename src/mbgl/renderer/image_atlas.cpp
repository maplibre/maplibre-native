#include <mbgl/renderer/image_atlas.hpp>
#include <mbgl/renderer/image_manager.hpp>
#include <mbgl/util/logging.hpp>

#include <mapbox/shelf-pack.hpp>

namespace mbgl {

static constexpr uint32_t padding = 1;

ImagePosition::ImagePosition(const mapbox::Bin& bin, const style::Image::Impl& image, uint32_t version_)
    : pixelRatio(image.pixelRatio),
      paddedRect(bin.x, bin.y, bin.w, bin.h),
      version(version_),
      stretchX(image.stretchX),
      stretchY(image.stretchY),
      content(image.content),
      textFitWidth(image.textFitWidth),
      textFitHeight(image.textFitHeight) {}

namespace {

const mapbox::Bin& _packImage(mapbox::ShelfPack& pack,
                              const style::Image::Impl& image,
                              ImageAtlas& resultImage,
                              ImageType imageType) {
    const mapbox::Bin& bin = *pack.packOne(
        -1, image.image.size.width + 2 * padding, image.image.size.height + 2 * padding);

    resultImage.image.resize({static_cast<uint32_t>(pack.width()), static_cast<uint32_t>(pack.height())});

    PremultipliedImage::copy(
        image.image, resultImage.image, {0, 0}, {bin.x + padding, bin.y + padding}, image.image.size);

    if (imageType == ImageType::Pattern) {
        const uint32_t x = bin.x + padding;
        const uint32_t y = bin.y + padding;
        const uint32_t w = image.image.size.width;
        const uint32_t h = image.image.size.height;

        // Add 1 pixel wrapped padding on each side of the image.
        PremultipliedImage::copy(image.image, resultImage.image, {0, h - 1}, {x, y - 1}, {w, 1}); // T
        PremultipliedImage::copy(image.image, resultImage.image, {0, 0}, {x, y + h}, {w, 1});     // B
        PremultipliedImage::copy(image.image, resultImage.image, {w - 1, 0}, {x - 1, y}, {1, h}); // L
        PremultipliedImage::copy(image.image, resultImage.image, {0, 0}, {x + w, y}, {1, h});     // R
    }
    return bin;
}

void populateImagePatches(ImagePositions& imagePositions,
                          const ImageManager& imageManager,
                          std::vector<ImagePatch>& /*out*/ patches) {
    if (imagePositions.empty()) {
        imagePositions.reserve(imageManager.updatedImageVersions.size());
    }
    for (auto& updatedImageVersion : imageManager.updatedImageVersions) {
        const std::string& name = updatedImageVersion.first;
        const uint32_t version = updatedImageVersion.second;
        const auto it = imagePositions.find(updatedImageVersion.first);
        if (it != imagePositions.end()) {
            auto& position = it->second;
            if (position.version == version) continue;

            const auto updatedImage = imageManager.getSharedImage(name);
            if (updatedImage == nullptr) continue;

            Immutable<style::Image::Impl> imagePatch = *updatedImage;

            // The max patch size that fits the current Atlas is the paddedRect size and the padding
            assert(position.paddedRect.w > ImagePosition::padding * 2);
            assert(position.paddedRect.h > ImagePosition::padding * 2);
            uint32_t maxPatchWidth = position.paddedRect.w - ImagePosition::padding * 2;
            uint32_t maxPatchHeight = position.paddedRect.h - ImagePosition::padding * 2;
            if (maxPatchWidth < imagePatch->image.size.width || maxPatchHeight < imagePatch->image.size.height) {
                // imagePositions are created in makeImageAtlas
                // User can update the image. e.g. an Android call to
                // MapLibreMap.getStyle.addImage(imageId, imageBitmap), which will call ImageManager.updateImage
                // If the updated image is larger than the previous image then position.paddedRect area in the atlas
                // won't fit the new image. ImageManager is unaware of the the atlas packing.
                // This code simply prints an error message and resizes the image to fit the atlas to avoid crashes
                // A better solution (potentially expensive) is to repack the atlas: this requires keeping the
                // previous atlas image and detect when a repack is required.
                // Another possibility is to simply throw an exception and requires the user to provide different
                // IDs for images with different sizes
                const auto& imageImpl = *updatedImage->get();
                Log::Error(Event::General,
                           imageImpl.id + " does not fit the atlas. " + imageImpl.id +
                               " will be resized from:" + std::to_string(imageImpl.image.size.width) + "x" +
                               std::to_string(imageImpl.image.size.height) + " to:" + std::to_string(maxPatchWidth) +
                               "x" + std::to_string(maxPatchHeight));
                auto resizedImage = imagePatch->image.clone();
                auto newWidth = std::min(maxPatchWidth, imagePatch->image.size.width);
                auto newHeight = std::min(maxPatchHeight, imagePatch->image.size.height);
                resizedImage.resize({newWidth, newHeight});
                auto mutableImagePatch = makeMutable<style::Image::Impl>(imageImpl.id,
                                                                         std::move(resizedImage),
                                                                         imageImpl.pixelRatio,
                                                                         imageImpl.sdf,
                                                                         style::ImageStretches(),
                                                                         style::ImageStretches());
                imagePatch = std::move(mutableImagePatch);
            }

            patches.emplace_back(imagePatch, position.paddedRect);
            position.version = version;
        }
    }
}

} // namespace

std::vector<ImagePatch> ImageAtlas::getImagePatchesAndUpdateVersions(const ImageManager& imageManager) {
    std::vector<ImagePatch> imagePatches;
    populateImagePatches(iconPositions, imageManager, imagePatches);
    populateImagePatches(patternPositions, imageManager, imagePatches);
    return imagePatches;
}

ImageAtlas makeImageAtlas(const ImageMap& icons, const ImageMap& patterns, const ImageVersionMap& versionMap) {
    ImageAtlas result;

    mapbox::ShelfPack::ShelfPackOptions options;
    options.autoResize = true;
    mapbox::ShelfPack pack(0, 0, options);

    result.iconPositions.reserve(icons.size());

    for (const auto& entry : icons) {
        const style::Image::Impl& image = *entry.second;
        const mapbox::Bin& bin = _packImage(pack, image, result, ImageType::Icon);
        const auto it = versionMap.find(entry.first);
        const auto version = it != versionMap.end() ? it->second : 0;
        result.iconPositions.emplace(image.id, ImagePosition{bin, image, version});
    }

    result.patternPositions.reserve(patterns.size());

    for (const auto& entry : patterns) {
        const style::Image::Impl& image = *entry.second;
        const mapbox::Bin& bin = _packImage(pack, image, result, ImageType::Pattern);
        const auto it = versionMap.find(entry.first);
        const auto version = it != versionMap.end() ? it->second : 0;
        result.patternPositions.emplace(image.id, ImagePosition{bin, image, version});
    }

    pack.shrink();
    result.image.resize({static_cast<uint32_t>(pack.width()), static_cast<uint32_t>(pack.height())});

    return result;
}

} // namespace mbgl
