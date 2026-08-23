#pragma once
#include <mln/style/layer_impl.hpp>

#include <set>
#include <string>

namespace mln {

namespace style {

/**
 * @brief An interface, wrapping evaluated layer properties.
 *
 *  It is an abstract base class; concrete derived classes that hold the actual
 * data are provided for each layer type.
 */
class LayerProperties {
public:
    virtual ~LayerProperties() = default;
    /// Returns constants mask for the data-driven properties.
    virtual unsigned long constantsMask() const { return 0u; }

    Immutable<Layer::Impl> baseImpl;
    /// Contains render passes used by the renderer, see `mln::RenderPass`.
    uint8_t renderPasses = 0u;

    virtual expression::Dependency getDependencies() const noexcept = 0;

    /// Returns the combined dependencies of the expressions retained in the
    /// evaluated properties (i.e. those that still require per-feature
    /// evaluation, such as data-driven paint properties).
    virtual expression::Dependency getEvaluatedDependencies() const noexcept { return expression::Dependency::None; }

    /// Collect the names of the global-state properties referenced by the
    /// expressions retained in the evaluated properties.
    virtual void collectEvaluatedGlobalStateRefs(std::set<std::string>&) const {}

protected:
    LayerProperties(Immutable<Layer::Impl> impl) noexcept
        : baseImpl(std::move(impl)) {}
};

template <class Derived>
inline const auto& getEvaluated(const Immutable<LayerProperties>& properties) noexcept {
    return static_cast<const Derived&>(*properties).evaluated;
}

template <class Derived>
inline const auto& getCrossfade(const Immutable<LayerProperties>& properties) noexcept {
    return static_cast<const Derived&>(*properties).crossfade;
}

} // namespace style
} // namespace mln
