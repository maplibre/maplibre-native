#pragma once

#include <mbgl/style/source_impl.hpp>
#include <mbgl/style/sources/contour_source.hpp>

namespace mbgl {
namespace style {

class ContourSource::Impl final : public Source::Impl {
public:
    Impl(std::string id, ContourSourceOptions options);
    ~Impl() final;

    const ContourSourceOptions& getOptions() const { return options; }

    std::optional<std::string> getAttribution() const final;

private:
    ContourSourceOptions options;
};

} // namespace style
} // namespace mbgl
