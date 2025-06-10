#include <fstream>
#include <iostream>

#include "cxxopts.hpp"
#include "json.hpp"
#include "maprenderer.h"

namespace {

bool verbose = false;

bool isUrl(const std::string &str) {
    return str.find("://") != std::string::npos;
}

nlohmann::json parseJson(std::istream &stream) {
    nlohmann::json jsonData;
    try {
        jsonData = nlohmann::json::parse(stream);
    } catch (const nlohmann::json::parse_error &e) {
        std::cerr << "Error parsing json: " << e.what() << std::endl;
    }
    stream.clear();
    stream.seekg(0, std::ios::beg);
    return jsonData;
}

void filterSourceWithBidLvl(nlohmann::json &source, const std::string &bid, int lvl) {
    auto &features = source["features"];
    auto it = features.begin();
    while (it != features.end()) {
        auto &feature = *it;
        if (feature["properties"]["bid"] != bid || feature["properties"]["lvl"] != lvl) {
            if (verbose) {
                std::cout << "Erasing feature " << feature["properties"]["fid"] << std::endl;
            }
            it = features.erase(it);
            continue;
        }
        it++;
    }
}

void filterStyleWithBidLvl(nlohmann::json &style, const std::string &bid, int lvl) {
    auto &layers = style["layers"];
    for (auto &layer : layers) {
        auto it = layer.find("source");
        if (it == layer.end() || *it != "source_ptr") {
            continue;
        }

        it = layer.find("filter");
        if (it == layer.end()) {
            continue;
        }

        auto &filter = *it;

        // bid/lvl filters always start with "all"
        if (filter.empty() || filter[0] != "all") {
            continue;
        }

        if (verbose) {
            std::cout << "Changing filter for " << layer["id"] << std::endl;
        }
        for (int i = 1; i < filter.size(); i++) {
            if (filter[i][1][1] == "bid") {
                filter[i][2].clear();
                filter[i][2].push_back(bid);
            }
            if (filter[i][1][1] == "lvl") {
                filter[i][2] = lvl;
            }
        }
    }
}

void removeSymbolLayers(nlohmann::json &style) {
    auto &layers = style["layers"];
    auto it = layers.begin();
    while (it != layers.end()) {
        auto &layer = *it;
        if (layer["type"] == "symbol") {
            if (verbose) {
                std::cout << "Erasing layer " << layer["id"] << std::endl;
            }
            it = layers.erase(it);
            continue;
        }
        it++;
    }
}

}  // namespace

