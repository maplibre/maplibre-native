#include <sstream>
#include <map>

#include <mbgl/platform/settings.hpp>
#include <mbgl/storage/mbtiles_file_source.hpp>
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

#include <mbgl/storage/sqlite3.hpp>

#include <sys/types.h>
#include <sys/stat.h>

#if defined(__QT__) && (defined(_WIN32) || defined(__EMSCRIPTEN__))
#include <QtZlib/zlib.h>
#else
#include <zlib.h>
#endif

namespace {
bool acceptsURL(const std::string &url) {
    return 0 == url.rfind(mbgl::util::MBTILES_PROTOCOL, 0);
}

std::string url_to_path(const std::string &url) {
    return mbgl::util::percentDecode(url.substr(std::char_traits<char>::length(mbgl::util::MBTILES_PROTOCOL)));
}
} // namespace

namespace mbgl {
using namespace rapidjson;

class MBTilesFileSource::Impl {
public:
    explicit Impl(const ActorRef<Impl> &, const ResourceOptions &resourceOptions_, const ClientOptions &clientOptions_)
        : resourceOptions(resourceOptions_.clone()),
          clientOptions(clientOptions_.clone()) {}

    std::vector<double> &split(const std::string &s, char delim, std::vector<double> &elems) {
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            double value = atof(item.c_str());
            elems.push_back(value);
        }
        return elems;
    }

    std::vector<double> split(const std::string &s, char delim) {
        std::vector<double> elems;
        split(s, delim, elems);
        return elems;
    }

    std::string serialize(Document &doc) {
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        doc.Accept(writer);

        return std::string(buffer.GetString(), buffer.GetSize());
    }

    std::string db_path(const std::string &path) { return path.substr(0, path.find('?')); }

    bool is_compressed(const std::string &v) { return (((uint8_t)v[0]) == 0x1f) && (((uint8_t)v[1]) == 0x8b); }

    // Generate a tilejson resource from .mbtiles file
    void request_tilejson(const Resource &resource, ActorRef<FileSourceRequest> req) {
        auto path = url_to_path(resource.url);

        Response response;

        Document doc;
        auto &allocator = doc.GetAllocator();

        std::map<std::string, std::string> values;
        auto db = mapbox::sqlite::Database::open(path, mapbox::sqlite::ReadOnly);

        mapbox::sqlite::Statement meta(db, "SELECT * from metadata");
        for (mapbox::sqlite::Query q(meta); q.run();) {
            auto name = q.get<std::string>(0);
            auto val = q.get<std::string>(1);
            if (name == "json") {
                doc.Parse(val.c_str());
            } else {
                values[name] = val;
            }
        }

        if (!doc.IsObject()) {
            doc.SetObject();
        }

        values["tilejson"] = "2.0.0";
        values["scheme"] = "xyz";

        auto format_ptr = values.find("format");
        std::string format = format_ptr == values.end() ? "png" : format_ptr->second;

        if (format != "pbf" && values.count("scale") == 0) {
            values["scale"] = "1";
        }

        // We use file location with appended parameter query parameter as URL for actual tile data
        std::string tile_url = std::string(resource.url + "?file={x}/{y}/{z}." + format);
        rapidjson::Value arr(kArrayType);
        rapidjson::Value tile_url_val(tile_url, allocator);
        arr.PushBack(tile_url_val, allocator);
        doc.AddMember("tiles", arr, allocator);

        std::string minz;
        std::string maxz;

        auto minzoom_ptr = values.find("minzoom");
        auto maxzoom_ptr = values.find("maxzoom");

        if (minzoom_ptr == values.end() || maxzoom_ptr == values.end()) {
            mapbox::sqlite::Statement zoom_stmt(db, "SELECT MIN(zoom_level),MAX(zoom_level) from tiles");

            for (mapbox::sqlite::Query q(zoom_stmt); q.run();) {
                minz = q.get<std::string>(0);
                maxz = q.get<std::string>(1);
            }

            values["minzoom"] = minz;
            values["maxzoom"] = maxz;
        } else {
            minz = minzoom_ptr->second;
            maxz = maxzoom_ptr->second;
        }

        auto minZoom = std::stoi(minz);
        auto maxZoom = std::stoi(maxz);

        for (auto const &entry : values) {
            if (entry.first == "scale") {
                doc.AddMember("scale", std::stod(entry.second), allocator);
            } else if (entry.first == "minzoom" || entry.first == "maxzoom") {
                auto name = rapidjson::Value(entry.first, allocator);
                doc.AddMember(name, std::stoi(entry.second), allocator);
            } else if (entry.first == "bounds") {
                std::vector<double> x = split(entry.second, ',');
                if (x.size() != 4) {
                    response.error = std::make_unique<Response::Error>(Response::Error::Reason::Other);
                    req.invoke(&FileSourceRequest::setResponse, response);
                    return;
                }
                double cLon = (x[0] + x[2]) / 2;
                double cLat = (x[1] + x[3]) / 2;
                int cZoom = (minZoom + maxZoom) / 2;
                rapidjson::Value bounds_arr(kArrayType);
                for (auto vv : x) {
                    bounds_arr.PushBack(vv, allocator);
                }

                if (format != "pbf") {
                    doc.AddMember("bounds", bounds_arr, allocator);
                }

                rapidjson::Value center_arr(kArrayType);
                center_arr.PushBack(cLon, allocator);
                center_arr.PushBack(cLat, allocator);
                center_arr.PushBack(cZoom, allocator);
                doc.AddMember("center", center_arr, allocator);
            } else {
                rapidjson::Value name = rapidjson::Value(entry.first, allocator);
                doc.AddMember(name, rapidjson::Value(entry.second, allocator), allocator);
            }
        }

        response.data = std::make_shared<std::string>(serialize(doc));
        req.invoke(&FileSourceRequest::setResponse, response);
    }

