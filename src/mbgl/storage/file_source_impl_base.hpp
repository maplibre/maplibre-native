#include <mbgl/storage/file_source.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/util/optional.hpp>
#include <mutex>

namespace mbgl {

class FileSourceImplBase {
public:
    mutable std::mutex resourceOptionsMutex;
    ResourceOptions resourceOptions;

    virtual void setResourceOptions(ResourceOptions options) {
        std::lock_guard<std::mutex> lock(resourceOptionsMutex);
        resourceOptions = options;
    }

    virtual ResourceOptions& getResourceOptions() {
        std::lock_guard<std::mutex> lock(resourceOptionsMutex);
        return resourceOptions;
    }

    explicit FileSourceImplBase(const ResourceOptions& options);
};

}