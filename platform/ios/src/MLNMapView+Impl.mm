#import "MLNMapView+Impl.h"
#import "MLNMapView_Private.h"
#import "MLNStyle_Private.h"
#import "NSBundle+MLNAdditions.h"

#include <mbgl/plugin/cross_platform_plugin.hpp>

#if MLN_RENDER_BACKEND_METAL
#import "MLNMapView+Metal.h"
#else // MLN_RENDER_BACKEND_OPENGL
#import "MLNMapView+OpenGL.h"
#endif

std::unique_ptr<MLNMapViewImpl> MLNMapViewImpl::Create(MLNMapView* nativeView) {
#if MLN_RENDER_BACKEND_METAL
    return std::make_unique<MLNMapViewMetalImpl>(nativeView);
#else // MLN_RENDER_BACKEND_OPENGL
    return std::make_unique<MLNMapViewOpenGLImpl>(nativeView);
#endif
}

MLNMapViewImpl::MLNMapViewImpl(MLNMapView* nativeView_) : mapView(nativeView_) {
}

void MLNMapViewImpl::render() {
    [mapView renderSync];
}

void MLNMapViewImpl::onCameraWillChange(mbgl::MapObserver::CameraChangeMode mode) {
    bool animated = mode == mbgl::MapObserver::CameraChangeMode::Animated;
    [mapView cameraWillChangeAnimated:animated];

    for (auto& plugin : [mapView plugins]) {
        plugin->onCameraWillChange(mode);
    }
}

void MLNMapViewImpl::onCameraIsChanging() {
    [mapView cameraIsChanging];

    for (auto& plugin : [mapView plugins]) {
        plugin->onCameraIsChanging();
    }
}

void MLNMapViewImpl::onCameraDidChange(mbgl::MapObserver::CameraChangeMode mode) {
    bool animated = mode == mbgl::MapObserver::CameraChangeMode::Animated;
    [mapView cameraDidChangeAnimated:animated];

    for (auto& plugin : [mapView plugins]) {
        plugin->onCameraDidChange(mode);
    }
}

void MLNMapViewImpl::onWillStartLoadingMap() {
    [mapView mapViewWillStartLoadingMap];

    for (auto& plugin : [mapView plugins]) {
        plugin->onWillStartLoadingMap();
    }
}

void MLNMapViewImpl::onDidFinishLoadingMap() {
    [mapView mapViewDidFinishLoadingMap];

    for (auto& plugin : [mapView plugins]) {
        plugin->onDidFinishLoadingMap();
    }
}

void MLNMapViewImpl::onDidFailLoadingMap(mbgl::MapLoadError mapError, const std::string& what) {
    NSString *description;
    MLNErrorCode code;
    switch (mapError) {
        case mbgl::MapLoadError::StyleParseError:
            code = MLNErrorCodeParseStyleFailed;
            description = NSLocalizedStringWithDefaultValue(@"PARSE_STYLE_FAILED_DESC", nil, nil, @"The map failed to load because the style is corrupted.", @"User-friendly error description");
            break;
        case mbgl::MapLoadError::StyleLoadError:
            code = MLNErrorCodeLoadStyleFailed;
            description = NSLocalizedStringWithDefaultValue(@"LOAD_STYLE_FAILED_DESC", nil, nil, @"The map failed to load because the style can’t be loaded.", @"User-friendly error description");
            break;
        case mbgl::MapLoadError::NotFoundError:
            code = MLNErrorCodeNotFound;
            description = NSLocalizedStringWithDefaultValue(@"STYLE_NOT_FOUND_DESC", nil, nil, @"The map failed to load because the style can’t be found or is incompatible.", @"User-friendly error description");
            break;
        default:
            code = MLNErrorCodeUnknown;
            description = NSLocalizedStringWithDefaultValue(@"LOAD_MAP_FAILED_DESC", nil, nil, @"The map failed to load because an unknown error occurred.", @"User-friendly error description");
    }
    NSDictionary *userInfo = @{
        NSLocalizedDescriptionKey: description,
        NSLocalizedFailureReasonErrorKey: @(what.c_str()),
    };
    NSError *error = [NSError errorWithDomain:MLNErrorDomain code:code userInfo:userInfo];

    [mapView mapViewDidFailLoadingMapWithError:error];

    for (auto& plugin : [mapView plugins]) {
        plugin->onDidFailLoadingMap(mapError, what);
    }
}

void MLNMapViewImpl::onWillStartRenderingFrame() {
    [mapView mapViewWillStartRenderingFrame];

    for (auto& plugin : [mapView plugins]) {
        plugin->onWillStartRenderingFrame();
    }
}

