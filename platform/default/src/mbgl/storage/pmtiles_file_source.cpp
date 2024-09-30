#include <sstream>
#include <map>

#include <mbgl/platform/settings.hpp>
#include <mbgl/storage/pmtiles_file_source.hpp>
#include <mbgl/storage/file_source_manager.hpp>
#include <mbgl/storage/file_source_request.hpp>

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <mbgl/util/thread.hpp>
#include <mbgl/util/url.hpp>
#include <mbgl/util/chrono.hpp>
#include <mbgl/util/compression.hpp>
#include <mbgl/util/filesystem.hpp>

#include <pmtiles.hpp>

#include <sys/types.h>
#include <sys/stat.h>

#if defined(__QT__) && (defined(_WIN32) || defined(__EMSCRIPTEN__))
#include <QtZlib/zlib.h>
#else
#include <zlib.h>
#endif

#define MAX_DIRECTORY_CACHE_ENTRIES = 100;

namespace {
bool acceptsURL(const std::string& url) {
    return 0 == url.rfind(mbgl::util::PMTILES_PROTOCOL, 0);
}

std::string extract_url(const std::string& url) {
    return url.substr(std::char_traits<char>::length(mbgl::util::PMTILES_PROTOCOL));
}
} // namespace

namespace mbgl {
using namespace rapidjson;

using AsyncCallback = std::function<void(std::unique_ptr<Response::Error>)>;
using AsyncTileCallback = std::function<void(std::pair<uint64_t, uint32_t>, std::unique_ptr<Response::Error>)>;

class PMTilesFileSource::Impl {
public:
    explicit Impl(const ActorRef<Impl> &, const ResourceOptions &resourceOptions_, const ClientOptions &clientOptions_)
        : resourceOptions(resourceOptions_.clone()),
          clientOptions(clientOptions_.clone()) {}

    // Generate a tilejson resource from .pmtiles file
    void request_tilejson(AsyncRequest* req, const Resource& resource, const ActorRef<FileSourceRequest>& ref) {
        auto url = extract_url(resource.url);
        
        getMetadata(url, req, [=](std::unique_ptr<Response::Error> error) {
            Response response;

            if (error) {
                response.error = std::move(error);
                ref.invoke(&FileSourceRequest::setResponse, response);
                return;
            }

            response.data = std::make_shared<std::string>(metadata_cache.at(url));
            ref.invoke(&FileSourceRequest::setResponse, response);
        });
    }

    // Load data for specific tile
    void request_tile(AsyncRequest* req, const Resource& resource, ActorRef<FileSourceRequest> ref) {
        auto url = extract_url(resource.url);

        getHeaderAndRootDirectory(url, req, [=](std::unique_ptr<Response::Error> error) {
            Response response;
            response.noContent = true;

            if (error) {
                response.error = std::move(error);
                ref.invoke(&FileSourceRequest::setResponse, response);
                return;
            }
            
            pmtiles::headerv3 header = header_cache.at(url);

            if (resource.tileData->z < header.min_zoom || resource.tileData->z > header.max_zoom) {
                ref.invoke(&FileSourceRequest::setResponse, response);
                return;
            }

            uint64_t tileID;
            
            try {
                tileID = pmtiles::zxy_to_tileid(static_cast<uint8_t>(resource.tileData->z), static_cast<uint32_t>(resource.tileData->x), static_cast<uint32_t>(resource.tileData->y));
            }
            catch(const std::exception& e)
            {
                response.error = std::make_unique<Response::Error>(
                    Response::Error::Reason::Other,
                    std::string("Invalid tile: ") + e.what());
                ref.invoke(&FileSourceRequest::setResponse, response);
                return;
            }

            getTileAddress(url, req, tileID, header.root_dir_offset, header.root_dir_bytes, 0, [=](std::pair<uint64_t, uint32_t> tileAddress, std::unique_ptr<Response::Error> tileError) {
                Response response;
                response.noContent = true;

                if (tileError) {
                    response.error = std::move(tileError);
                    ref.invoke(&FileSourceRequest::setResponse, response);
                    return;
                }

                if (tileAddress.first == 0 && tileAddress.second == 0) {
                    ref.invoke(&FileSourceRequest::setResponse, response);
                    return;
                }
                
                Resource tileResource(Resource::Kind::Source, url);
                tileResource.loadingMethod = Resource::LoadingMethod::Network;
                tileResource.dataRange = std::make_pair(tileAddress.first, tileAddress.first + tileAddress.second - 1);

                tasks[req] = getFileSource()->request(tileResource, [=](const Response& tileResponse) {
                    Response response;
                    response.noContent = true;

                    if (tileResponse.error) {
                        response.error = std::make_unique<Response::Error>(
                            tileResponse.error->reason,
                            std::string("Error fetching PMTiles tile: ") + tileResponse.error->message);
                        ref.invoke(&FileSourceRequest::setResponse, response);
                        return;
                    }

                    response.data = tileResponse.data;
                    response.noContent = false;
                    response.modified = tileResponse.modified;
                    response.expires = tileResponse.expires;
                    response.etag = tileResponse.etag;

                    if (header.tile_compression == pmtiles::COMPRESSION_GZIP) {
                        response.data = std::make_shared<std::string>(util::decompress(*tileResponse.data));
                    }

                    ref.invoke(&FileSourceRequest::setResponse, response);
                    return;
                });
            });
        });
    }

