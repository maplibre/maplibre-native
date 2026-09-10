#pragma once

#include <mln/style/layer.hpp>

#include <vector>
#include <map>
#include <mutex>

namespace mln {
namespace style {
class LayerProperties;
} // namespace style
class GeometryTileLayer;
class LayerFactory;
class RenderLayer;
class Bucket;
class BucketParameters;
class Layout;
class LayoutParameters;

/**
 * @brief A singleton class responsible for creating layer instances.
 *
 * The LayerManager has implementation per platform. The LayerManager implementation
 * defines what layer types are available and it can also disable annotations.
 *
 * Linker excludes the unreachable code for the disabled annotations and layers
 * from the binaries, significantly reducing their size.
 */
class LayerManager {
public:
    /**
     * @brief A singleton getter.
     *
     * @return LayerManager*
     */
    static LayerManager* get() noexcept;

#if MLN_WITH_PLUGINS
    /**
     * Registers runtime layer factories as one atomic operation. The manager
     * takes ownership, including on failure. Existing types cannot be replaced.
     * Factories and their immutable LayerTypeInfo must remain valid for the
     * manager's lifetime. Call before loading styles that use these types.
     * Registration and lookups are thread-safe; callbacks run without the lock.
     * This is a C++ API for code built against the same core, not a DSO ABI.
     */
    bool registerLayerFactories(std::vector<std::unique_ptr<LayerFactory>>, std::string& error);
    bool registerLayerFactory(std::unique_ptr<LayerFactory>, std::string& error);
    bool hasLayerType(const std::string&) noexcept;
#endif

    /// Returns a new Layer instance on success call; returns `nullptr` otherwise.
    std::unique_ptr<style::Layer> createLayer(const std::string& type,
                                              const std::string& id,
                                              const style::conversion::Convertible& value,
                                              style::conversion::Error& error) noexcept;
    /// Returns a new RenderLayer instance on success call; returns `nullptr` otherwise.
    std::unique_ptr<RenderLayer> createRenderLayer(Immutable<style::Layer::Impl>) noexcept;
    /// Returns a new Bucket instance on success call; returns `nullptr` otherwise.
    std::unique_ptr<Bucket> createBucket(const BucketParameters&,
                                         const std::vector<Immutable<style::LayerProperties>>&);
    /// Returns a new Layout instance on success call; returns `nullptr` otherwise.
    std::unique_ptr<Layout> createLayout(const LayoutParameters&,
                                         std::unique_ptr<GeometryTileLayer>,
                                         const std::vector<Immutable<style::LayerProperties>>&);

    /**
     * @brief a build-time flag to enable/disable annotations in
     * mapbox-gl-native core.
     *
     * At the moment, the annotations implementation in core is creating
     * concrete layer instances apart from LayerManager/LayerFactory code path.
     *
     * So, annotations must be disabled if the LayerManager implementation does
     * not provide line, fill or symbol layers (those, used by the annotations
     * implementation).
     *
     * Note: in future, annotations implementation will be moved from the core
     * to platform SDK (see
     * https://github.com/maplibre/maplibre-plugins-android)
     * and this flag won't be needed any more.
     */
    static const bool annotationsEnabled;

    /**
     * Enables a layer type for JSON style only.
     *
     * We might not want to expose runtime API for some layer types
     * in order to save binary size (the corresponding SDK layer wrappers
     * should be excluded from the project build).
     */
    virtual void addLayerTypeCoreOnly(std::unique_ptr<mln::LayerFactory>);

protected:
    virtual ~LayerManager();
    virtual LayerFactory* getFactory(const std::string& type) noexcept = 0;
    virtual LayerFactory* getFactory(const style::LayerTypeInfo*) noexcept = 0;

private:
    LayerFactory* findFactory(const std::string&) noexcept;
    LayerFactory* findFactory(const style::LayerTypeInfo*) noexcept;
#if MLN_WITH_PLUGINS
    std::mutex runtimeMutex;
    std::map<std::string, std::unique_ptr<LayerFactory>> runtimeFactories;
#endif
};

} // namespace mln