void MLNMapViewImpl::onDidFinishRenderingFrame(const mbgl::MapObserver::RenderFrameStatus& status) {
    const bool fullyRendered = status.mode == mbgl::MapObserver::RenderMode::Full;
    [mapView mapViewDidFinishRenderingFrameFullyRendered:fullyRendered renderingStats:status.renderingStats];

    for (auto& plugin : [mapView plugins]) {
        plugin->onDidFinishRenderingFrame(status);
    }
}

void MLNMapViewImpl::onWillStartRenderingMap() {
    [mapView mapViewWillStartRenderingMap];

    for (auto& plugin : [mapView plugins]) {
        plugin->onWillStartRenderingMap();
    }
}

void MLNMapViewImpl::onDidFinishRenderingMap(mbgl::MapObserver::RenderMode mode) {
    bool fullyRendered = mode == mbgl::MapObserver::RenderMode::Full;
    [mapView mapViewDidFinishRenderingMapFullyRendered:fullyRendered];

    for (auto& plugin : [mapView plugins]) {
        plugin->onDidFinishRenderingMap(mode);
    }
}

void MLNMapViewImpl::onDidFinishLoadingStyle() {
    [mapView mapViewDidFinishLoadingStyle];

    for (auto& plugin : [mapView plugins]) {
        plugin->onDidFinishLoadingStyle();
    }
}

void MLNMapViewImpl::onSourceChanged(mbgl::style::Source& source) {
    NSString *identifier = @(source.getID().c_str());
    MLNSource * nativeSource = [mapView.style sourceWithIdentifier:identifier];
    [mapView sourceDidChange:nativeSource];

    for (auto& plugin : [mapView plugins]) {
        plugin->onSourceChanged(source);
    }
}

void MLNMapViewImpl::onDidBecomeIdle() {
    [mapView mapViewDidBecomeIdle];

    for (auto& plugin : [mapView plugins]) {
        plugin->onDidBecomeIdle();
    }
}

void MLNMapViewImpl::onStyleImageMissing(const std::string& imageIdentifier) {
    NSString *imageName = [NSString stringWithUTF8String:imageIdentifier.c_str()];
    [mapView didFailToLoadImage:imageName];

    for (auto& plugin : [mapView plugins]) {
        plugin->onStyleImageMissing(imageIdentifier);
    }
}

bool MLNMapViewImpl::onCanRemoveUnusedStyleImage(const std::string &imageIdentifier) {
    NSString *imageName = [NSString stringWithUTF8String:imageIdentifier.c_str()];
    bool canRemove = [mapView shouldRemoveStyleImage:imageName];

    // Plugins are notified of the call and can veto the result.
    // We don't short-circuit the loop to give a chance for each plugin to be notified.
    for (auto& plugin : [mapView plugins]) {
        canRemove = canRemove && plugin->onCanRemoveUnusedStyleImage(imageIdentifier);
    }

    return canRemove;
}

void MLNMapViewImpl::onRegisterShaders(mbgl::gfx::ShaderRegistry& shaders) {
    for (auto& plugin : [mapView plugins]) {
        plugin->onRegisterShaders(shaders);
    }
}

void MLNMapViewImpl::onPreCompileShader(mbgl::shaders::BuiltIn shaderID, mbgl::gfx::Backend::Type backend, const std::string& defines) {
    NSString *definesCopy = [NSString stringWithUTF8String:defines.c_str()];
    [mapView shaderWillCompile:static_cast<int>(shaderID) backend:static_cast<int>(backend) defines:definesCopy];

    for (auto& plugin : [mapView plugins]) {
        plugin->onPreCompileShader(shaderID, backend, defines);
    }
}

void MLNMapViewImpl::onPostCompileShader(mbgl::shaders::BuiltIn shaderID, mbgl::gfx::Backend::Type backend, const std::string& defines) {
    NSString *definesCopy = [NSString stringWithUTF8String:defines.c_str()];
    [mapView shaderDidCompile:static_cast<int>(shaderID) backend:static_cast<int>(backend) defines:definesCopy];

    for (auto& plugin : [mapView plugins]) {
        plugin->onPostCompileShader(shaderID, backend, defines);
    }
}

void MLNMapViewImpl::onShaderCompileFailed(mbgl::shaders::BuiltIn shaderID, mbgl::gfx::Backend::Type backend, const std::string& defines) {
    NSString *definesCopy = [NSString stringWithUTF8String:defines.c_str()];
    [mapView shaderDidFailCompile:static_cast<int>(shaderID) backend:static_cast<int>(backend) defines:definesCopy];

    for (auto& plugin : [mapView plugins]) {
        plugin->onShaderCompileFailed(shaderID, backend, defines);
    }
}

