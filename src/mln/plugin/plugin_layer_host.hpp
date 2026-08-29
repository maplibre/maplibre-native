#pragma once

#include <mln/plugin/plugin_registry.hpp>
#include <mln/style/layer_impl.hpp>
#include <mln/util/immutable.hpp>

#include <memory>
#include <string>
#include <vector>

namespace mln {

class PaintParameters;
class FileSource;
class RendererObserver;

namespace plugin {

/**
 * Per-style-layer runtime for registered extensions.
 *
 * This host is deliberately independent of generated render layer classes. A
 * plugin is selected only by the style layer type stored in Layer::Impl.
 */
class PluginLayerHost final {
public:
    PluginLayerHost(std::string layerID, std::string layerType, Immutable<style::Layer::Impl> layerImpl);
    ~PluginLayerHost();

    PluginLayerHost(const PluginLayerHost&) = delete;
    PluginLayerHost& operator=(const PluginLayerHost&) = delete;

    bool empty() const noexcept;
    void updateLayer(Immutable<style::Layer::Impl>);
    void updateEnvironment(std::shared_ptr<FileSource>, RendererObserver*);
    void prepareFrame(PaintParameters&, const std::vector<mln_plugin_draw_packet_v1>&);
    void renderBeforeLayer(PaintParameters&, const std::vector<mln_plugin_draw_packet_v1>&);
    void contextLost();

private:
    struct Instance;
    struct PropertySnapshot;

    PropertySnapshot makePropertySnapshot(const Instance&) const;
    void invoke(Instance&, PaintParameters&, bool prepare, const std::vector<mln_plugin_draw_packet_v1>&);
    void disable(Instance&, const char* callbackName, mln_plugin_status);

    std::string layerID;
    std::string layerType;
    Immutable<style::Layer::Impl> layerImpl;
    std::vector<std::unique_ptr<Instance>> instances;
};

} // namespace plugin
} // namespace mln
