#include "example_custom_drawable_style_layer.h"

#include <mbgl/style/layer.hpp>
#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/gfx/drawable.hpp>

#include <memory>
#include <cmath>
#include <filesystem>

ExampleCustomDrawableStyleLayerHost::ExampleCustomDrawableStyleLayerHost() {
    
}
    
void ExampleCustomDrawableStyleLayerHost::initialize() {

}

void ExampleCustomDrawableStyleLayerHost::deinitialize() {

}
    
void ExampleCustomDrawableStyleLayerHost::update(Interface& interface) {

    // if we have built our drawable(s) already, either update or skip
    if (interface.getDrawableCount() == 0) {
        createDrawables(interface);
        return;
    }
}

void ExampleCustomDrawableStyleLayerHost::createDrawables(Interface& interface) {
    
    constexpr float extent = mbgl::util::EXTENT;

    // add classic polylines
    {
        using namespace mbgl;
            
        // set tile
        interface.setTileID({11, 327, 792});
            
        constexpr auto numLines = 6;
        Interface::LineOptions options[numLines] {
            {/*geometry=*/{},   /*blur=*/0.0f,  /*opacity=*/1.0f, /*gapWidth=*/0.0f, /*offset=*/0.0f,   /*width=*/8.0f,     /*color=*/Color::red() },
            {/*geometry=*/{},   /*blur=*/4.0f,  /*opacity=*/1.0f, /*gapWidth=*/2.0f, /*offset=*/-1.0f,  /*width=*/4.0f,     /*color=*/Color::blue() },
            {/*geometry=*/{},   /*blur=*/16.0f, /*opacity=*/1.0f, /*gapWidth=*/1.0f, /*offset=*/2.0f,   /*width=*/16.0f,    /*color=*/Color(1.f, 0.5f, 0, 0.5f) },
            {/*geometry=*/{},   /*blur=*/2.0f,  /*opacity=*/1.0f, /*gapWidth=*/1.0f, /*offset=*/-2.0f,  /*width=*/2.0f,     /*color=*/Color(1.f, 1.f, 0, 0.3f) },
            {/*geometry=*/{},   /*blur=*/0.5f,  /*opacity=*/0.5f, /*gapWidth=*/1.0f, /*offset=*/0.5f,   /*width=*/0.5f,     /*color=*/Color::black() },
            {/*geometry=*/{},   /*blur=*/24.0f, /*opacity=*/0.5f, /*gapWidth=*/1.0f, /*offset=*/-5.0f,  /*width=*/24.0f,    /*color=*/Color(1.f, 0, 1.f, 0.2f) },
        };

        for(auto& opt: options) {
            opt.geometry.beginCap = style::LineCapType::Butt;
            opt.geometry.endCap = style::LineCapType::Butt;
            opt.geometry.joinType = style::LineJoinType::Miter;
        }
            
        constexpr auto numPoints = 10;
        GeometryCoordinates polyline;
        for (auto ipoint{0}; ipoint < numPoints; ++ipoint) {
            polyline.emplace_back(
                static_cast<int16_t>(ipoint * extent / numPoints),
                static_cast<int16_t>(std::sin(ipoint * 2 * M_PI / numPoints) * extent / numLines / 2.f));
        }
            
        for (auto index {0}; index <  numLines; ++index) {
            for(auto &p : polyline) {
                p.y += extent / numLines;
            }
                
            // set property values
            interface.setLineOptions(options[index]);
                
            // add polyline
            interface.addPolyline(polyline, Interface::LineShaderType::Classic);
        }
    }

    // add wide vector polylines with tile coordinates
    {
        using namespace mbgl;
            
        // set tile
        interface.setTileID({11, 327, 792});
            
        constexpr auto numLines = 6;
        Interface::LineOptions options[numLines] {
            {/*geometry=*/{},   /*blur=*/0.0f,  /*opacity=*/1.0f, /*gapWidth=*/0.0f, /*offset=*/0.0f,   /*width=*/8.0f,     /*color=*/Color::red() },
            {/*geometry=*/{},   /*blur=*/4.0f,  /*opacity=*/1.0f, /*gapWidth=*/2.0f, /*offset=*/-1.0f,  /*width=*/4.0f,     /*color=*/Color::blue() },
            {/*geometry=*/{},   /*blur=*/16.0f, /*opacity=*/1.0f, /*gapWidth=*/1.0f, /*offset=*/2.0f,   /*width=*/16.0f,    /*color=*/Color(1.f, 0.5f, 0, 0.5f) },
            {/*geometry=*/{},   /*blur=*/2.0f,  /*opacity=*/1.0f, /*gapWidth=*/1.0f, /*offset=*/-2.0f,  /*width=*/2.0f,     /*color=*/Color(1.f, 1.f, 0, 0.3f) },
            {/*geometry=*/{},   /*blur=*/0.5f,  /*opacity=*/0.5f, /*gapWidth=*/1.0f, /*offset=*/0.5f,   /*width=*/0.5f,     /*color=*/Color::black() },
            {/*geometry=*/{},   /*blur=*/24.0f, /*opacity=*/0.5f, /*gapWidth=*/1.0f, /*offset=*/-5.0f,  /*width=*/24.0f,    /*color=*/Color(1.f, 0, 1.f, 0.2f) },
        };

        for(auto& opt: options) {
            opt.geometry.beginCap = style::LineCapType::Butt;
            opt.geometry.endCap = style::LineCapType::Butt;
            opt.geometry.joinType = style::LineJoinType::Miter;
        }
            
        constexpr auto numPoints = 10;
        GeometryCoordinates polyline;
        for (auto ipoint{0}; ipoint < numPoints; ++ipoint) {
            polyline.emplace_back(
                static_cast<int16_t>(ipoint * extent / numPoints),
                static_cast<int16_t>(std::sin(ipoint * 2 * M_PI / numPoints) * extent / numLines / 2.f));
        }
            
        for (auto index {0}; index <  numLines; ++index) {
            for(auto &p : polyline) {
                if (0 == index) p.y += 0.25f * extent / numLines;
                p.y += extent / numLines;
            }
                
            // set property values
            interface.setLineOptions(options[index]);
                
            // add polyline
            interface.addPolyline(polyline);
                
            // add clone
            for(auto &p : polyline) {
                p.y += 0.05f * extent / numLines;
            }
            interface.addPolyline(polyline);
            for(auto &p : polyline) {
                p.y -= 0.05f * extent / numLines;
            }
        }
    }
        
    // add fill polygon
    {
        using namespace mbgl;

        // set tile
        interface.setTileID({11, 327, 790});

        GeometryCollection geometry{
            {
                // ring 1
                {static_cast<int16_t>(extent* 0.1f), static_cast<int16_t>(extent* 0.2f)},
                {static_cast<int16_t>(extent* 0.5f), static_cast<int16_t>(extent* 0.5f)},
                {static_cast<int16_t>(extent* 0.7f), static_cast<int16_t>(extent* 0.5f)},
                {static_cast<int16_t>(extent* 0.5f), static_cast<int16_t>(extent* 1.0f)},
                {static_cast<int16_t>(extent* 0.0f), static_cast<int16_t>(extent* 0.5f)},
                {static_cast<int16_t>(extent* 0.1f), static_cast<int16_t>(extent* 0.2f)},
            },
            {
                // ring 2
                {static_cast<int16_t>(extent* 0.1f), static_cast<int16_t>(extent* 0.25f)},
                {static_cast<int16_t>(extent* 0.15f), static_cast<int16_t>(extent* 0.5f)},
                {static_cast<int16_t>(extent* 0.25f), static_cast<int16_t>(extent* 0.45f)},
                {static_cast<int16_t>(extent* 0.1f), static_cast<int16_t>(extent* 0.25f)},
            },
        };

        // set properties
        interface.setFillOptions({/*color=*/Color::green(), /*opacity=*/0.5f});

        // add fill
        interface.addFill(geometry);
    }
        
    // add symbol
    {
        using namespace mbgl;

        // set tile
        interface.setTileID({11, 327, 789});

        GeometryCoordinate position {static_cast<int16_t>(extent* 0.5f), static_cast<int16_t>(extent* 0.5f)};

        // load image
        std::shared_ptr<PremultipliedImage> image = std::make_shared<PremultipliedImage>(
            mbgl::decodeImage(mbgl::util::read_file("rocket.png")));

        // set symbol options
        Interface::SymbolOptions options;
        options.texture = interface.context.createTexture2D();
        options.texture->setImage(image);
        options.texture->setSamplerConfiguration(
            {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});

        const std::array<std::array<float, 2>, 2> textureCoordinates = {{{0.0f, 0.08f}, {1.0f, 0.9f}}};
        const float xspan = textureCoordinates[1][0] - textureCoordinates[0][0];
        const float yspan = textureCoordinates[1][1] - textureCoordinates[0][1];
        assert(xspan > 0.0f && yspan > 0.0f);
        options.size = {static_cast<uint32_t>(image->size.width * xspan),
                        static_cast<uint32_t>(image->size.height * yspan)};

        options.anchor = {0.5f, 0.95f};
        options.angleDegrees = 45.0f;
        options.scaleWithMap = true;
        options.pitchWithMap = false;
        interface.setSymbolOptions(options);

        // add symbol
        interface.addSymbol(position, textureCoordinates);
    }

    {
        using namespace mbgl;

        GeometryCoordinate position{static_cast<int16_t>(extent * 0.5f), static_cast<int16_t>(extent * 0.5f)};

        // load image
        std::shared_ptr<PremultipliedImage> image = std::make_shared<PremultipliedImage>(
            mbgl::decodeImage(mbgl::util::read_file("rocket.png")));

        // set symbol options
        Interface::SymbolOptions options;
        options.texture = interface.context.createTexture2D();
        options.texture->setImage(image);
        options.texture->setSamplerConfiguration(
            {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});

        options.size = {static_cast<uint32_t>(image->size.width), static_cast<uint32_t>(image->size.height)};

        options.anchor = {0.5f, 0.95f};
        options.angleDegrees = 45.0f;
        options.scaleWithMap = true;
        options.pitchWithMap = false;
        interface.setSymbolOptions(options);

        // add symbol
        interface.addSymbol(position);
    }

    // add polylines using wide vectors using geographic coordinates
    {
        using namespace mbgl;

        // add polyline with geographic coordinates
        Interface::LineOptions options = {/*geometry=*/{},
                                          /*blur=*/0.0f,
                                          /*opacity=*/1.0f,
                                          /*gapWidth=*/0.0f,
                                          /*offset=*/0.0f,
                                          /*width=*/12.0f,
                                          /*color=*/{.0f, .0f, .0f, .5f}};

        options.geometry.beginCap = style::LineCapType::Square;
        options.geometry.endCap = style::LineCapType::Square;
        options.geometry.joinType = style::LineJoinType::Bevel;
        options.geometry.type = FeatureType::LineString;
        interface.setLineOptions(options);

        LineString<double> polyline_geo {
            // San Francisco
            {-122.38186800073211, 37.77466003457463},
            {-122.3869373450997, 37.774352128895615},
            {-122.38680767979824, 37.773294612880306},
            {-122.38476465260224, 37.773350946288765},
            {-122.38146223043374, 37.77194168964067},
            {-122.6813560305925, 37.666084247570964},
            {-122.26765538866474, 37.65037232584494},
            {-122.42528413673159, 38.020443518012584},
            // Seattle
            {-122.34927775216809, 47.62050663596438},
            // New York
            {-74.04454331829972, 40.6892168305434},
        };
        interface.addPolyline(polyline_geo, Interface::LineShaderType::Classic);
    }
        
    // add polylines using wide vectors in tile coordinates
    {
        using namespace mbgl;
            
        // set tile
        interface.setTileID({11, 327, 790});

        Interface::LineOptions options{/*geometry=*/{},
                                       /*blur=*/0.0f,
                                       /*opacity=*/1.0f,
                                       /*gapWidth=*/0.0f,
                                       /*offset=*/0.0f,
                                       /*width=*/7.0f,
                                       /*color=*/{1.0f, 0, 0, .5f}};

        options.geometry.beginCap = style::LineCapType::Round;
        options.geometry.endCap = style::LineCapType::Round;
        options.geometry.joinType = style::LineJoinType::Round;
            
        // add polyline with tile coordinates
        GeometryCollection polyline_tile{
            {
                // ring 1
                {static_cast<int16_t>(extent* 0.1f), static_cast<int16_t>(extent* 0.2f)},
                {static_cast<int16_t>(extent* 0.5f), static_cast<int16_t>(extent* 0.5f)},
                {static_cast<int16_t>(extent* 0.7f), static_cast<int16_t>(extent* 0.5f)},
                {static_cast<int16_t>(extent* 0.5f), static_cast<int16_t>(extent* 1.0f)},
                {static_cast<int16_t>(extent* 0.0f), static_cast<int16_t>(extent* 0.5f)},
            },
            {
                // ring 2
                {static_cast<int16_t>(extent* 0.1f), static_cast<int16_t>(extent* 0.25f)},
                {static_cast<int16_t>(extent* 0.15f), static_cast<int16_t>(extent* 0.5f)},
                {static_cast<int16_t>(extent* 0.25f), static_cast<int16_t>(extent* 0.45f)},
            },
        };
            
        options.geometry.type = FeatureType::Polygon;
        interface.setLineOptions(options);
        interface.addPolyline(polyline_tile[0], Interface::LineShaderType::Classic);
        interface.addPolyline(polyline_tile[1], Interface::LineShaderType::Classic);

    }

    generateCommonGeometry(interface);
    loadCommonGeometry(interface);

    // finish
    interface.finish();
}

