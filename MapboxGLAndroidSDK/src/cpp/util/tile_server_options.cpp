#include "tile_server_options.hpp"

namespace mbgl {
namespace android {

jni::Local<jni::Object<TileServerOptions>> TileServerOptions::New(jni::JNIEnv& env, const mbgl::TileServerOptions& tileServerOptions) {
    static auto& javaClass = jni::Class<TileServerOptions>::Singleton(env);
    static auto constructor = javaClass.GetConstructor<
        jni::String, jni::String, jni::String, jni::String, jni::String, jni::String,
        jni::String, jni::String, jni::String, jni::String, jni::String,
        jni::String, jni::String, jni::String, jni::String, jni::String, 
        jni::String, jni::String, jni::jboolean, jni::String, jni::Array<jni::Object<DefaultStyle>>>(env);
    
    optional<std::string> sourceVersionPrefixValue = tileServerOptions.sourceVersionPrefix();
    optional<std::string> styleVersionPrefixValue = tileServerOptions.styleVersionPrefix();
    optional<std::string> spritesVersionPrefixValue = tileServerOptions.spritesVersionPrefix();
    optional<std::string> glyphsVersionPrefixValue = tileServerOptions.glyphsVersionPrefix();
    optional<std::string> tileVersionPrefixValue = tileServerOptions.tileVersionPrefix();

    return javaClass.New(env, constructor,
        jni::Make<jni::String>(env, tileServerOptions.baseURL()),
        jni::Make<jni::String>(env, tileServerOptions.uriSchemeAlias()),
        jni::Make<jni::String>(env, tileServerOptions.sourceTemplate()),
        jni::Make<jni::String>(env, tileServerOptions.sourceDomainName()),
        sourceVersionPrefixValue ? jni::Make<jni::String>(env, *sourceVersionPrefixValue) : jni::Local<jni::String>(),
        jni::Make<jni::String>(env, tileServerOptions.styleTemplate()),
        jni::Make<jni::String>(env, tileServerOptions.styleDomainName()),
        styleVersionPrefixValue ? jni::Make<jni::String>(env, *styleVersionPrefixValue) : jni::Local<jni::String>(),
        jni::Make<jni::String>(env, tileServerOptions.spritesTemplate()),
        jni::Make<jni::String>(env, tileServerOptions.spritesDomainName()),
        spritesVersionPrefixValue ? jni::Make<jni::String>(env, *spritesVersionPrefixValue) : jni::Local<jni::String>(),
        jni::Make<jni::String>(env, tileServerOptions.glyphsTemplate()),
        jni::Make<jni::String>(env, tileServerOptions.glyphsDomainName()),
        glyphsVersionPrefixValue ? jni::Make<jni::String>(env, *glyphsVersionPrefixValue) : jni::Local<jni::String>(),
        jni::Make<jni::String>(env, tileServerOptions.tileTemplate()),
        jni::Make<jni::String>(env, tileServerOptions.tileDomainName()),
        tileVersionPrefixValue ? jni::Make<jni::String>(env, *tileVersionPrefixValue) : jni::Local<jni::String>(),
        jni::Make<jni::String>(env, tileServerOptions.apiKeyParameterName()),
        jni::jboolean(tileServerOptions.requiresApiKey()),
        jni::Make<jni::String>(env, tileServerOptions.defaultStyle()),
        TileServerOptions::NewStyles(env, tileServerOptions.defaultStyles())
    );
}

jni::Local<jni::Array<jni::Object<DefaultStyle>>> TileServerOptions::NewStyles(jni::JNIEnv& env, const std::vector<mbgl::util::DefaultStyle> &nativeStyles) {

    auto retVal = jni::Array<jni::Object<DefaultStyle>>::New(env,  nativeStyles.size());
    for (auto it = begin(nativeStyles); it != end(nativeStyles); ++it) {
        auto converted = DefaultStyle::New(env, *it);
        retVal.Set(env, it - nativeStyles.begin(), converted);
    }

    return retVal;
}

jni::Local<jni::Object<TileServerOptions>> TileServerOptions::DefaultConfiguration(jni::JNIEnv& env, const jni::Class<TileServerOptions>& jOptions) {
    auto options = mbgl::TileServerOptions::DefaultConfiguration();
    return TileServerOptions::New(env, options);
}

jni::Local<jni::Object<TileServerOptions>> TileServerOptions::MapboxConfiguration(jni::JNIEnv& env, const jni::Class<TileServerOptions>& jOptions) {
    auto options = mbgl::TileServerOptions::MapboxConfiguration();
    return TileServerOptions::New(env, options);
}

jni::Local<jni::Object<TileServerOptions>> TileServerOptions::MapTilerConfiguration(jni::JNIEnv& env, const jni::Class<TileServerOptions>& jOptions) {
    auto options = mbgl::TileServerOptions::MapTilerConfiguration();
    return TileServerOptions::New(env, options);
}

jni::Local<jni::Object<TileServerOptions>> TileServerOptions::MapLibreConfiguration(jni::JNIEnv& env, const jni::Class<TileServerOptions>& jOptions) {
    auto options = mbgl::TileServerOptions::MapLibreConfiguration();
    return TileServerOptions::New(env, options);
}

mbgl::TileServerOptions TileServerOptions::getTileServerOptions(jni::JNIEnv& env, const jni::Object<TileServerOptions>& options) {
    auto opts = mbgl::TileServerOptions();
    static auto& javaClass = jni::Class<TileServerOptions>::Singleton(env);

    static auto baseURLField = javaClass.GetField<jni::String>(env, "baseURL");

    static auto uriSchemeAliasField = javaClass.GetField<jni::String>(env, "uriSchemeAlias");

    static auto sourceTemplateField = javaClass.GetField<jni::String>(env, "sourceTemplate");
    static auto sourceDomainNameField = javaClass.GetField<jni::String>(env, "sourceDomainName");
    static auto sourceVersionPrefixField = javaClass.GetField<jni::String>(env, "sourceVersionPrefix");

    static auto styleTemplateField = javaClass.GetField<jni::String>(env, "styleTemplate");
    static auto styleDomainNameField = javaClass.GetField<jni::String>(env, "styleDomainName");
    static auto styleVersionPrefixField = javaClass.GetField<jni::String>(env, "styleVersionPrefix");

    static auto spritesTemplateField = javaClass.GetField<jni::String>(env, "spritesTemplate");
    static auto spritesDomainNameField = javaClass.GetField<jni::String>(env, "spritesDomainName");
    static auto spritesVersionPrefixField = javaClass.GetField<jni::String>(env, "spritesVersionPrefix");

    static auto glyphsTemplateField = javaClass.GetField<jni::String>(env, "glyphsTemplate");
    static auto glyphsDomainNameField = javaClass.GetField<jni::String>(env, "glyphsDomainName");
    static auto glyphsVersionPrefixField = javaClass.GetField<jni::String>(env, "glyphsVersionPrefix");

    static auto tileTemplateField = javaClass.GetField<jni::String>(env, "tileTemplate");
    static auto tileDomainNameField = javaClass.GetField<jni::String>(env, "tileDomainName");
    static auto tileVersionPrefixField = javaClass.GetField<jni::String>(env, "tileVersionPrefix");

    static auto apiKeyParameterNameField = javaClass.GetField<jni::String>(env, "apiKeyParameterName");
    static auto apiKeyRequiredField = javaClass.GetField<jni::jboolean>(env, "apiKeyRequired");

    static auto defaultStyleField = javaClass.GetField<jni::String>(env, "defaultStyle");

    static auto defaultStylesField = javaClass.GetField<jni::Array<jni::Object<DefaultStyle>>>(env, "defaultStyles");
    std::vector<mbgl::util::DefaultStyle> defaultStyles = TileServerOptions::getDefaultStyles(env, options.Get(env, defaultStylesField));

    auto retVal = mbgl::TileServerOptions()
        .withBaseURL(jni::Make<std::string>(env, options.Get(env, baseURLField)))
        .withUriSchemeAlias(jni::Make<std::string>(env, options.Get(env, uriSchemeAliasField)))
        .withApiKeyParameterName(jni::Make<std::string>(env, options.Get(env, apiKeyParameterNameField)))
        .setRequiresApiKey(options.Get(env, apiKeyRequiredField));
    
    auto sourcePrefixValue = options.Get(env, sourceVersionPrefixField);
    retVal.withSourceTemplate(
        jni::Make<std::string>(env, options.Get(env, sourceTemplateField)),
        jni::Make<std::string>(env, options.Get(env, sourceDomainNameField)),
        sourcePrefixValue ? jni::Make<std::string>(env, sourcePrefixValue): optional<std::string>{});

    auto styleVersionPrefixValue = options.Get(env, styleVersionPrefixField);
    retVal.withStyleTemplate(
        jni::Make<std::string>(env, options.Get(env, styleTemplateField)),
        jni::Make<std::string>(env, options.Get(env, styleDomainNameField)),
        styleVersionPrefixValue ? jni::Make<std::string>(env, styleVersionPrefixValue): optional<std::string>{});

    auto spritesVersionPrefixValue = options.Get(env, spritesVersionPrefixField);
    retVal.withSpritesTemplate(
            jni::Make<std::string>(env, options.Get(env, spritesTemplateField)),
            jni::Make<std::string>(env, options.Get(env, spritesDomainNameField)),
            spritesVersionPrefixValue ? jni::Make<std::string>(env, spritesVersionPrefixValue): optional<std::string>{});

    auto glyphsVersionPrefixValue = options.Get(env, glyphsVersionPrefixField);           
    retVal.withGlyphsTemplate(
            jni::Make<std::string>(env, options.Get(env, glyphsTemplateField)),
            jni::Make<std::string>(env, options.Get(env, glyphsDomainNameField)),
            glyphsVersionPrefixValue ? jni::Make<std::string>(env, glyphsVersionPrefixValue) : optional<std::string>{});

    auto tileVersionPrefixValue = options.Get(env, tileVersionPrefixField);        
    retVal.withTileTemplate(
            jni::Make<std::string>(env, options.Get(env, tileTemplateField)),
            jni::Make<std::string>(env, options.Get(env, tileDomainNameField)),
            tileVersionPrefixValue ? jni::Make<std::string>(env, tileVersionPrefixValue): optional<std::string>{});

    retVal.withDefaultStyle(jni::Make<std::string>(env, options.Get(env, defaultStyleField)));
    retVal.withDefaultStyles(defaultStyles);

    return retVal;
}

std::vector<mbgl::util::DefaultStyle> TileServerOptions::getDefaultStyles(jni::JNIEnv& env, const jni::Array<jni::Object<DefaultStyle>>& styles_) {

    std::size_t length = styles_.Length(env);
    std::vector<mbgl::util::DefaultStyle> convertedStyles;
    //convertedStyles.reserve(length);
    for (std::size_t i = 0; i < length; i++) {
        mbgl::util::DefaultStyle converted = DefaultStyle::getDefaultStyle(env, styles_.Get(env, i));
        convertedStyles.push_back(converted);
    }

    return convertedStyles;
}

void TileServerOptions::registerNative(jni::JNIEnv& env) {
    static auto& javaClass = jni::Class<TileServerOptions>::Singleton(env);
    jni::RegisterNatives(env,
                        *javaClass,
                        jni::MakeNativeMethod<decltype(&TileServerOptions::DefaultConfiguration), &TileServerOptions::DefaultConfiguration>("defaultConfiguration"),
                        jni::MakeNativeMethod<decltype(&TileServerOptions::MapboxConfiguration), &TileServerOptions::MapboxConfiguration>("mapboxConfiguration"),
                        jni::MakeNativeMethod<decltype(&TileServerOptions::MapTilerConfiguration), &TileServerOptions::MapTilerConfiguration>("mapTilerConfiguration"),
                        jni::MakeNativeMethod<decltype(&TileServerOptions::MapLibreConfiguration), &TileServerOptions::MapLibreConfiguration>("mapLibreConfiguration"));
}

} // namespace android
} // namespace mbgl
