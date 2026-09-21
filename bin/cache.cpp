#include <mln/storage/file_source_manager.hpp>
#include <mln/storage/resource.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/util/chrono.hpp>
#include <mln/util/run_loop.hpp>

#include <args.hxx>
#include <mapbox/io/io.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

int main(int argc, char* argv[]) {
    args::ArgumentParser p("MapLibre cache tool", "");
    args::HelpFlag helpFlag(p, "help", "Display this help menu", {'h'});

    args::ValueFlag<std::string> urlValue(p, "URL", "Resource URL (required)", {'u'}, args::Options::Required);
    args::ValueFlag<std::string> cacheValue(
        p, "cache", "Path to the cache database (required)", {'c'}, args::Options::Required);
    args::ValueFlag<std::string> dataValue(
        p, "data", "Path to the resource data (required)", {'d'}, args::Options::Required);
    args::ValueFlag<std::string> etagValue(p, "etag", "Cache eTag, none otherwise", {'t'});
    args::ValueFlag<unsigned long> expiresValue(p, "expires", "Expires date, will use 'now' otherwise", {'e'});
    args::ValueFlag<unsigned long> modifiedValue(p, "modified", "Modified date, will use 'now' otherwise", {'m'});

    std::unordered_map<std::string, mln::Resource::Kind> typeMap{{"glyphs", mln::Resource::Glyphs},
                                                                 {"image", mln::Resource::Image},
                                                                 {"source", mln::Resource::Source},
                                                                 {"sprite-image", mln::Resource::SpriteImage},
                                                                 {"sprite-json", mln::Resource::SpriteJSON},
                                                                 {"style", mln::Resource::Style},
                                                                 {"tile", mln::Resource::Tile}};

    std::string typeHelp("One of the following (required):");
    for (const auto& key : typeMap) {
        typeHelp += " " + key.first;
    }

    args::MapFlag<std::string, mln::Resource::Kind> typeFlag(
        p, "type", typeHelp, {"type"}, typeMap, args::Options::Required);

    args::Group tileIdGroup(p, "Coordinates (required for 'tile')", args::Group::Validators::AllOrNone);
    args::ValueFlag<int32_t> xValueFlag(tileIdGroup, "x", "Tile x coordinate", {'x'});
    args::ValueFlag<int32_t> yValueFlag(tileIdGroup, "y", "Tile y coordinate", {'y'});
    args::ValueFlag<int32_t> zValueFlag(tileIdGroup, "z", "The zoom level", {'z'});

    try {
        p.ParseCLI(argc, argv);
    } catch (const args::Help&) {
        std::cout << p;
        exit(0);
    } catch (const args::ParseError& e) {
        std::cerr << e.what() << '\n';
        std::cerr << p;
        exit(1);
    } catch (const args::ValidationError& e) {
        std::cerr << e.what() << '\n';
        std::cerr << p;
        exit(2);
    }

    auto file = mapbox::base::io::readFile(args::get(dataValue));
    if (!file) {
        std::cerr << file.error() << '\n';
        exit(3);
    }

    mln::Response response;
    response.data = std::make_shared<std::string>(*file);
    response.etag = etagValue ? args::get(etagValue) : std::string();
    response.expires = expiresValue ? mln::Timestamp(mln::Seconds(args::get(expiresValue))) : mln::util::now();
    response.modified = modifiedValue ? mln::Timestamp(mln::Seconds(args::get(modifiedValue))) : mln::util::now();

    mln::Resource resource(args::get(typeFlag), args::get(urlValue));

    if (args::get(typeFlag) == mln::Resource::Tile) {
        if (!xValueFlag || !yValueFlag || !zValueFlag) {
            std::cerr << "Error: -x, -y and -z must be provided for tiles" << '\n';
            exit(1);
        }

        resource.tileData = {{.urlTemplate = args::get(urlValue),
                              .pixelRatio = 1,
                              .x = args::get(xValueFlag),
                              .y = args::get(yValueFlag),
                              .z = static_cast<int8_t>(args::get(zValueFlag))}};
    }

    mln::util::RunLoop loop;
    std::shared_ptr<mln::FileSource> dbfs = mln::FileSourceManager::get()->getFileSource(
        mln::FileSourceType::Database, mln::ResourceOptions().withCachePath(args::get(cacheValue)));
    dbfs->forward(resource, response, [&loop] { loop.stop(); });
    loop.run();
    return 0;
}
