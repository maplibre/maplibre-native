#include <mbgl/renderer/layers/render_raster_layer.hpp>
#include <mbgl/renderer/buckets/raster_bucket.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/sources/render_image_source.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/programs/raster_program.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/math/angles.hpp>
#include <mbgl/style/layers/raster_layer_impl.hpp>

#if MLN_DRAWABLE_RENDERER
#include <mbgl/renderer/layers/raster_layer_tweaker.hpp>
#include <mbgl/gfx/image_drawable_data.hpp>
#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#endif

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

#if MLN_DRAWABLE_RENDERER
    if (layerGroup && layerGroup->getLayerTweaker()) {
        layerGroup->setLayerTweaker(std::make_shared<RasterLayerTweaker>(evaluatedProperties));
    }
#endif
}

bool RenderRasterLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderRasterLayer::hasCrossfade() const {
    return false;
}

#if MLN_LEGACY_RENDERER
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
#endif

void RenderRasterLayer::prepare(const LayerPrepareParameters& params) {
    renderTiles = params.source->getRenderTiles();
    imageData = params.source->getImageRenderData();
    // It is possible image data is not available until the source loads it.
    assert(renderTiles || imageData || !params.source->isEnabled());

#if MLN_DRAWABLE_RENDERER
    updateRenderTileIDs();
#endif // MLN_DRAWABLE_RENDERER
}

#if MLN_LEGACY_RENDERER
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

#endif // MLN_LEGACY_RENDERER

#if MLN_DRAWABLE_RENDERER
void RenderRasterLayer::markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) {
    RenderLayer::markLayerRenderable(willRender, changes);
    activateLayerGroup(imageLayerGroup, willRender, changes);
}

constexpr auto PosAttribName = "a_pos";
constexpr auto TexturePosAttribName = "a_texture_pos";
constexpr auto Image0UniformName = "u_image0";
constexpr auto Image1UniformName = "u_image1";