void MLNMapViewImpl::onGlyphsLoaded(const mbgl::FontStack& fontStack, const mbgl::GlyphRange& range) {
    NSMutableArray* fontStackCopy = [[NSMutableArray alloc] init];
    std::for_each(fontStack.begin(), fontStack.end(), ^(const std::string& str) {
        [fontStackCopy addObject:[NSString stringWithUTF8String:str.c_str()]];
    });

    [mapView glyphsDidLoad:fontStackCopy range:NSMakeRange(range.first, range.second - range.first)];

    for (auto& plugin : [mapView plugins]) {
        plugin->onGlyphsLoaded(fontStack, range);
    }
}

void MLNMapViewImpl::onGlyphsError(const mbgl::FontStack& fontStack, const mbgl::GlyphRange& range, std::exception_ptr error) {
    NSMutableArray* fontStackCopy = [[NSMutableArray alloc] init];
    std::for_each(fontStack.begin(), fontStack.end(), ^(const std::string& str) {
        [fontStackCopy addObject:[NSString stringWithUTF8String:str.c_str()]];
    });

    [mapView glyphsDidError:fontStackCopy range:NSMakeRange(range.first, range.second - range.first)];

    for (auto& plugin : [mapView plugins]) {
        plugin->onGlyphsError(fontStack, range, error);
    }
}

void MLNMapViewImpl::onGlyphsRequested(const mbgl::FontStack& fontStack, const mbgl::GlyphRange& range) {
    NSMutableArray* fontStackCopy = [[NSMutableArray alloc] init];
    std::for_each(fontStack.begin(), fontStack.end(), ^(const std::string& str) {
        [fontStackCopy addObject:[NSString stringWithUTF8String:str.c_str()]];
    });

    [mapView glyphsWillLoad:fontStackCopy range:NSMakeRange(range.first, range.second - range.first)];

    for (auto& plugin : [mapView plugins]) {
        plugin->onGlyphsRequested(fontStack, range);
    }
}

void MLNMapViewImpl::onTileAction(mbgl::TileOperation operation, const mbgl::OverscaledTileID& tile, const std::string& sourceID) {
    [mapView tileDidTriggerAction:MLNTileOperation(static_cast<int>(operation))
                                x:tile.canonical.x
                                y:tile.canonical.y
                                z:tile.canonical.z
                             wrap:tile.wrap
                      overscaledZ:tile.overscaledZ
                         sourceID:[NSString stringWithUTF8String:sourceID.c_str()]];

    for (auto& plugin : [mapView plugins]) {
        plugin->onTileAction(operation, tile, sourceID);
    }
}

void MLNMapViewImpl::onSpriteLoaded(const std::optional<mbgl::style::Sprite>& spriteID) {
    if (!spriteID.has_value()) {
        [mapView spriteDidLoad:nil url:nil];
    } else {
        [mapView spriteDidLoad:[NSString stringWithUTF8String:spriteID.value().id.c_str()]
                           url:[NSString stringWithUTF8String:spriteID.value().spriteURL.c_str()]];
    }

    for (auto& plugin : [mapView plugins]) {
        plugin->onSpriteLoaded(spriteID);
    }
}

void MLNMapViewImpl::onSpriteError(const std::optional<mbgl::style::Sprite>& spriteID, std::exception_ptr error) {
    if (!spriteID.has_value()) {
        [mapView spriteDidError:nil url:nil];
    } else {
        [mapView spriteDidError:[NSString stringWithUTF8String:spriteID.value().id.c_str()]
                            url:[NSString stringWithUTF8String:spriteID.value().spriteURL.c_str()]];
    }

    for (auto& plugin : [mapView plugins]) {
        plugin->onSpriteError(spriteID, error);
    }
}

void MLNMapViewImpl::onSpriteRequested(const std::optional<mbgl::style::Sprite>& spriteID) {
    if (!spriteID.has_value()) {
        [mapView spriteWillLoad:nil url:nil];
    } else {
        [mapView spriteWillLoad:[NSString stringWithUTF8String:spriteID.value().id.c_str()]
                            url:[NSString stringWithUTF8String:spriteID.value().spriteURL.c_str()]];
    }

    for (auto& plugin : [mapView plugins]) {
        plugin->onSpriteRequested(spriteID);
    }
}
