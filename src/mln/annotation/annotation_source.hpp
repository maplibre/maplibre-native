#pragma once

#include <mln/style/source.hpp>
#include <mln/style/source_impl.hpp>

namespace mln {

class AnnotationSource final : public style::Source {
public:
    AnnotationSource();

    class Impl;
    const Impl& impl() const;

protected:
    Mutable<Source::Impl> createMutable() const noexcept final;

private:
    void loadDescription(FileSource&) final;
    bool supportsLayerType(const mln::style::LayerTypeInfo*) const override;
};

class AnnotationSource::Impl : public style::Source::Impl {
public:
    Impl();

    std::optional<std::string> getAttribution() const final;
};

} // namespace mln
