#include <mbgl/renderer/layers/render_raster_layer.hpp>
#include <mbgl/renderer/layers/raster_layer_tweaker.hpp>
#include <mbgl/renderer/buckets/raster_bucket.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/sources/render_image_source.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/programs/raster_program.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/math/angles.hpp>
#include <mbgl/style/layers/raster_layer_impl.hpp>
#include <mbgl/shaders/shader_program_base.hpp>

namespace mbgl {

using namespace style;

namespace {

inline const RasterLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == RasterLayer::Impl::staticTypeInfo());
    return static_cast<const RasterLayer::Impl&>(*impl);
}

} // namespace

RenderRasterLayer::RenderRasterLayer(Immutable<style::RasterLayer::Impl> _impl)
    : RenderLayer(makeMutable<RasterLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {}

RenderRasterLayer::~RenderRasterLayer() = default;

void RenderRasterLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
}

void RenderRasterLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    auto properties = makeMutable<RasterLayerProperties>(staticImmutableCast<RasterLayer::Impl>(baseImpl),
                                                         unevaluated.evaluate(parameters));
    passes = properties->evaluated.get<style::RasterOpacity>() > 0 ? RenderPass::Translucent : RenderPass::None;
    properties->renderPasses = mbgl::underlying_type(passes);
    evaluatedProperties = std::move(properties);

    if (tileLayerGroup && tileLayerGroup->getLayerTweaker()) {
        tileLayerGroup->setLayerTweaker(std::make_shared<RasterLayerTweaker>(evaluatedProperties));
    }
}

bool RenderRasterLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderRasterLayer::hasCrossfade() const {
    return false;
}

static float saturationFactor(float saturation) {
    if (saturation > 0) {
        return 1.f - 1.f / (1.001f - saturation);
    } else {
        return -saturation;
    }
}

static float contrastFactor(float contrast) {
    if (contrast > 0) {
        return 1 / (1 - contrast);
    } else {
        return 1 + contrast;
    }
}

static std::array<float, 3> spinWeights(float spin) {
    spin = util::deg2radf(spin);
    float s = std::sin(spin);
    float c = std::cos(spin);
    std::array<float, 3> spin_weights = {
        {(2 * c + 1) / 3, (-std::sqrt(3.0f) * s - c + 1) / 3, (std::sqrt(3.0f) * s - c + 1) / 3}};
    return spin_weights;
}

void RenderRasterLayer::prepare(const LayerPrepareParameters& params) {
    renderTiles = params.source->getRenderTiles();
    imageData = params.source->getImageRenderData();
    // It is possible image data is not available until the source loads it.
    assert(renderTiles || imageData || !params.source->isEnabled());
}