void RenderRasterLayer::update(gfx::ShaderRegistry& shaders,
                               gfx::Context& context,
                               const TransformState& /*state*/,
                               [[maybe_unused]] const RenderTree& renderTree,
                               [[maybe_unused]] UniqueChangeRequestVec& changes) {
    std::unique_lock<std::mutex> guard(mutex);

    if ((!renderTiles || renderTiles->empty()) && !imageData) {
        if (layerGroup) {
            stats.drawablesRemoved += layerGroup->clearDrawables();
        }
        if (imageLayerGroup) {
            stats.drawablesRemoved += imageLayerGroup->clearDrawables();
        }
        return;
    }

    auto renderPass = RenderPass::Translucent;

    if (!rasterShader) {
        rasterShader = context.getGenericShader(shaders, "RasterShader");
    }

    if (!staticDataVertices) {
        staticDataVertices = std::make_shared<RasterVertexVector>(RenderStaticData::rasterVertices());
    }
    if (!staticDataIndices) {
        staticDataIndices = std::make_shared<TriangleIndexVector>(RenderStaticData::quadTriangleIndices());
    }
    if (!staticDataSegments) {
        staticDataSegments = std::make_shared<RasterSegmentVector>(RenderStaticData::rasterSegments());
    }

    const auto& evaluated = static_cast<const RasterLayerProperties&>(*evaluatedProperties).evaluated;
    RasterProgram::Binders paintAttributeData{evaluated, 0};

    const gfx::TextureFilterType filter = evaluated.get<RasterResampling>() == RasterResamplingType::Nearest
                                              ? gfx::TextureFilterType::Nearest
                                              : gfx::TextureFilterType::Linear;

    auto createBuilder = [&context, &renderPass, this]() -> std::unique_ptr<gfx::DrawableBuilder> {
        std::unique_ptr<gfx::DrawableBuilder> builder{context.createDrawableBuilder("raster")};
        builder->setShader(rasterShader);
        builder->setRenderPass(renderPass);
        builder->setSubLayerIndex(0);
        builder->setDepthType((renderPass == RenderPass::Opaque) ? gfx::DepthMaskType::ReadWrite
                                                                 : gfx::DepthMaskType::ReadOnly);
        builder->setColorMode(gfx::ColorMode::alphaBlended());
        builder->setCullFaceMode(gfx::CullFaceMode::disabled());
        builder->setVertexAttrName(PosAttribName);

        return builder;
    };

    auto setTextures = [&context, &filter, this](std::unique_ptr<gfx::DrawableBuilder>& builder,
                                                 const RasterBucket& bucket) {
        // textures
        auto location0 = rasterShader->getSamplerLocation(Image0UniformName);
        if (location0.has_value()) {
            std::shared_ptr<gfx::Texture2D> tex0 = context.createTexture2D();
            tex0->setImage(bucket.image);
            tex0->setSamplerConfiguration({filter, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
            builder->setTexture(tex0, location0.value());
        }
        auto location1 = rasterShader->getSamplerLocation(Image1UniformName);
        if (location1.has_value()) {
            std::shared_ptr<gfx::Texture2D> tex1 = context.createTexture2D();
            tex1->setImage(bucket.image);
            tex1->setSamplerConfiguration({filter, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
            builder->setTexture(tex1, location1.value());
        }
    };

    auto buildVertexData = [this](std::unique_ptr<gfx::DrawableBuilder>& builder, const RasterBucket& bucket) {
        RasterVertexVectorPtr vertices = staticDataVertices;
        TriangleIndexVectorPtr indices = staticDataIndices;
        const RasterSegmentVector* segments = staticDataSegments.get();

        if (!bucket.vertices.empty() && !bucket.indices.empty() && !bucket.segments.empty()) {
            vertices = bucket.sharedVertices;
            indices = bucket.sharedTriangles;
            segments = &bucket.segments;
        }

        // attributes
        {
            gfx::VertexAttributeArray vertexAttrs;

            if (auto& attr = vertexAttrs.add(PosAttribName)) {
                attr->setSharedRawData(vertices,
                                       offsetof(RasterLayoutVertex, a1),
                                       /*vertexOffset=*/0,
                                       sizeof(RasterLayoutVertex),
                                       gfx::AttributeDataType::Short2);
            }

            if (auto& attr = vertexAttrs.add(TexturePosAttribName)) {
                attr->setSharedRawData(vertices,
                                       offsetof(RasterLayoutVertex, a2),
                                       /*vertexOffset=*/0,
                                       sizeof(RasterLayoutVertex),
                                       gfx::AttributeDataType::Short2);
            }
            builder->setVertexAttributes(std::move(vertexAttrs));
        }

        builder->setRawVertices({}, vertices->elements(), gfx::AttributeDataType::Short2);
        builder->setSegments(gfx::Triangles(), indices, segments->data(), segments->size());
    };

    auto buildDrawables = [&setTextures, &buildVertexData](std::unique_ptr<gfx::DrawableBuilder>& builder,
                                                           const RasterBucket& bucket) {
        buildVertexData(builder, bucket);
        setTextures(builder, bucket);
    };

    auto updateTileDrawables = [&](std::unique_ptr<gfx::DrawableBuilder>& builder,
                                   auto* tileLayerGroup,
                                   const auto& tileID,
                                   const RasterBucket& bucket) {
        // TODO: check for oportunity to skip

        buildVertexData(builder, bucket);
        tileLayerGroup->visitDrawables(renderPass, tileID, [&](gfx::Drawable& drawable) {
            for (auto& tex : drawable.getTextures()) {
                builder->setTexture(tex.second, tex.first);
            }
        });
    };

    if (imageData) {
        RasterBucket& bucket = *imageData->bucket;
        if (!bucket.vertices.empty()) {
            if (imageLayerGroup) {
                stats.drawablesRemoved += imageLayerGroup->clearDrawables();
            } else {
                // Set up a layer group
                imageLayerGroup = context.createLayerGroup(layerIndex, /*initialCapacity=*/64, getID());
                imageLayerGroup->setLayerTweaker(std::make_shared<RasterLayerTweaker>(evaluatedProperties));
                activateLayerGroup(imageLayerGroup, isRenderable, changes);
            }

            auto builder = createBuilder();
            for (const auto& matrix_ : imageData->matrices) {
                buildDrawables(builder, bucket);

                // finish
                builder->flush();

                for (auto& drawable : builder->clearDrawables()) {
                    drawable->setData(std::make_unique<gfx::ImageDrawableData>(matrix_));
                    imageLayerGroup->addDrawable(std::move(drawable));
                    ++stats.drawablesAdded;
                }
            }
        }
    } else if (renderTiles) {
        if (layerGroup) {
            stats.drawablesRemoved += layerGroup->removeDrawablesIf([&](gfx::Drawable& drawable) {
                // Has this tile dropped out of the cover set?
                return drawable.getTileID() && !hasRenderTile(*drawable.getTileID());
            });
        } else {
            // Set up a tile layer group
            if (auto layerGroup_ = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64, getID())) {
                layerGroup_->setLayerTweaker(std::make_shared<RasterLayerTweaker>(evaluatedProperties));
                setLayerGroup(std::move(layerGroup_), changes);
            }
        }

        auto* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());

        auto builder = createBuilder();
        for (const RenderTile& tile : *renderTiles) {
            const auto& tileID = tile.getOverscaledTileID();

            const auto* bucket_ = tile.getBucket(*baseImpl);
            if (!bucket_ || !bucket_->hasData()) {
                removeTile(renderPass, tileID);
                continue;
            }

            const auto& bucket = static_cast<const RasterBucket&>(*bucket_);

            const auto prevBucketID = getRenderTileBucketID(tileID);
            if (prevBucketID != util::SimpleIdentity::Empty && prevBucketID != bucket.getID()) {
                // This tile was previously set up from a different bucket, drop and re-create any drawables for it.
                removeTile(renderPass, tileID);
            }
            setRenderTileBucketID(tileID, bucket.getID());

            if (tileLayerGroup->getDrawableCount(renderPass, tileID) > 0) {
                // re-create drawable geometry and pass texture
                updateTileDrawables(builder, tileLayerGroup, tileID, bucket);

                // erase current drawable
                removeTile(renderPass, tileID);
            } else if (bucket.image) {
                // build new drawable for this tile
                buildDrawables(builder, bucket);
            };

            // finish
            builder->flush();
            for (auto& drawable : builder->clearDrawables()) {
                drawable->setTileID(tileID);
                tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                ++stats.drawablesAdded;
            }
        }
    }
}
#endif // MLN_DRAWABLE_RENDERER

} // namespace mbgl
