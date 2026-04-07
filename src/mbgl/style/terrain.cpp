#include <mbgl/style/terrain.hpp>
#include <mbgl/style/terrain_impl.hpp>
#include <mbgl/style/terrain_observer.hpp>
#include <utility>

namespace mbgl {
namespace style {

namespace {
TerrainObserver nullObserver;
}

Terrain::Terrain(Immutable<Terrain::Impl> impl_)
    : impl(std::move(impl_)),
      observer(&nullObserver) {}

Terrain::Terrain()
    : Terrain(makeMutable<Impl>()) {}

Terrain::Terrain(const std::string& sourceID, float exaggeration)
    : Terrain(makeMutable<Impl>(sourceID, exaggeration)) {}

Terrain::~Terrain() = default;

void Terrain::setObserver(TerrainObserver* observer_) {
    observer = observer_ ? observer_ : &nullObserver;
}

Mutable<Terrain::Impl> Terrain::mutableImpl() const {
    return makeMutable<Impl>(*impl);
}

std::string Terrain::getSource() const {
    return impl->sourceID;
}

void Terrain::setSource(const std::string& sourceID) {
    auto impl_ = mutableImpl();
    impl_->sourceID = sourceID;
    impl = std::move(impl_);
    observer->onTerrainChanged(*this);
}

float Terrain::getExaggeration() const {
    return impl->exaggeration;
}

void Terrain::setExaggeration(float exaggeration) {
    auto impl_ = mutableImpl();
    impl_->exaggeration = exaggeration;
    impl = std::move(impl_);
    observer->onTerrainChanged(*this);
}

} // namespace style
} // namespace mbgl