void RenderRasterLayer::render(PaintParameters& parameters) {
    if (parameters.pass != RenderPass::Translucent || (!renderTiles && !imageData)) {
        return;
    }

    if (!parameters.shaders.getLegacyGroup().populate(rasterProgram)) return;

    const auto& evaluated = static_cast<const RasterLayerProperties&>(*evaluatedProperties).evaluated;
    RasterProgram::Binders paintAttributeData{evaluated, 0};

    auto draw = [&](const mat4& matrix,
                    const auto& vertexBuffer,
                    const auto& indexBuffer,
                    const auto& segments,
                    const auto& textureBindings,
                    const std::string& drawScopeID) {
        const auto allUniformValues = RasterProgram::computeAllUniformValues(
            RasterProgram::LayoutUniformValues{
                uniforms::matrix::Value(matrix),
                uniforms::opacity::Value(evaluated.get<RasterOpacity>()),
                uniforms::fade_t::Value(1),
                uniforms::brightness_low::Value(evaluated.get<RasterBrightnessMin>()),
                uniforms::brightness_high::Value(evaluated.get<RasterBrightnessMax>()),
                uniforms::saturation_factor::Value(saturationFactor(evaluated.get<RasterSaturation>())),
                uniforms::contrast_factor::Value(contrastFactor(evaluated.get<RasterContrast>())),
                uniforms::spin_weights::Value(spinWeights(evaluated.get<RasterHueRotate>())),
                uniforms::buffer_scale::Value(1.0f),
                uniforms::scale_parent::Value(1.0f),
                uniforms::tl_parent::Value(std::array<float, 2>{{0.0f, 0.0f}}),
            },
            paintAttributeData,
            evaluated,
            static_cast<float>(parameters.state.getZoom()));
        const auto allAttributeBindings = RasterProgram::computeAllAttributeBindings(
            vertexBuffer, paintAttributeData, evaluated);

        checkRenderability(parameters, RasterProgram::activeBindingCount(allAttributeBindings));

        rasterProgram->draw(parameters.context,
                            *parameters.renderPass,
                            gfx::Triangles(),
                            parameters.depthModeForSublayer(0, gfx::DepthMaskType::ReadOnly),
                            gfx::StencilMode::disabled(),
                            parameters.colorModeForRenderPass(),
                            gfx::CullFaceMode::disabled(),
                            indexBuffer,
                            segments,
                            allUniformValues,
                            allAttributeBindings,
                            textureBindings,
                            getID() + "/" + drawScopeID);
    };

    const gfx::TextureFilterType filter = evaluated.get<RasterResampling>() == RasterResamplingType::Nearest
                                              ? gfx::TextureFilterType::Nearest
                                              : gfx::TextureFilterType::Linear;

    if (imageData && !imageData->bucket->needsUpload()) {
        RasterBucket& bucket = *imageData->bucket;
        assert(bucket.texture);

        size_t i = 0;
        for (const auto& matrix_ : imageData->matrices) {
            draw(matrix_,
                 *bucket.vertexBuffer,
                 *bucket.indexBuffer,
                 bucket.segments,
                 RasterProgram::TextureBindings{
                     textures::image0::Value{bucket.texture->getResource(), filter},
                     textures::image1::Value{bucket.texture->getResource(), filter},
                 },
                 std::to_string(i++));
        }
    } else if (renderTiles) {
        for (const RenderTile& tile : *renderTiles) {
            auto* bucket_ = tile.getBucket(*baseImpl);
            if (!bucket_) {
                continue;
            }
            auto& bucket = static_cast<RasterBucket&>(*bucket_);

            if (!bucket.hasData()) continue;

            assert(bucket.texture);
            if (bucket.vertexBuffer && bucket.indexBuffer) {
                // Draw only the parts of the tile that aren't drawn by another tile in the layer.
                draw(parameters.matrixForTile(tile.id, !parameters.state.isChanging()),
                     *bucket.vertexBuffer,
                     *bucket.indexBuffer,
                     bucket.segments,
                     RasterProgram::TextureBindings{
                         textures::image0::Value{bucket.texture->getResource(), filter},
                         textures::image1::Value{bucket.texture->getResource(), filter},
                     },
                     "image");
            } else {
                // Draw the full tile.
                if (bucket.segments.empty()) {
                    // Copy over the segments so that we can create our own DrawScopes.
                    bucket.segments = RenderStaticData::rasterSegments();
                }
                draw(parameters.matrixForTile(tile.id, !parameters.state.isChanging()),
                     *parameters.staticData.rasterVertexBuffer,
                     *parameters.staticData.quadTriangleIndexBuffer,
                     bucket.segments,
                     RasterProgram::TextureBindings{
                         textures::image0::Value{bucket.texture->getResource(), filter},
                         textures::image1::Value{bucket.texture->getResource(), filter},
                     },
                     "image");
            }
        }
    }
}

void RenderRasterLayer::layerRemoved(UniqueChangeRequestVec& changes) {
    // Remove everything
    if (tileLayerGroup) {
        changes.emplace_back(std::make_unique<RemoveLayerGroupRequest>(tileLayerGroup->getLayerIndex()));
        tileLayerGroup.reset();
    }
}

void RenderRasterLayer::removeTile(RenderPass renderPass, const OverscaledTileID& tileID) {
    stats.tileDrawablesRemoved += tileLayerGroup->removeDrawables(renderPass, tileID).size();
}