int main(int argc, char **argv) {
    std::string argv_str(argv[0]);
    std::string exeDir = argv_str.substr(0, argv_str.find_last_of("/"));

    cxxopts::Options options(
        "map-renderer", "Renders MapLibre maps to raster images. Either center/zoom or bounds should be provided.");

    options.add_options()                                                                                         //
        ("s,style", "Path to style json file", cxxopts::value<std::string>())                                     //
        ("o,output", "[Optional] Output png file path", cxxopts::value<std::string>()->default_value("map.png"))  //
        ("metadata",                                                                                              //
         "[Optional] Path to metadata json file containing extra info about the generated map image",             //
         cxxopts::value<std::string>())                                                                           //
        ("center", "Center of map <longitude,latitude> (no spaces)", cxxopts::value<std::vector<double>>())       //
        ("zoom", "Zoom level", cxxopts::value<double>()->default_value("19"))                                     //
        ("bounds",                                                                                                //
         "Map bounds <lon-west,lat-south,lon-east,lat-north> (no spaces)",                                        //
         cxxopts::value<std::vector<double>>())                                                                   //
        ("padding",                                                                                               //
         "[Optional] Padding in pixels (only applicable when bounds are provided)",                               //
         cxxopts::value<double>()->default_value("0"))                                                            //
        ("width", "[Optional] Image width", cxxopts::value<int>()->default_value("1024"))                         //
        ("height", "[Optional] Image height", cxxopts::value<int>()->default_value("1024"))                       //
        ("ratio", "[Optional] Pixel ratio", cxxopts::value<double>()->default_value("4"))                         //
        ("source",                                                                                                //
         "[Optional] Vector tiles url or path to geojson file for source_ptr",                                    //
         cxxopts::value<std::string>())                                                                           //
        ("bid", "[Optional] Building id to filter style", cxxopts::value<std::string>())                          //
        ("lvl", "[Optional] Level index to filter style", cxxopts::value<int>())                                  //
        ("symbols", "[Flag] Whether to render symbol layers")                                                     //
        ("verbose", "Verbose mode")                                                                               //
        ("h,help", "Print usage")                                                                                 //
        ;

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (!result.count("style")) {
        std::cerr << "style is a required parameter" << std::endl;
        return -1;
    }

    if (result.count("verbose")) {
        verbose = true;
    }

    // Option 1: center and zoom
    double centerLon = 0.0;
    double centerLat = 0.0;
    if (result.count("center")) {
        auto coords = result["center"].as<std::vector<double>>();
        if (coords.size() != 2) {
            std::cerr << "Center must have two coordinates in <longitude,latitude> order" << std::endl;
            return -1;
        }
        centerLon = coords[0];
        centerLat = coords[1];
    }
    double zoom = result["zoom"].as<double>();

    // Option 2: bounds
    std::vector<double> bounds;
    if (result.count("bounds")) {
        bounds = result["bounds"].as<std::vector<double>>();
        if (bounds.size() != 4) {
            std::cerr << "Bounds must have four coordinates in <west,south,east,north> order" << std::endl;
            return -1;
        }
    }
    double padding = result["padding"].as<double>();

    // Parse image properties
    int width = result["width"].as<int>();
    int height = result["height"].as<int>();
    double ratio = result["ratio"].as<double>();

    // Parse local style file
    auto styleJsonPath = result["style"].as<std::string>();
    auto in = std::ifstream(styleJsonPath);
    if (in.fail()) {
        std::cerr << "Cannot open style json file" << std::endl;
        return -1;
    }

    std::string styleStr;
    try {
        if (verbose) {
            std::cout << "Parsing style json" << std::endl;
        }

        auto styleJson = parseJson(in);

        // Insert Pointr source
        if (result.count("source")) {
            auto source = result["source"].as<std::string>();
            if (isUrl(source)) {
                // Source must be a vector tiles url
                if (verbose) {
                    std::cout << "Setting vector tiles url as source" << std::endl;
                }
                styleJson["sources"]["source_ptr"]["type"] = "vector";
                styleJson["sources"]["source_ptr"]["url"] = source;
            } else {
                // Source must be a local geojson file
                if (verbose) {
                    std::cout << "Parsing local source geojson" << std::endl;
                }
                auto inSource = std::ifstream{source};
                if (inSource.fail()) {
                    std::cerr << "Cannot open source geojson file" << std::endl;
                    return -1;
                }
                auto sourceGeojson = parseJson(inSource);
                auto itResult = sourceGeojson.find("result");
                if (itResult != sourceGeojson.end()) {
                    sourceGeojson = *itResult;
                }

                if (result.count("bid") && result.count("lvl")) {
                    auto bid = result["bid"].as<std::string>();
                    int lvl = result["lvl"].as<int>();
                    if (verbose) {
                        std::cout << "Filtering source with given bid/lvl" << std::endl;
                    }
                    filterSourceWithBidLvl(sourceGeojson, bid, lvl);
                }

                if (verbose) {
                    std::cout << "Setting geojson as source" << std::endl;
                }
                styleJson["sources"]["source_ptr"]["type"] = "geojson";
                styleJson["sources"]["source_ptr"]["data"] = sourceGeojson;
                std::cout << "Set geojson source" << std::endl;
            }
        }

        if (!result["symbols"].as<bool>()) {
            if (verbose) {
                std::cout << "Removing symbol layers" << std::endl;
            }
            removeSymbolLayers(styleJson);
        }

        if (result.count("bid") && result.count("lvl")) {
            auto bid = result["bid"].as<std::string>();
            int lvl = result["lvl"].as<int>();
            if (verbose) {
                std::cout << "Filtering style with given bid/lvl" << std::endl;
            }
            filterStyleWithBidLvl(styleJson, bid, lvl);
        }

        // Dump to string
        styleStr = styleJson.dump();
    } catch (const nlohmann::json::exception &e) {
        std::cerr << "An exception occurred while processing style/source json: " << e.what() << std::endl;
        return -2;
    }

    try {
        // Create map
        PTR::MapRenderer mapRenderer{styleStr, width, height, ratio, centerLon, centerLat, zoom};

        // Set map bounds if provided
        if (!bounds.empty()) {
            mapRenderer.setBounds(bounds[0], bounds[1], bounds[2], bounds[3], padding);
        }

        // Render png map image
        if (verbose) {
            std::cout << "Rendering map image" << std::endl;
        }
        auto str = mapRenderer.renderPNG();

        // Write png to file
        auto outPath = result["output"].as<std::string>();
        std::ofstream out(outPath);
        out << str;
        out.close();

        if (verbose) {
            std::cout << "Map image saved to " << outPath << std::endl;
        }

        // Write metadata to file
        auto box = mapRenderer.getBoundingBox();
        auto southWest = box.first;
        auto northEast = box.second;
        nlohmann::ordered_json metadata;
        metadata["boundingBox"]["west"] = southWest.longitude();
        metadata["boundingBox"]["south"] = southWest.latitude();
        metadata["boundingBox"]["east"] = northEast.longitude();
        metadata["boundingBox"]["north"] = northEast.latitude();

        auto metadataStr = metadata.dump();

        if (verbose) {
            std::cout << "Metadata: " << metadataStr << std::endl;
        }

        if (result.count("metadata")) {
            auto outPathMetadata = result["metadata"].as<std::string>();
            std::ofstream outMetadata(outPathMetadata);
            outMetadata << metadataStr;
            outMetadata.close();
            if (verbose) {
                std::cout << "Metadata saved to " << outPathMetadata << std::endl;
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "An exception occurred while generating map image: " << e.what() << std::endl;
        return -3;
    }
}