    void setResourceOptions(ResourceOptions options) {
        std::lock_guard<std::mutex> lock(resourceOptionsMutex);
        resourceOptions = options;
    }

    ResourceOptions getResourceOptions() {
        std::lock_guard<std::mutex> lock(resourceOptionsMutex);
        return resourceOptions.clone();
    }

    void setClientOptions(ClientOptions options) {
        std::lock_guard<std::mutex> lock(clientOptionsMutex);
        clientOptions = options;
    }

    ClientOptions getClientOptions() {
        std::lock_guard<std::mutex> lock(clientOptionsMutex);
        return clientOptions.clone();
    }

private:
    mutable std::mutex resourceOptionsMutex;
    mutable std::mutex clientOptionsMutex;
    ResourceOptions resourceOptions;
    ClientOptions clientOptions;

    std::shared_ptr<FileSource> fileSource;
    std::map<std::string, pmtiles::headerv3> header_cache;
    std::map<std::string, std::string> metadata_cache;
    std::map<std::string, std::map<std::string, std::vector<pmtiles::entryv3>>> directory_cache;
    std::map<std::string, std::vector<std::string>> directory_cache_control;
    std::map<AsyncRequest*, std::unique_ptr<AsyncRequest>> tasks;

    std::shared_ptr<FileSource> getFileSource() {
        if (!fileSource) {
            fileSource = FileSourceManager::get()->getFileSource(FileSourceType::ResourceLoader, resourceOptions, clientOptions);
        }
        
        return fileSource;
    }

    void getHeaderAndRootDirectory(const std::string& url, AsyncRequest* req, AsyncCallback callback) {
        if (header_cache.find(url) != header_cache.end()) {
            callback(std::unique_ptr<Response::Error>());
        }

        Resource resource(Resource::Kind::Source, url);
        resource.loadingMethod = Resource::LoadingMethod::Network;
        resource.dataRange = std::make_pair<uint64_t, uint64_t>(0, 16383);

        tasks[req] = getFileSource()->request(resource, [=](const Response& response) {
            if (response.error) {
                callback(std::make_unique<Response::Error>(
                    response.error->reason,
                    std::string("Error fetching PMTiles header and root directory: ") + response.error->message));
                
                return;
            }
            
            try {
                pmtiles::headerv3 header = pmtiles::deserialize_header(response.data->substr(0, 127));
                
                if (header.tile_compression != pmtiles::COMPRESSION_NONE && header.tile_compression != pmtiles::COMPRESSION_GZIP) {
                    throw std::runtime_error("Compression method not supported");
                }
                
                header_cache.emplace(url, header);

                std::string directoryData = response.data->substr(header.root_dir_offset, header.root_dir_bytes);

                if (header.tile_compression == pmtiles::COMPRESSION_GZIP) {
                    directoryData = util::decompress(directoryData);
                }
                
                storeDirectory(url, header.root_dir_offset, header.root_dir_bytes, directoryData);
                
                callback(std::unique_ptr<Response::Error>());
            } catch(const std::exception& e) {
                callback(std::make_unique<Response::Error>(
                    Response::Error::Reason::Other,
                    std::string("Error parsing PMTiles header and root directory: ") + e.what()));
            }
        });
    }