void RenderRasterLayer::update(gfx::ShaderRegistry& shaders,
                               gfx::Context& context,
                               const TransformState& /*state*/,
                               [[maybe_unused]] const RenderTree& renderTree,
                               [[maybe_unused]] UniqueChangeRequestVec& changes) {
    std::unique_lock<std::mutex> guard(mutex);

    if ((!renderTiles || renderTiles->empty()) && !imageData) {
        if (tileLayerGroup) {
            stats.tileDrawablesRemoved += tileLayerGroup->getDrawableCount();
            tileLayerGroup->clearDrawables();
        }
        return;
    }

    // Set up a layer group
    if (!tileLayerGroup) {
        tileLayerGroup = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64, getID());
        if (!tileLayerGroup) {
            return;
        }
        tileLayerGroup->setLayerTweaker(std::make_shared<RasterLayerTweaker>(evaluatedProperties));
        changes.emplace_back(std::make_unique<AddLayerGroupRequest>(tileLayerGroup, /*canReplace=*/true));
    }

    auto renderPass = RenderPass::Translucent;

    if (!rasterShader) {
        rasterShader = context.getGenericShader(shaders, "RasterShader");
    }

    const auto& evaluated = static_cast<const RasterLayerProperties&>(*evaluatedProperties).evaluated;
    RasterProgram::Binders paintAttributeData{evaluated, 0};

    const gfx::TextureFilterType filter = evaluated.get<RasterResampling>() == RasterResamplingType::Nearest
                                              ? gfx::TextureFilterType::Nearest
                                              : gfx::TextureFilterType::Linear;
    (void)filter;

    if (imageData && !imageData->bucket->needsUpload()) {
        // TODO: implement arbitrary image
        // RasterBucket& bucket = *imageData->bucket;
        assert(imageData->bucket->texture);

        //        size_t i = 0;
        for (const auto& matrix_ : imageData->matrices) {
            /*draw(matrix_,
                 *bucket.vertexBuffer,
                 *bucket.indexBuffer,
                 bucket.segments,
                 RasterProgram::TextureBindings{
                     textures::image0::Value{bucket.texture->getResource(), filter},
                     textures::image1::Value{bucket.texture->getResource(), filter},
                 },
                 std::to_string(i++));*/
            (void)matrix_;
        }
    } else if (renderTiles) {
        tileLayerGroup->observeDrawables([&](gfx::UniqueDrawable& drawable) {
            // Has this tile dropped out of the cover set?
            if (const auto it = std::find_if(renderTiles->begin(),
                                             renderTiles->end(),
                                             [&drawable](const auto& renderTile) {
                                                 return drawable->getTileID() == renderTile.get().getOverscaledTileID();
                                             });
                it == renderTiles->end()) {
                // remove it
                drawable.reset();
                ++stats.tileDrawablesRemoved;
            }
        });

        for (const RenderTile& tile : *renderTiles) {
            const auto& tileID = tile.getOverscaledTileID();
            auto* bucket_ = tile.getBucket(*baseImpl);
            if (!bucket_) {
                continue;
            }
            auto& bucket = static_cast<RasterBucket&>(*bucket_);

            if (!bucket.hasData()) continue;

            if (tileLayerGroup->getDrawableCount(renderPass, tileID) > 0) continue;

            if (bucket.image) {
                std::unique_ptr<gfx::DrawableBuilder> builder{context.createDrawableBuilder("raster")};
                builder->setShader(rasterShader);
                builder->setRenderPass(renderPass);
                builder->setColorAttrMode(gfx::DrawableBuilder::ColorAttrMode::None);
                builder->setSubLayerIndex(0);
                builder->setDepthType((renderPass == RenderPass::Opaque) ? gfx::DepthMaskType::ReadWrite
                                                                         : gfx::DepthMaskType::ReadOnly);
                builder->setCullFaceMode(gfx::CullFaceMode::disabled());
                builder->setVertexAttrName("a_pos");

                // render data
                auto buildRenderData = [](const TileMask& mask,
                                          std::vector<std::array<int16_t, 2>>& vertices,
                                          std::vector<std::array<int16_t, 2>>& attributes,
                                          std::vector<uint16_t>& indices,
                                          std::vector<Segment<void>>& segments) {
                    constexpr const uint16_t vertexLength = 4;

                    // Create the vertex buffer for the specified tile mask.
                    for (const auto& id : mask) {
                        // Create a quad for every masked tile.
                        const int32_t vertexExtent = util::EXTENT >> id.z;

                        const Point<int16_t> tlVertex = {static_cast<int16_t>(id.x * vertexExtent),
                                                         static_cast<int16_t>(id.y * vertexExtent)};
                        const Point<int16_t> brVertex = {static_cast<int16_t>(tlVertex.x + vertexExtent),
                                                         static_cast<int16_t>(tlVertex.y + vertexExtent)};

                        if (segments.empty() ||
                            (segments.back().vertexLength + vertexLength > std::numeric_limits<uint16_t>::max())) {
                            // Move to a new segments because the old one can't hold the geometry.
                            segments.emplace_back(vertices.size(), indices.size());
                        }

                        vertices.emplace_back(std::array<int16_t, 2>{{tlVertex.x, tlVertex.y}});
                        attributes.emplace_back(std::array<int16_t, 2>{{tlVertex.x, tlVertex.y}});

                        vertices.emplace_back(std::array<int16_t, 2>{{brVertex.x, tlVertex.y}});
                        attributes.emplace_back(std::array<int16_t, 2>{{brVertex.x, tlVertex.y}});

                        vertices.emplace_back(std::array<int16_t, 2>{{tlVertex.x, brVertex.y}});
                        attributes.emplace_back(std::array<int16_t, 2>{{tlVertex.x, brVertex.y}});

                        vertices.emplace_back(std::array<int16_t, 2>{{brVertex.x, brVertex.y}});
                        attributes.emplace_back(std::array<int16_t, 2>{{brVertex.x, brVertex.y}});

                        auto& segment = segments.back();
                        assert(segment.vertexLength <= std::numeric_limits<uint16_t>::max());
                        const auto offset = static_cast<uint16_t>(segment.vertexLength);

                        // 0, 1, 2
                        // 1, 2, 3
                        indices.insert(indices.end(),
                                       {offset, static_cast<uint16_t>(offset + 1), static_cast<uint16_t>(offset + 2u)});
                        indices.insert(indices.end(),
                                       {static_cast<uint16_t>(offset + 1),
                                        static_cast<uint16_t>(offset + 2),
                                        static_cast<uint16_t>(offset + 3)});

                        segment.vertexLength += vertexLength;
                        segment.indexLength += 6;
                    }
                };

                std::vector<std::array<int16_t, 2>> vertices, attributes;
                std::vector<uint16_t> indices;
                std::vector<Segment<void>> segments;
                buildRenderData(bucket.mask, vertices, attributes, indices, segments);
                builder->addVertices(vertices, 0, vertices.size());
                builder->setSegments(gfx::Triangles(), indices, segments);

                // attributes
                {
                    gfx::VertexAttributeArray vertexAttrs;
                    if (auto& attr = vertexAttrs.getOrAdd("a_texture_pos")) {
                        std::size_t index{0};
                        for (auto& a : attributes) {
                            attr->set<gfx::VertexAttribute::int2>(index++, {a[0], a[1]});
                        }
                    }
                    builder->setVertexAttributes(std::move(vertexAttrs));
                }

                // textures
                // TODO: move texture creation into gfx::Context
                std::shared_ptr<gfx::Texture2D> tex0 = context.createTexture2D();
                tex0->setImage(bucket.image).create();
                auto location0 = rasterShader->getSamplerLocation("u_image0");
                if (location0.has_value()) {
                    builder->setTexture(tex0, location0.value());
                }
                std::shared_ptr<gfx::Texture2D> tex1 = context.createTexture2D();
                tex1->setImage(bucket.image).create();
                auto location1 = rasterShader->getSamplerLocation("u_image1");
                if (location1.has_value()) {
                    builder->setTexture(tex1, location1.value());
                }

                // finish
                builder->flush();
                for (auto& drawable : builder->clearDrawables()) {
                    drawable->setTileID(tileID);
                    tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                    ++stats.tileDrawablesAdded;
                }
            };
        }
    }
}

} // namespace mbgl
