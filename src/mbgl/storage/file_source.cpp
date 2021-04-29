#include <mbgl/storage/file_source.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/util/optional.hpp>
#include <mutex>

namespace mbgl {

FileSource::FileSource() {}

FileSource::~FileSource() = default;

void FileSource::setResourceOptions(ResourceOptions options) {
    impl->setResourceOptions(std::move(options));
}

ResourceOptions& FileSource::getResourceOptions() {
    return impl->getResourceOptions();
}

}