#import "MLNMapView+Impl.h"
#import <MLNStyle_Private.h>
#import "NSBundle+MLNAdditions.h"

#if MLN_RENDER_BACKEND_METAL
#import "MLNMapView+Metal.h"
#else // MLN_RENDER_BACKEND_OPENGL
#import "MLNMapView+OpenGL.h"
#endif

#include <mbgl/map/map.hpp>
#include <mbgl/style/style.hpp>

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
}

void MLNMapViewImpl::onCameraIsChanging() {
    [mapView cameraIsChanging];
}

void MLNMapViewImpl::onCameraDidChange(mbgl::MapObserver::CameraChangeMode mode) {
    bool animated = mode == mbgl::MapObserver::CameraChangeMode::Animated;
    [mapView cameraDidChangeAnimated:animated];
}

void MLNMapViewImpl::onWillStartLoadingMap() {
    [mapView mapViewWillStartLoadingMap];
}

void MLNMapViewImpl::onDidFinishLoadingMap() {
    [mapView mapViewDidFinishLoadingMap];
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
            description = NSLocalizedStringWithDefaultValue(@"LOAD_STYLE_FAILED_DESC", nil, nil, @"The map failed to load because the style can't be loaded.", @"User-friendly error description");
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
}

void MLNMapViewImpl::onWillStartRenderingFrame() {
    [mapView mapViewWillStartRenderingFrame];
}

void MLNMapViewImpl::onDidFinishRenderingFrame(const mbgl::MapObserver::RenderFrameStatus& status) {
    bool fullyRendered = status.mode == mbgl::MapObserver::RenderMode::Full;
    [mapView mapViewDidFinishRenderingFrameFullyRendered:fullyRendered renderingStats:status.renderingStats];
}

void MLNMapViewImpl::onWillStartRenderingMap() {
    [mapView mapViewWillStartRenderingMap];
}

void MLNMapViewImpl::onDidFinishRenderingMap(mbgl::MapObserver::RenderMode mode) {
    bool fullyRendered = mode == mbgl::MapObserver::RenderMode::Full;
    [mapView mapViewDidFinishRenderingMapFullyRendered:fullyRendered];
}

void MLNMapViewImpl::onDidBecomeIdle() {
    [mapView mapViewDidBecomeIdle];
}

void MLNMapViewImpl::onDidFinishLoadingStyle() {
    [mapView mapViewDidFinishLoadingStyle];
}

void MLNMapViewImpl::onSourceChanged(mbgl::style::Source& source) {
    NSString *identifier = @(source.getID().c_str());
    MLNSource * nativeSource = [mapView.style sourceWithIdentifier:identifier];
    [mapView sourceDidChange:nativeSource];
}

bool MLNMapViewImpl::onCanRemoveUnusedStyleImage(const std::string &imageIdentifier) {
    NSString *imageName = [NSString stringWithUTF8String:imageIdentifier.c_str()];
    return [mapView shouldRemoveStyleImage:imageName];
}

void MLNMapViewImpl::onRegisterShaders(mbgl::gfx::ShaderRegistry& shaders) {

}

void MLNMapViewImpl::onPreCompileShader(mbgl::shaders::BuiltIn shaderID, mbgl::gfx::Backend::Type backend, const std::string& defines) {
    NSString *definesCopy = [NSString stringWithUTF8String:defines.c_str()];
    [mapView shaderWillCompile:static_cast<int>(shaderID) backend:static_cast<int>(backend) defines:definesCopy];
}

void MLNMapViewImpl::onPostCompileShader(mbgl::shaders::BuiltIn shaderID, mbgl::gfx::Backend::Type backend, const std::string& defines) {
    NSString *definesCopy = [NSString stringWithUTF8String:defines.c_str()];
    [mapView shaderDidCompile:static_cast<int>(shaderID) backend:static_cast<int>(backend) defines:definesCopy];
}

void MLNMapViewImpl::onShaderCompileFailed(mbgl::shaders::BuiltIn shaderID, mbgl::gfx::Backend::Type backend, const std::string& defines) {
    NSString *definesCopy = [NSString stringWithUTF8String:defines.c_str()];
    [mapView shaderDidFailCompile:static_cast<int>(shaderID) backend:static_cast<int>(backend) defines:definesCopy];
}

void MLNMapViewImpl::onGlyphsLoaded(const mbgl::FontStack& fontStack, const mbgl::GlyphRange& range) {
    NSMutableArray* fontStackCopy = [[NSMutableArray alloc] init];
    std::for_each(fontStack.begin(), fontStack.end(), ^(const std::string& str) {
        [fontStackCopy addObject:[NSString stringWithUTF8String:str.c_str()]];
    });

    [mapView glyphsDidLoad:fontStackCopy range:NSMakeRange(range.first, range.second - range.first)];
}

void MLNMapViewImpl::onGlyphsError(const mbgl::FontStack& fontStack, const mbgl::GlyphRange& range, std::exception_ptr error) {
    NSMutableArray* fontStackCopy = [[NSMutableArray alloc] init];
    std::for_each(fontStack.begin(), fontStack.end(), ^(const std::string& str) {
        [fontStackCopy addObject:[NSString stringWithUTF8String:str.c_str()]];
    });

    [mapView glyphsDidError:fontStackCopy range:NSMakeRange(range.first, range.second - range.first)];
}

void MLNMapViewImpl::onGlyphsRequested(const mbgl::FontStack& fontStack, const mbgl::GlyphRange& range) {
    NSMutableArray* fontStackCopy = [[NSMutableArray alloc] init];
    std::for_each(fontStack.begin(), fontStack.end(), ^(const std::string& str) {
        [fontStackCopy addObject:[NSString stringWithUTF8String:str.c_str()]];
    });

    [mapView glyphsWillLoad:fontStackCopy range:NSMakeRange(range.first, range.second - range.first)];
}

void MLNMapViewImpl::onTileAction(mbgl::TileOperation operation, const mbgl::OverscaledTileID& tile, const std::string& sourceID) {
    [mapView tileDidTriggerAction:MLNTileOperation(static_cast<int>(operation))
                                x:tile.canonical.x
                                y:tile.canonical.y
                                z:tile.canonical.z
                             wrap:tile.wrap
                      overscaledZ:tile.overscaledZ
                         sourceID:[NSString stringWithUTF8String:sourceID.c_str()]];
}

void MLNMapViewImpl::onSpriteLoaded(const std::optional<mbgl::style::Sprite>& spriteID) {
    if (!spriteID.has_value()) {
        [mapView spriteDidLoad:nil url:nil];
        return;
    }

    [mapView spriteDidLoad:[NSString stringWithUTF8String:spriteID.value().id.c_str()]
                       url:[NSString stringWithUTF8String:spriteID.value().spriteURL.c_str()]];
}

void MLNMapViewImpl::onSpriteError(const std::optional<mbgl::style::Sprite>& spriteID, std::exception_ptr error) {
    if (!spriteID.has_value()) {
        [mapView spriteDidError:nil url:nil];
        return;
    }

    [mapView spriteDidError:[NSString stringWithUTF8String:spriteID.value().id.c_str()]
                        url:[NSString stringWithUTF8String:spriteID.value().spriteURL.c_str()]];
}

void MLNMapViewImpl::onSpriteRequested(const std::optional<mbgl::style::Sprite>& spriteID) {
    if (!spriteID.has_value()) {
        [mapView spriteWillLoad:nil url:nil];
        return;
    }

    [mapView spriteWillLoad:[NSString stringWithUTF8String:spriteID.value().id.c_str()]
                        url:[NSString stringWithUTF8String:spriteID.value().spriteURL.c_str()]];
}