    // Load data for specific tile
    void request_tile(const Resource &resource, ActorRef<FileSourceRequest> req) {
        std::string base_path = url_to_path(resource.url);
        std::string path = db_path(base_path);
        auto &db = get_db(path);

        int iy = resource.tileData->y;
        int iz = resource.tileData->z;

        auto x = std::to_string(resource.tileData->x);
        auto y = std::to_string((int)(pow(2, iz) - 1) - iy);
        auto z = std::to_string(iz);

        std::string sql = "SELECT tile_data FROM tiles where zoom_level = " + z + " AND tile_column = " + x +
                          " AND tile_row = " + y;
        mapbox::sqlite::Statement stmt(db, sql.c_str());

        Response response;
        response.noContent = true;

        for (mapbox::sqlite::Query q(stmt); q.run();) {
            std::optional<std::string> data = q.get<std::optional<std::string>>(0);
            if (data) {
                response.data = std::make_shared<std::string>(*data);
                response.noContent = false;
                response.expires = Timestamp::max();
                response.etag = resource.url;

                if (is_compressed(*response.data)) {
                    response.data = std::make_shared<std::string>(util::decompress(*response.data));
                }
            }
        }
        req.invoke(&FileSourceRequest::setResponse, response);
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
    std::map<std::string, mapbox::sqlite::Database> db_cache;

    void close_db(const std::string &path) {
        auto ptr = db_cache.find(path);
        if (ptr != db_cache.end()) {
            db_cache.erase(path);
        }
    }

    void close_all() { db_cache.clear(); }

    // Multiple databases open simultaneoulsy, to effectively support multiple .mbtiles maps
    mapbox::sqlite::Database &get_db(const std::string &path) {
        auto ptr = db_cache.find(path);
        if (ptr != db_cache.end()) {
            return ptr->second;
        };

        auto ptr2 = db_cache.insert(std::pair<std::string, mapbox::sqlite::Database>(
            path, mapbox::sqlite::Database::open(path, mapbox::sqlite::ReadOnly)));
        return ptr2.first->second;
    }

    mutable std::mutex resourceOptionsMutex;
    mutable std::mutex clientOptionsMutex;
    ResourceOptions resourceOptions;
    ClientOptions clientOptions;
};

MBTilesFileSource::MBTilesFileSource(const ResourceOptions &resourceOptions, const ClientOptions &clientOptions)
    : thread(std::make_unique<util::Thread<Impl>>(
          util::makeThreadPrioritySetter(platform::EXPERIMENTAL_THREAD_PRIORITY_FILE),
          "MBTilesFileSource",
          resourceOptions.clone(),
          clientOptions.clone())) {}

std::unique_ptr<AsyncRequest> MBTilesFileSource::request(const Resource &resource, FileSource::Callback callback) {
    auto req = std::make_unique<FileSourceRequest>(std::move(callback));

    // assume if there is a tile request, that the mbtiles file has been validated
    if (resource.kind == Resource::Tile) {
        thread->actor().invoke(&Impl::request_tile, resource, req->actor());
        return req;
    }

    if (resource.url.find("://") == std::string::npos ||
        !util::is_absolute_path(resource.url.substr(resource.url.find("://") + 3))) {
        Response response;
        response.noContent = true;
        response.error = std::make_unique<Response::Error>(Response::Error::Reason::Other,
                                                           "MBTilesFileSource only supports absolute path urls");
        req->actor().invoke(&FileSourceRequest::setResponse, response);
        return req;
    }

    // file must exist
    auto path = url_to_path(resource.url);
    struct stat buffer;
    int result = stat(path.c_str(), &buffer);
    if (result == -1 && errno == ENOENT) {
        Response response;
        response.noContent = true;
        response.error = std::make_unique<Response::Error>(Response::Error::Reason::NotFound,
                                                           "path not found: " + path);
        req->actor().invoke(&FileSourceRequest::setResponse, response);
        return req;
    }

    // return TileJSON
    thread->actor().invoke(&Impl::request_tilejson, resource, req->actor());
    return req;
}

bool MBTilesFileSource::canRequest(const Resource &resource) const {
    return acceptsURL(resource.url);
}

MBTilesFileSource::~MBTilesFileSource() = default;

void MBTilesFileSource::setResourceOptions(ResourceOptions options) {
    thread->actor().invoke(&Impl::setResourceOptions, options.clone());
}

ResourceOptions MBTilesFileSource::getResourceOptions() {
    return thread->actor().ask(&Impl::getResourceOptions).get();
}

void MBTilesFileSource::setClientOptions(ClientOptions options) {
    thread->actor().invoke(&Impl::setClientOptions, options.clone());
}

ClientOptions MBTilesFileSource::getClientOptions() {
    return thread->actor().ask(&Impl::getClientOptions).get();
}

} // namespace mbgl
