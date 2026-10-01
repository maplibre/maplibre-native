#pragma once

#include <mln/annotation/shape_annotation_impl.hpp>
#include <mln/annotation/annotation.hpp>

namespace mln {

class FillAnnotationImpl : public ShapeAnnotationImpl {
public:
    FillAnnotationImpl(AnnotationID, FillAnnotation);

    void updateStyle(style::Style::Impl&) const final;
    const ShapeAnnotationGeometry& geometry() const final;

private:
    const FillAnnotation annotation;
};

} // namespace mln
