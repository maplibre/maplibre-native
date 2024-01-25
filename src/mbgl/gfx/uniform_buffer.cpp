#include <mbgl/gfx/uniform_buffer.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/uniform_metadata.hpp>

#include <map>

namespace mbgl {
namespace gfx {

std::map<std::string, size_t> gUBOUpdates;
std::size_t gStructLevelUpdatedSize{0}, gFieldLevelUpdatedSize{0};

std::shared_ptr<UniformBuffer> UniformBufferArray::nullref = nullptr;

UniformBufferArray::UniformBufferArray(UniformBufferArray&& other)
    : uniformBufferMap(std::move(other.uniformBufferMap)) {}

UniformBufferArray& UniformBufferArray::operator=(UniformBufferArray&& other) {
    uniformBufferMap = std::move(other.uniformBufferMap);
    return *this;
}

UniformBufferArray& UniformBufferArray::operator=(const UniformBufferArray& other) {
    uniformBufferMap.clear();
    for (const auto& kv : other.uniformBufferMap) {
        add(kv.first, copy(*kv.second));
    }
    return *this;
}

const std::shared_ptr<UniformBuffer>& UniformBufferArray::get(const StringIdentity id) const {
    const auto result = uniformBufferMap.find(id);
    return (result != uniformBufferMap.end()) ? result->second : nullref;
}

const std::shared_ptr<UniformBuffer>& UniformBufferArray::addOrReplace(const StringIdentity id,
                                                                       std::shared_ptr<UniformBuffer> uniformBuffer) {
    const auto result = uniformBufferMap.insert(std::make_pair(id, std::shared_ptr<UniformBuffer>()));
    result.first->second = std::move(uniformBuffer);
    return result.first->second;
}

void UniformBufferArray::createOrUpdate(const StringIdentity id,
                                        const std::vector<uint8_t>& data,
                                        gfx::Context& context,
                                        bool persistent) {
    createOrUpdate(id, data.data(), data.size(), context, persistent);
}

void UniformBufferArray::createOrUpdate(
    const StringIdentity id, const void* data, const std::size_t size, gfx::Context& context, bool persistent) {
    if (auto& ubo = get(id); ubo && ubo->getSize() == size) {
        // measure
        std::vector<std::string> fields;
        std::size_t updatedSize{0};
        uniform_metadata::getChangedUBOFields(
            stringIndexer().get(id), size, ubo->getCurrent(), static_cast<const uint8_t*>(data), fields, updatedSize);
        for (auto& field : fields) {
            gUBOUpdates[std::string(stringIndexer().get(id)) + "." + field]++;
        }
        gFieldLevelUpdatedSize += updatedSize;

        // update
        ubo->update(data, size);
    } else {
        // measure
        std::vector<std::string> fields;
        std::size_t dataSize{0};
        uniform_metadata::getUBOFields(stringIndexer().get(id), size, fields, dataSize);
        for (auto& field : fields) {
            gUBOUpdates[std::string(stringIndexer().get(id)) + "." + field]++;
        }
        gFieldLevelUpdatedSize += dataSize;

        // add
        add(id, context.createUniformBuffer(data, size, persistent));
    }

    // print
    {
        static size_t idx = 0;
        gStructLevelUpdatedSize += size;

        std::vector<std::pair<std::string, size_t>> vec;
        std::copy(gUBOUpdates.begin(), gUBOUpdates.end(), std::back_insert_iterator<decltype(vec)>(vec));
        std::sort(vec.begin(), vec.end(), [](auto a, auto b) { return a.second > b.second; });

        if (idx++ % 10 == 0) {
            printf("--- %zu fields:\n", vec.size());
            for (auto& v : vec) {
                printf("%zu %s\n", v.second, v.first.c_str());
            }
            printf("---\n");
        }
        printf("fields updates: %zu KB\n", gFieldLevelUpdatedSize / 1024);
        printf("struct updates: %zu KB\n", gStructLevelUpdatedSize / 1024);
    }
}

const std::shared_ptr<UniformBuffer>& UniformBufferArray::add(const StringIdentity id,
                                                              std::shared_ptr<UniformBuffer>&& uniformBuffer) {
    const auto result = uniformBufferMap.insert(std::make_pair(id, std::shared_ptr<UniformBuffer>()));
    if (result.second) {
        result.first->second = std::move(uniformBuffer);
        return result.first->second;
    } else {
        return nullref;
    }
}

} // namespace gfx
} // namespace mbgl
