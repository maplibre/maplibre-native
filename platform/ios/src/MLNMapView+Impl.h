#import <mbgl/gfx/renderer_backend.hpp>
#import <mbgl/map/map_observer.hpp>
#import <mbgl/util/image.hpp>

#import "MLNBackendResource.h"

#import <QuartzCore/CALayer.h>
#import <UIKit/UIImage.h>
#import <UIKit/UIView.h>

@class MLNMapView;

class MLNMapViewImpl : public mbgl::MapObserver {
 public:
  static std::unique_ptr<MLNMapViewImpl> Create(MLNMapView*);

  MLNMapViewImpl(MLNMapView*);
  virtual ~MLNMapViewImpl() = default;

  virtual mbgl::gfx::RendererBackend& getRendererBackend() = 0;

  // Returns a handle to the OpenGL context object if this view is rendered with OpenGL.
  virtual EAGLContext* getEAGLContext() { return nullptr; }

  // Gets called when the opaqueness of the view changes.
  virtual void setOpaque(bool) = 0;

  // Triggers an immediate render of the underlying view.
  virtual void display() = 0;

  // We update the transaction mode when the user adds annotation views that need to be layered on
  // top of the view.
  virtual void setPresentsWithTransaction(bool) = 0;

  // Called when initially creating the rendering view, and when resuming rendering after returning
  // from the background.
  virtual void createView() = 0;

  // We expose the underlying UIView because we need it in order to add annotation views above it.
  virtual UIView* getView() = 0;

  // Called when the application goes to the background and we've replaced the interactively
  // rendered map view with a static image.
  virtual void deleteView() = 0;

  // called before the application goes to the background. The resulting image is used as a place
  // holder instead of the regular view. This allows us to drop the framebuffers associated with
  // the rendering context and reduce memory while in the background.
  virtual UIImage* snapshot() = 0;

  // Called when UIView's layout has changed.
  virtual void layoutChanged() {};

  // Called by the view delegate when it's time to render.
  void render();

  virtual MLNBackendResource* getObject() = 0;

  // mbgl::MapObserver implementation
  void onCameraWillChange(mbgl::MapObserver::CameraChangeMode) override;
  void onCameraIsChanging() override;
  void onCameraDidChange(mbgl::MapObserver::CameraChangeMode) override;
  void onWillStartLoadingMap() override;
  void onDidFinishLoadingMap() override;
  void onDidFailLoadingMap(mbgl::MapLoadError mapError, const std::string& what) override;
  void onWillStartRenderingFrame() override;
  void onDidFinishRenderingFrame(const mbgl::MapObserver::RenderFrameStatus&) override;
  void onWillStartRenderingMap() override;
  void onDidFinishRenderingMap(mbgl::MapObserver::RenderMode) override;
  void onDidFinishLoadingStyle() override;
  void onSourceChanged(mbgl::style::Source& source) override;
  void onDidBecomeIdle() override;
  void onStyleImageMissing(const std::string& imageIdentifier) override;
  bool onCanRemoveUnusedStyleImage(const std::string& imageIdentifier) override;
  void onRegisterShaders(mbgl::gfx::ShaderRegistry&) override;
  void onPreCompileShader(mbgl::shaders::BuiltIn, mbgl::gfx::Backend::Type,
                          const std::string&) override;
  void onPostCompileShader(mbgl::shaders::BuiltIn, mbgl::gfx::Backend::Type,
                           const std::string&) override;
  void onShaderCompileFailed(mbgl::shaders::BuiltIn, mbgl::gfx::Backend::Type,
                             const std::string&) override;
  void onGlyphsLoaded(const mbgl::FontStack&, const mbgl::GlyphRange&) override;
  void onGlyphsError(const mbgl::FontStack&, const mbgl::GlyphRange&, std::exception_ptr) override;
  void onGlyphsRequested(const mbgl::FontStack&, const mbgl::GlyphRange&) override;
  void onTileAction(mbgl::TileOperation, const mbgl::OverscaledTileID&,
                    const std::string&) override;
  void onSpriteLoaded(const std::optional<mbgl::style::Sprite>&) override;
  void onSpriteError(const std::optional<mbgl::style::Sprite>&, std::exception_ptr) override;
  void onSpriteRequested(const std::optional<mbgl::style::Sprite>&) override;

 protected:
  /// Cocoa map view that this adapter bridges to.
  __weak MLNMapView* mapView = nullptr;
};
