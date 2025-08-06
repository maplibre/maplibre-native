#include <mbgl/layermanager/layer_manager.hpp>
#include <mbgl/map/map_impl.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/storage/file_source.hpp>
#include <mbgl/style/style_impl.hpp>
#include <mbgl/util/exception.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/traits.hpp>
#include <mbgl/util/action_journal.hpp>
#include <mbgl/util/action_journal_impl.hpp>
#include <mbgl/gfx/rendering_stats.hpp>

namespace mbgl {

#if !defined(NDEBUG)
namespace {
void logStyleDependencies(EventSeverity severity, Event event, const style::Style& style) {
    using Dependency = style::expression::Dependency;
    constexpr auto maskCount = underlying_type(Dependency::MaskCount);
    std::array<std::size_t, maskCount + 1> counts = {0};
    const auto layers = style.getLayers();
    for (const auto& layer : layers) {
        const auto deps = layer->getDependencies();
        if (deps == Dependency::None) {
            counts[0]++;
        } else {
            for (size_t i = 0; i < maskCount; ++i) {
                if (deps & Dependency{1u << i}) {
                    counts[i + 1]++;
                }
            }
        }
    }
    std::ostringstream ss;
    ss << "Style '" << style.getName() << "' has " << layers.size() << " layers:\n";
    ss << "  " << Dependency::None << ": " << counts[0] << "\n";
    for (size_t i = 0; i < maskCount; ++i) {
        if (counts[i + 1]) {
            ss << "  " << Dependency{1u << i} << ": " << counts[i + 1] << "\n";
        }
    }
    Log::Record(severity, event, ss.str());
}
} // namespace
#endif

Map::Impl::Impl(RendererFrontend& frontend_,
                MapObserver& observer_,
                std::shared_ptr<FileSource> fileSource_,
                const MapOptions& mapOptions)
    : observer(observer_),
      rendererFrontend(frontend_),
      transform(std::make_unique<Transform>(*this, mapOptions.constrainMode(), mapOptions.viewportMode())),
      mode(mapOptions.mapMode()),
      pixelRatio(mapOptions.pixelRatio()),
      crossSourceCollisions(mapOptions.crossSourceCollisions()),
      fileSource(std::move(fileSource_)),
      style(std::make_unique<style::Style>(fileSource, pixelRatio, frontend_.getThreadPool())),
      annotationManager(*style) {
    transform->setNorthOrientation(mapOptions.northOrientation());
    style->impl->setObserver(this);
    rendererFrontend.setObserver(*this);
    transform->resize(mapOptions.size());
}

Map::Impl::~Impl() {
    // Explicitly reset the RendererFrontend first to ensure it releases
    // All shared resources (AnnotationManager)
    rendererFrontend.reset();
};

void Map::Impl::onCameraWillChange(MapObserver::CameraChangeMode cameraMode) {
    observer.onCameraWillChange(cameraMode);

    if (actionJournal) {
        actionJournal->impl->onCameraWillChange(cameraMode);
    }
}

void Map::Impl::onCameraIsChanging() {
    observer.onCameraIsChanging();

    if (actionJournal) {
        actionJournal->impl->onCameraIsChanging();
    }
}

void Map::Impl::onCameraDidChange(MapObserver::CameraChangeMode cameraMode) {
    observer.onCameraDidChange(cameraMode);

    if (actionJournal) {
        actionJournal->impl->onCameraDidChange(cameraMode);
    }
}

// MARK: - Map::Impl StyleObserver

void Map::Impl::onSourceChanged(style::Source& source) {
    observer.onSourceChanged(source);

    if (actionJournal) {
        actionJournal->impl->onSourceChanged(source);
    }
}

void Map::Impl::onUpdate() {
    // Don't load/render anything in still mode until explicitly requested.
    if (mode != MapMode::Continuous && !stillImageRequest) {
        return;
    }

    TimePoint timePoint = mode == MapMode::Continuous ? Clock::now() : Clock::time_point::max();

    transform->updateTransitions(timePoint);

    UpdateParameters params = {style->impl->isLoaded(),
                               mode,
                               pixelRatio,
                               debugOptions,
                               timePoint,
                               transform->getState(),
                               style->impl->getGlyphURL(),
                               style->impl->getFontFaces(),
                               style->impl->areSpritesLoaded(),
                               style->impl->getTransitionOptions(),
                               style->impl->getLight()->impl,
                               style->impl->getImageImpls(),
                               style->impl->getSourceImpls(),
                               style->impl->getLayerImpls(),
                               annotationManager.makeWeakPtr(),
                               fileSource,
                               prefetchZoomDelta,
                               bool(stillImageRequest),
                               crossSourceCollisions,
                               tileLodMinRadius,
                               tileLodScale,
                               tileLodPitchThreshold,
                               tileLodZoomShift};

    rendererFrontend.update(std::make_shared<UpdateParameters>(std::move(params)));
}

void Map::Impl::onStyleLoading() {
    loading = true;
    rendererFullyLoaded = false;
    observer.onWillStartLoadingMap();

    if (actionJournal) {
        actionJournal->impl->onWillStartLoadingMap();
    }
}

void Map::Impl::onStyleLoaded() {
    if (!cameraMutated) {
        jumpTo(style->getDefaultCamera());
    }
    if (LayerManager::annotationsEnabled) {
        annotationManager.onStyleLoaded();
    }

    observer.onDidFinishLoadingStyle();

    if (actionJournal) {
        actionJournal->impl->onDidFinishLoadingStyle();
    }

#if !defined(NDEBUG)
    logStyleDependencies(EventSeverity::Info, Event::Style, *style);
#endif
}

void Map::Impl::onStyleError(std::exception_ptr error) {
    MapLoadError type;
    std::string description;

    try {
        std::rethrow_exception(error);
    } catch (const mbgl::util::StyleParseException& e) {
        type = MapLoadError::StyleParseError;
        description = e.what();
    } catch (const mbgl::util::StyleLoadException& e) {
        type = MapLoadError::StyleLoadError;
        description = e.what();
    } catch (const mbgl::util::NotFoundException& e) {
        type = MapLoadError::NotFoundError;
        description = e.what();
    } catch (const std::exception& e) {
        type = MapLoadError::UnknownError;
        description = e.what();
    }

    observer.onDidFailLoadingMap(type, description);

    if (actionJournal) {
        actionJournal->impl->onDidFailLoadingMap(type, description);
    }
}

void Map::Impl::onSpriteLoaded(const std::optional<style::Sprite>& sprite) {
    observer.onSpriteLoaded(sprite);

    if (actionJournal) {
        actionJournal->impl->onSpriteLoaded(sprite);
    }
}

void Map::Impl::onSpriteError(const std::optional<style::Sprite>& sprite, std::exception_ptr ex) {
    observer.onSpriteError(sprite, ex);

    if (actionJournal) {
        actionJournal->impl->onSpriteError(sprite, ex);
    }
}

void Map::Impl::onSpriteRequested(const std::optional<style::Sprite>& sprite) {
    observer.onSpriteRequested(sprite);

    if (actionJournal) {
        actionJournal->impl->onSpriteRequested(sprite);
    }
}

// MARK: - Map::Impl RendererObserver

void Map::Impl::onInvalidate() {
    onUpdate();
}

void Map::Impl::onResourceError(std::exception_ptr error) {
    if (mode != MapMode::Continuous && stillImageRequest) {
        auto request = std::move(stillImageRequest);
        request->callback(error);
    }
}

void Map::Impl::onWillStartRenderingFrame() {
    if (mode == MapMode::Continuous) {
        observer.onWillStartRenderingFrame();

        if (actionJournal) {
            actionJournal->impl->onWillStartRenderingFrame();
        }
    }
}

void Map::Impl::onDidFinishRenderingFrame(RenderMode renderMode,
                                          bool needsRepaint,
                                          bool placemenChanged,
                                          const gfx::RenderingStats& stats) {
    rendererFullyLoaded = renderMode == RenderMode::Full;

    if (renderingStatsView && style) {
        renderingStatsView->update(*style, stats);
    }

    if (mode == MapMode::Continuous) {
        const MapObserver::RenderFrameStatus frameStatus{
            MapObserver::RenderMode(renderMode), needsRepaint, placemenChanged, stats};
        observer.onDidFinishRenderingFrame(frameStatus);

        if (actionJournal) {
            actionJournal->impl->onDidFinishRenderingFrame(frameStatus);
        }

        if (needsRepaint || transform->inTransition()) {
            onUpdate();
        } else if (rendererFullyLoaded) {
            observer.onDidBecomeIdle();

            if (actionJournal) {
                actionJournal->impl->onDidBecomeIdle();
            }
        }
    } else if (stillImageRequest && rendererFullyLoaded) {
        auto request = std::move(stillImageRequest);
        request->callback(nullptr);
    }
}

void Map::Impl::onWillStartRenderingMap() {
    if (mode == MapMode::Continuous) {
        observer.onWillStartRenderingMap();

        if (actionJournal) {
            actionJournal->impl->onWillStartRenderingMap();
        }
    }
}

void Map::Impl::onDidFinishRenderingMap() {
    if (mode == MapMode::Continuous && loading) {
        observer.onDidFinishRenderingMap(MapObserver::RenderMode::Full);

        if (actionJournal) {
            actionJournal->impl->onDidFinishRenderingMap(MapObserver::RenderMode::Full);
        }

        if (loading) {
            loading = false;
            observer.onDidFinishLoadingMap();

            if (actionJournal) {
                actionJournal->impl->onDidFinishLoadingMap();
            }
        }
    }
};

void Map::Impl::jumpTo(const CameraOptions& camera) {
    cameraMutated = true;
    transform->jumpTo(camera);
    onUpdate();
}

bool Map::Impl::isRenderingStatsViewEnabled() const {
    return !!renderingStatsView;
}

void Map::Impl::enableRenderingStatsView(bool value) {
    if (value) {
        if (!renderingStatsView) {
            renderingStatsView = std::make_unique<gfx::RenderingStatsView>();
            if (style) {
                renderingStatsView->create(*style);
            }
        }
    } else {
        if (renderingStatsView) {
            if (style) {
                renderingStatsView->destroy(*style);
            }
            renderingStatsView = nullptr;
        }
    }
}

void Map::Impl::onStyleImageMissing(const std::string& id, const std::function<void()>& done) {
    if (!style->getImage(id)) {
        observer.onStyleImageMissing(id);

        if (actionJournal) {
            actionJournal->impl->onStyleImageMissing(id);
        }
    }

    done();
    onUpdate();
}

void Map::Impl::onRemoveUnusedStyleImages(const std::vector<std::string>& unusedImageIDs) {
    for (const auto& unusedImageID : unusedImageIDs) {
        if (observer.onCanRemoveUnusedStyleImage(unusedImageID)) {
            style->removeImage(unusedImageID);
        }
    }
}

void Map::Impl::onRegisterShaders(gfx::ShaderRegistry& registry) {
    observer.onRegisterShaders(registry);

    if (actionJournal) {
        actionJournal->impl->onRegisterShaders(registry);
    }
}

void Map::Impl::onPreCompileShader(shaders::BuiltIn shaderID,
                                   gfx::Backend::Type type,
                                   const std::string& additionalDefines) {
    observer.onPreCompileShader(shaderID, type, additionalDefines);

    if (actionJournal) {
        actionJournal->impl->onPreCompileShader(shaderID, type, additionalDefines);
    }
}

void Map::Impl::onPostCompileShader(shaders::BuiltIn shaderID,
                                    gfx::Backend::Type type,
                                    const std::string& additionalDefines) {
    observer.onPostCompileShader(shaderID, type, additionalDefines);

    if (actionJournal) {
        actionJournal->impl->onPostCompileShader(shaderID, type, additionalDefines);
    }
}

void Map::Impl::onShaderCompileFailed(shaders::BuiltIn shaderID,
                                      gfx::Backend::Type type,
                                      const std::string& additionalDefines) {
    observer.onShaderCompileFailed(shaderID, type, additionalDefines);

    if (actionJournal) {
        actionJournal->impl->onShaderCompileFailed(shaderID, type, additionalDefines);
    }
}

void Map::Impl::onGlyphsLoaded(const FontStack& fontStack, const GlyphRange& ranges) {
    observer.onGlyphsLoaded(fontStack, ranges);

    if (actionJournal) {
        actionJournal->impl->onGlyphsLoaded(fontStack, ranges);
    }
}

void Map::Impl::onGlyphsError(const FontStack& fontStack, const GlyphRange& ranges, std::exception_ptr ex) {
    observer.onGlyphsError(fontStack, ranges, ex);

    if (actionJournal) {
        actionJournal->impl->onGlyphsError(fontStack, ranges, ex);
    }
}

void Map::Impl::onGlyphsRequested(const FontStack& fontStack, const GlyphRange& ranges) {
    observer.onGlyphsRequested(fontStack, ranges);

    if (actionJournal) {
        actionJournal->impl->onGlyphsRequested(fontStack, ranges);
    }
}

void Map::Impl::onTileAction(TileOperation op, const OverscaledTileID& id, const std::string& sourceID) {
    observer.onTileAction(op, id, sourceID);

    if (actionJournal) {
        actionJournal->impl->onTileAction(op, id, sourceID);
    }
}

} // namespace mbgl
