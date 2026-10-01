#pragma once

#include <mln/style/layer.hpp>
#include <mln/style/layers/custom_layer_host.hpp>

#include <memory>

namespace mln {

namespace style {

class CustomLayer final : public Layer {
public:
    CustomLayer(const std::string& id, std::unique_ptr<CustomLayerHost> host);

    CustomLayer(const CustomLayer&) = delete;
    ~CustomLayer() final;
    class Impl;
    const Impl& impl() const;
    Mutable<Impl> mutableImpl() const;

private:
    std::optional<conversion::Error> setPropertyInternal(const std::string& name,
                                                         const conversion::Convertible& value) final;
    StyleProperty getProperty(const std::string&) const final;
    std::unique_ptr<Layer> cloneRef(const std::string& id) const final;
    Mutable<Layer::Impl> mutableBaseImpl() const final;
};

} // namespace style
} // namespace mln
