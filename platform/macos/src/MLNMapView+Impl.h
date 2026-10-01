#import <mln/gfx/renderer_backend.hpp>
#import <mln/map/map_observer.hpp>
#import <mln/util/image.hpp>

#import "MLNBackendResource.h"

@class MLNMapView;

typedef struct _CGLContextObject* CGLContextObj;

class MLNMapViewImpl : public mln::MapObserver {
public:
  static std::unique_ptr<MLNMapViewImpl> Create(MLNMapView*);

  MLNMapViewImpl(MLNMapView*);
  virtual ~MLNMapViewImpl() = default;

  virtual mln::gfx::RendererBackend& getRendererBackend() = 0;

  // We need a static image of what was rendered for printing.
  virtual mln::PremultipliedImage readStillImage() = 0;

  virtual CGLContextObj getCGLContextObj() { return nullptr; }

  virtual void display();

#if MLN_RENDER_BACKEND_METAL
  // Returns the backend resource for Metal rendering in custom layers
  virtual MLNBackendResource* getObject() { return nullptr; }
#endif

  // Called by the view delegate when it's time to render.
  void render();

  // mln::MapObserver implementation
  void onCameraWillChange(mln::MapObserver::CameraChangeMode) override;
  void onCameraIsChanging() override;
  void onCameraDidChange(mln::MapObserver::CameraChangeMode) override;
  void onWillStartLoadingMap() override;
  void onDidFinishLoadingMap() override;
  void onDidFailLoadingMap(mln::MapLoadError mapError, const std::string& what) override;
  void onWillStartRenderingFrame() override;
  void onDidFinishRenderingFrame(const mln::MapObserver::RenderFrameStatus&) override;
  void onWillStartRenderingMap() override;
  void onDidFinishRenderingMap(mln::MapObserver::RenderMode) override;
  void onDidFinishLoadingStyle() override;
  void onSourceChanged(mln::style::Source& source) override;
  void onDidBecomeIdle() override;
  bool onCanRemoveUnusedStyleImage(const std::string& imageIdentifier) override;
  void onRegisterShaders(mln::gfx::ShaderRegistry&) override;
  void onPreCompileShader(mln::shaders::BuiltIn, mln::gfx::Backend::Type,
                          const std::string&) override;
  void onPostCompileShader(mln::shaders::BuiltIn, mln::gfx::Backend::Type,
                           const std::string&) override;
  void onShaderCompileFailed(mln::shaders::BuiltIn, mln::gfx::Backend::Type,
                             const std::string&) override;
  void onGlyphsLoaded(const mln::FontStack&, const mln::GlyphRange&) override;
  void onGlyphsError(const mln::FontStack&, const mln::GlyphRange&, std::exception_ptr) override;
  void onGlyphsRequested(const mln::FontStack&, const mln::GlyphRange&) override;
  void onTileAction(mln::TileOperation, const mln::OverscaledTileID&, const std::string&) override;
  void onSpriteLoaded(const std::optional<mln::style::Sprite>&) override;
  void onSpriteError(const std::optional<mln::style::Sprite>&, std::exception_ptr) override;
  void onSpriteRequested(const std::optional<mln::style::Sprite>&) override;

protected:
  /// Cocoa map view that this adapter bridges to.
  __weak MLNMapView* mapView = nullptr;
};
