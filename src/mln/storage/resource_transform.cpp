#include <mln/storage/resource_transform.hpp>

namespace mln {

ResourceTransform::ResourceTransform(TransformCallback callback)
    : transformCallback(std::move(callback)) {}

void ResourceTransform::transform(Resource::Kind kind, const std::string& url, FinishedCallback finished) {
    assert(finished);
    assert(transformCallback);
    transformCallback(kind, url, std::move(finished));
}

} // namespace mln
