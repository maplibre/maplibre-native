#include <mbgl/style/image.hpp>
#include <mbgl/util/exception.hpp>
#include "image.hpp"

namespace mbgl {
namespace android {

mbgl::style::Image Image::getImage(jni::JNIEnv& env, const jni::Object<Image>& image) {
    static auto& javaClass = jni::Class<Image>::Singleton(env);
    static auto widthField = javaClass.GetField<jni::jint>(env, "width");
    static auto heightField = javaClass.GetField<jni::jint>(env, "height");
    static auto pixelRatioField = javaClass.GetField<jni::jfloat>(env, "pixelRatio");
    static auto bufferField = javaClass.GetField<jni::Array<jbyte>>(env, "buffer");
    static auto nameField = javaClass.GetField<jni::String>(env, "name");
    static auto sdfField = javaClass.GetField<jni::jboolean>(env, "sdf");
    static auto contentField = javaClass.GetField<jni::Array<jfloat >>(env, "content");
    static auto stretchXField = javaClass.GetField<jni::Array<jfloat>>(env, "stretchX");
    static auto stretchYField = javaClass.GetField<jni::Array<jfloat>>(env, "stretchY");

    auto height = image.Get(env, heightField);
    auto width = image.Get(env, widthField);
    auto pixelRatio = image.Get(env, pixelRatioField);
    auto pixels = image.Get(env, bufferField);
    auto name = jni::Make<std::string>(env, image.Get(env, nameField));
    auto sdf = (bool) image.Get(env, sdfField);
    auto content = image.Get(env, contentField);
    auto stretchX = image.Get(env, stretchXField);
    auto stretchY = image.Get(env, stretchYField);
    jni::NullCheck(env, pixels.get());
    std::size_t size = pixels.Length(env);

    mbgl::PremultipliedImage premultipliedImage({ static_cast<uint32_t>(width), static_cast<uint32_t>(height) });
    if (premultipliedImage.bytes() != uint32_t(size)) {
        throw mbgl::util::StyleImageException("Image pixel count mismatch");
    }

    jni::GetArrayRegion(env, *pixels, 0, size,
                        reinterpret_cast<jbyte *>(premultipliedImage.data.get()));

    style::ImageStretches imageStretchesX = {};
    style::ImageStretches imageStretchesY = {};
    if (stretchX.get()) {
        std::size_t sizeX = stretchX.Length(env);
        for (jni::jsize i = 0; i < sizeX - 1; i += 2) {
            imageStretchesX.push_back({stretchX.Get(env, i), stretchX.Get(env, i + 1)});
        }
    }
    if (stretchY.get()) {
        std::size_t sizeY = stretchY.Length(env);
        for (jni::jsize i = 0; i < sizeY - 1; i += 2) {
            imageStretchesY.push_back({stretchY.Get(env, i), stretchY.Get(env, i + 1)});
        }
    }

    if (content) {
        const style::ImageContent imageContent = style::ImageContent{content.Get(env, 0),
                                                                     content.Get(env, 1),
                                                                     content.Get(env, 2),
                                                                     content.Get(env, 3)};
        return mbgl::style::Image{name, std::move(premultipliedImage), pixelRatio, sdf,
                                  imageStretchesX,
                                  imageStretchesY,
                                  std::move(imageContent)};
    }

    return mbgl::style::Image{name, std::move(premultipliedImage), pixelRatio, sdf,
                              imageStretchesX,
                              imageStretchesY};
}

void Image::registerNative(jni::JNIEnv &env) {
    jni::Class<Image>::Singleton(env);
}

} // namespace android
} // namespace mb