    void getMetadata(std::string& url, AsyncRequest* req, AsyncCallback callback) {
        if (metadata_cache.find(url) != metadata_cache.end()) {
            callback(std::unique_ptr<Response::Error>());
        }

        getHeaderAndRootDirectory(url, req, [=](std::unique_ptr<Response::Error> error) {
            if (error) {
                callback(std::move(error));
                return;
            }

            pmtiles::headerv3 header = header_cache.at(url);

            auto parse_callback = [=](const std::string& data) {
                Document doc;
                auto& allocator = doc.GetAllocator();

                if (!data.empty()) {
                    doc.Parse(data);
                }

                if (!doc.IsObject()) {
                    doc.SetObject();
                }

                doc.AddMember("tilejson", "3.0.0", allocator);
                doc.AddMember("scheme", "xyz", allocator);

                std::string format;

                switch (header.tile_type) {
                    case pmtiles::TILETYPE_MVT:
                        format = "pbf";
                        break;
                    case pmtiles::TILETYPE_PNG:
                        format = "png";
                        break;
                    case pmtiles::TILETYPE_JPEG:
                        format = "jpg";
                        break;
                    case pmtiles::TILETYPE_WEBP:
                        format = "webp";
                        break;
                    case pmtiles::TILETYPE_AVIF:
                        format = "avif";
                        break;
                    default:
                        break;
                }

                rapidjson::Value arr(kArrayType);
                rapidjson::Value tile_url_val(util::PMTILES_PROTOCOL + url, allocator);
                arr.PushBack(tile_url_val, allocator);
                doc.AddMember("tiles", arr, allocator);

                rapidjson::Value bounds_arr(kArrayType);
                bounds_arr.PushBack(static_cast<double>(header.min_lon_e7) / 1e7, allocator);
                bounds_arr.PushBack(static_cast<double>(header.min_lat_e7) / 1e7, allocator);
                bounds_arr.PushBack(static_cast<double>(header.max_lon_e7) / 1e7, allocator);
                bounds_arr.PushBack(static_cast<double>(header.max_lat_e7) / 1e7, allocator);

                doc.AddMember("bounds", bounds_arr, allocator);

                rapidjson::Value center_arr(kArrayType);
                center_arr.PushBack(static_cast<double>(header.center_lon_e7) / 1e7, allocator);
                center_arr.PushBack(static_cast<double>(header.center_lat_e7) / 1e7, allocator);
                center_arr.PushBack(header.center_zoom, allocator);
                doc.AddMember("center", center_arr, allocator);

                doc.AddMember("minzoom", header.min_zoom, allocator);
                doc.AddMember("maxzoom", header.max_zoom, allocator);

                std::string metadata = serialize(doc);
                metadata_cache.emplace(url, metadata);

                callback(std::unique_ptr<Response::Error>());
            };

            if (header.json_metadata_bytes > 0) {
                Resource resource(Resource::Kind::Source, url);
                resource.loadingMethod = Resource::LoadingMethod::Network;
                resource.dataRange = std::make_pair(header.json_metadata_offset, header.json_metadata_offset + header.json_metadata_bytes - 1);

                tasks[req] = getFileSource()->request(resource, [=](const Response& responseMetadata) {
                    if (responseMetadata.error) {
                        callback(std::make_unique<Response::Error>(
                            responseMetadata.error->reason,
                            std::string("Error fetching PMTiles metadata: ") + responseMetadata.error->message));

                        return;
                    }

                    std::string data = *responseMetadata.data;

                    if (header.internal_compression == pmtiles::COMPRESSION_GZIP) {
                        data = util::decompress(data);
                    }
                    
                    parse_callback(data);
                });

                return;
            }

            parse_callback(std::string());
        });
    }

    void storeDirectory(const std::string& url, uint64_t directoryOffset, uint64_t directoryLength, const std::string& directoryData) {
        if (directory_cache.find(url) == directory_cache.end()) {
            directory_cache.emplace(url, std::map<std::string, std::vector<pmtiles::entryv3>>());
            directory_cache_control.emplace(url, std::vector<std::string>());
        }

        std::string directory_cache_key = url + "|" + std::to_string(directoryOffset) + "|" + std::to_string(directoryLength);
        directory_cache.at(url).emplace(directory_cache_key, pmtiles::deserialize_directory(rootDirectoryData));
        directory_cache_control.at(url).emplace_back(directory_cache_key);

        if (directory_cache_control.at(url).size() > MAX_DIRECTORY_CACHE_ENTRIES) {
            directory_cache.at(url).erase(directory_cache_control.at(url).front());
            directory_cache_control.at(url).erase(directory_cache_control.at(url).begin());
        }
    }