static mbgl::Point<double> project(const mbgl::LatLng& c, const mbgl::TransformState& s) {
    mbgl::LatLng unwrappedLatLng = c.wrapped();
    unwrappedLatLng.unwrapForShortestPath(s.getLatLng(mbgl::LatLng::Wrapped));
    return mbgl::Projection::project(unwrappedLatLng, s.getScale());
}

void ExampleCustomDrawableStyleLayerHost::generateCommonGeometry(Interface& interface) {
    constexpr bool useTextures = true;
    constexpr float itemScale = 10.0f;
    const mbgl::LatLng location{37.78, -122.47};

    Interface::CommonGeometryOptions options;

    options.color = mbgl::Color::green();

    if (useTextures) {
        // load image
        std::shared_ptr<mbgl::PremultipliedImage> image = std::make_shared<mbgl::PremultipliedImage>(
            mbgl::decodeImage(mbgl::util::read_file("rocket.png")));

        options.texture = interface.context.createTexture2D();
        options.texture->setImage(image);
        options.texture->setSamplerConfiguration({mbgl::gfx::TextureFilterType::Linear,
                                                  mbgl::gfx::TextureWrapType::Clamp,
                                                  mbgl::gfx::TextureWrapType::Clamp});
    }

    using VertexVector = mbgl::gfx::VertexVector<Interface::CommonGeometryVertex>;
    const std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
    VertexVector& vertices = *sharedVertices;

    using TriangleIndexVector = mbgl::gfx::IndexVector<mbgl::gfx::Triangles>;
    const std::shared_ptr<TriangleIndexVector> sharedIndices = std::make_shared<TriangleIndexVector>();
    TriangleIndexVector& indices = *sharedIndices;

    const unsigned long numVtxCircumference = 72;
    const float bearingStep = 360.0f / static_cast<float>(numVtxCircumference - 1);

    vertices.emplace_back(Interface::CommonGeometryVertex{0.0f, 0.0f, 0.0f, 0.5f, 0.5f});

    for (unsigned long i = 1; i <= numVtxCircumference; ++i) {
        const float rad = mbgl::util::deg2radf((i - 1) * bearingStep);

        Interface::CommonGeometryVertex vertex;
        vertex.position = {sinf(rad) / 2.0f, cosf(rad) / 2.0f, 0.0f};
        vertex.texcoords = {0.5f + vertex.position[0], 0.5f - vertex.position[1]};

        vertices.emplace_back(std::move(vertex));
    }

    for (uint16_t i = 1; i < vertices.elements() - 1; ++i) {
        indices.emplace_back(0, i, static_cast<uint16_t>(i + 1));
    }
    indices.emplace_back(0, static_cast<uint16_t>(vertices.elements() - 1), 1);

    interface.setCommonGeometryOptions(options);

    interface.setCommonGeometryTweakerCallback([=, commonGeometryBearing = 0.0f](
                                                   mbgl::gfx::Drawable& drawable,
                                                   const mbgl::PaintParameters& params,
                                                   Interface::CommonGeometryOptions& currentOptions) mutable {
        mbgl::Point<double> center = project(location, params.state);

        commonGeometryBearing += 0.1f;
        float scale = itemScale * static_cast<float>(std::pow(
                                      2.f, params.state.getZoom() - drawable.getTileID()->toUnwrapped().canonical.z));

        mbgl::mat4 projMatrix;
        params.state.getProjMatrix(projMatrix);

        mbgl::mat4 matrix = mbgl::matrix::identity4();
        mbgl::matrix::translate(matrix, matrix, center.x, center.y, 0.0);
        mbgl::matrix::rotate_z(matrix, matrix, commonGeometryBearing);
        mbgl::matrix::scale(matrix, matrix, scale * params.pixelsToGLUnits[0], scale * params.pixelsToGLUnits[0], 1.0f);
        mbgl::matrix::multiply(currentOptions.matrix, projMatrix, matrix);
    });

    interface.addCommonGeometry(sharedVertices, sharedIndices);
}

void ExampleCustomDrawableStyleLayerHost::loadCommonGeometry(Interface& interface) {
    
}