    void getDirectory(const std::string& url, AsyncRequest* req, uint64_t directoryOffset, uint32_t directoryLength, AsyncCallback callback) {
        std::string directory_cache_key = url + "|" + std::to_string(directoryOffset) + "|" + std::to_string(directoryLength);

        if (directory_cache.find(url) != directory_cache.end() &&
            directory_cache.at(url).find(directory_cache_key) != directory_cache.at(url).end()) {
            
            if (directory_cache_control.at(url).back() != directory_cache_key) {
                directory_cache_control.at(url).emplace_back(directory_cache_key);

                for (auto it = directory_cache_control.at(url).begin(); it != directory_cache_control.end(); ++it) {
                    if (*it == directory_cache_key) {
                        directory_cache_control.at(url).erase(it);
                        break;
                    }
                }
            }
            
            callback(std::unique_ptr<Response::Error>());
            return;
        }
        
        getHeaderAndRootDirectory(url, req, [=](std::unique_ptr<Response::Error> error) {
            if (error) {
                callback(std::move(error));
                return;
            }

            pmtiles::headerv3 header = header_cache.at(url);

            Resource resource(Resource::Kind::Source, url);
            resource.loadingMethod = Resource::LoadingMethod::Network;
            resource.dataRange = std::make_pair(directoryOffset, directoryOffset + directoryLength - 1);

            tasks[req] = getFileSource()->request(resource, [=](const Response& response) {
                if (response.error) {
                    callback(std::make_unique<Response::Error>(
                        response.error->reason,
                        std::string("Error fetching PMTiles directory: ") + response.error->message));

                    return;
                }

                try {
                    std::string directoryData = *response.data;

                    if (header.tile_compression == pmtiles::COMPRESSION_GZIP) {
                        directoryData = util::decompress(directoryData);
                    }
                    
                    storeDirectory(url, directoryOffset, directoryLength, directoryData);

                    callback(std::unique_ptr<Response::Error>());
                } catch(const std::exception& e)
                {
                    callback(std::make_unique<Response::Error>(
                        Response::Error::Reason::Other,
                        std::string(std::string("Error parsing PMTiles directory: ") + e.what())));
                }
            });
        });
    }

    void getTileAddress(const std::string& url, AsyncRequest* req, uint64_t tileID, uint64_t directoryOffset, uint32_t directoryLength, uint32_t directoryDepth, AsyncTileCallback callback) {
        if (directoryDepth > 3) {
            callback(std::make_pair<uint64_t, uint32_t>(0, 0),
                std::make_unique<Response::Error>(Response::Error::Reason::Other,
                    std::string("Error fetching PMTiles tile address: Maximum directory depth exceeded")));
        }

        fetchDirectory(url, req, directoryOffset, directoryLength, [=](std::unique_ptr<Response::Error> error) {
            if (error) {
                callback(std::make_pair(0, 0), std::move(error));
                return;
            }

            pmtiles::headerv3 header = header_cache.at(url);
            std::vector<pmtiles::entryv3> directory = directory_cache.at(url).at(url + "|" + std::to_string(directoryOffset) + "|" + std::to_string(directoryLength));

            pmtiles::entryv3 entry = pmtiles::find_tile(directory, tileID);

            if (entry.length > 0) {
                if (entry.run_length > 0) {
                    callback(std::make_pair(header.tile_data_offset + entry.offset, entry.length), {});
                    return;
                }
                
                getTileAddress(url, req, tileID, header.leaf_dirs_offset + entry.offset, entry.length, directoryDepth + 1, std::move(callback));
                return;
            }

            callback(std::make_pair(0, 0), {});
        });
    }

    std::string serialize(Document& doc) {
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        doc.Accept(writer);

        return std::string(buffer.GetString(), buffer.GetSize());
    }
};

PMTilesFileSource::PMTilesFileSource(const ResourceOptions& resourceOptions, const ClientOptions& clientOptions)
    : thread(std::make_unique<util::Thread<Impl>>(
          util::makeThreadPrioritySetter(platform::EXPERIMENTAL_THREAD_PRIORITY_FILE),
          "PMTilesFileSource",
          resourceOptions.clone(),
          clientOptions.clone())) {}

std::unique_ptr<AsyncRequest> PMTilesFileSource::request(const Resource& resource, FileSource::Callback callback) {
    auto req = std::make_unique<FileSourceRequest>(std::move(callback));

    // assume if there is a tile request, that the pmtiles file has been validated
    if (resource.kind == Resource::Tile) {
        thread->actor().invoke(&Impl::request_tile, req.get(), resource, req->actor());
        return req;
    }

    // return TileJSON
    thread->actor().invoke(&Impl::request_tilejson, req.get(), resource, req->actor());
    return req;
}

bool PMTilesFileSource::canRequest(const Resource& resource) const {
    return acceptsURL(resource.url);
}

PMTilesFileSource::~PMTilesFileSource() = default;

void PMTilesFileSource::setResourceOptions(ResourceOptions options) {
    thread->actor().invoke(&Impl::setResourceOptions, options.clone());
}

ResourceOptions PMTilesFileSource::getResourceOptions() {
    return thread->actor().ask(&Impl::getResourceOptions).get();
}

void PMTilesFileSource::setClientOptions(ClientOptions options) {
    thread->actor().invoke(&Impl::setClientOptions, options.clone());
}

ClientOptions PMTilesFileSource::getClientOptions() {
    return thread->actor().ask(&Impl::getClientOptions).get();
}

} // namespace mbgl
