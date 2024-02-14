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
#include <mbgl/util/logging.hpp>

#if MLN_DRAWABLE_RENDERER
#include <mbgl/renderer/layers/raster_layer_tweaker.hpp>
#include <mbgl/gfx/image_drawable_data.hpp>
#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#endif

namespace mbgl {

using namespace style;
using namespace shaders;

namespace {

inline const RasterLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == RasterLayer::Impl::staticTypeInfo());
    return static_cast<const RasterLayer::Impl&>(*impl);
}

} // namespace

RenderRasterLayer::RenderRasterLayer(Immutable<style::RasterLayer::Impl> _impl)
    : RenderLayer(makeMutable<RasterLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {
    styleDependencies = unevaluated.getDependencies();
}

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
    if (layerTweaker) {
        layerTweaker->updateProperties(evaluatedProperties);
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
    if (imageLayerGroup) {
        activateLayerGroup(imageLayerGroup, willRender, changes);
    }
}

void RenderRasterLayer::layerRemoved(UniqueChangeRequestVec& changes) {
    RenderLayer::layerRemoved(changes);
    if (imageLayerGroup) {
        activateLayerGroup(imageLayerGroup, false, changes);
    }
}

void RenderRasterLayer::layerIndexChanged(int32_t newLayerIndex, UniqueChangeRequestVec& changes) {
    RenderLayer::layerIndexChanged(newLayerIndex, changes);

    changeLayerIndex(imageLayerGroup, newLayerIndex, changes);
}

static const StringIdentity idPosAttribName = stringIndexer().get("a_pos");
static const StringIdentity idTexturePosAttribName = stringIndexer().get("a_texture_pos");
static const StringIdentity idTexImage0Name = stringIndexer().get("u_image0");
static const StringIdentity idTexImage1Name = stringIndexer().get("u_image1");

void RenderRasterLayer::update(gfx::ShaderRegistry& shaders,
                               gfx::Context& context,
                               const TransformState& /*state*/,
                               const std::shared_ptr<UpdateParameters>&,
                               [[maybe_unused]] const RenderTree& renderTree,
                               [[maybe_unused]] UniqueChangeRequestVec& changes) {
    if ((!renderTiles || renderTiles->empty()) && !imageData) {
        if (layerGroup) {
            stats.drawablesRemoved += layerGroup->clearDrawables();
        }
        if (imageLayerGroup) {
            stats.drawablesRemoved += imageLayerGroup->clearDrawables();
        }
        return;
    }

    constexpr auto renderPass = RenderPass::Translucent;

    if (!rasterShader) {
        rasterShader = context.getGenericShader(shaders, "RasterShader");
        if (!rasterShader) {
            return;
        }
    }

    if (!layerTweaker) {
        layerTweaker = std::make_shared<RasterLayerTweaker>(getID(), evaluatedProperties);

        if (layerGroup) {
            layerGroup->addLayerTweaker(layerTweaker);
        }
        if (imageLayerGroup) {
            imageLayerGroup->addLayerTweaker(layerTweaker);
        }
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

    const auto createBuilder = [&] {
        auto builder = context.createDrawableBuilder("raster");
        builder->setShader(rasterShader);
        builder->setRenderPass(renderPass);
        builder->setSubLayerIndex(0);
        builder->setDepthType(gfx::DepthMaskType::ReadOnly);
        builder->setColorMode(gfx::ColorMode::alphaBlended());
        builder->setCullFaceMode(gfx::CullFaceMode::disabled());
        builder->setVertexAttrNameId(idPosAttribName);
        return builder;
    };

    const auto setTextures = [&](gfx::UniqueDrawableBuilder& builder, RasterBucket& bucket) {
        if (bucket.image) {
            if (!bucket.texture2d) {
                if (auto tex = context.createTexture2D()) {
                    tex->setImage(bucket.image);
                    bucket.texture2d = std::move(tex);
                }
            }

            if (bucket.texture2d) {
                const auto& evaluated = static_cast<const RasterLayerProperties&>(*evaluatedProperties).evaluated;
                const bool nearest = evaluated.get<RasterResampling>() == RasterResamplingType::Nearest;
                const auto filter = nearest ? gfx::TextureFilterType::Nearest : gfx::TextureFilterType::Linear;

                bucket.texture2d->setSamplerConfiguration(
                    {filter, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});

                builder->setTexture(bucket.texture2d, idRasterImage0Texture);
                builder->setTexture(bucket.texture2d, idRasterImage1Texture);
            }
        }
    };

    gfx::VertexAttributeArrayPtr staticAttrs;

    // Build vertex attributes and apply them to a drawable or a builder.
    // Populates a drawable xor a drawable builder for creates and updates, respectively.
    // Returns false if the drawable must be re-created.
    const auto buildVertexData =
        [&](const gfx::UniqueDrawableBuilder& builder, gfx::Drawable* drawable, const RasterBucket& bucket) {
            // The bucket may later add, remove, or change masking.  In that case, the tile's
            // shared data and segments are not updated, and it needs to be re-created.
            if (drawable && (bucket.sharedVertices->getDirty() || bucket.sharedTriangles->getDirty())) {
                return false;
            }

            // The bucket only fills in geometry for masked tiles,
            // otherwise the standard tile extent geometry should be used.
            const bool shared = (!bucket.sharedVertices->empty() && !bucket.sharedTriangles->empty() &&
                                 !bucket.segments.empty());
            const auto& vertices = shared ? bucket.sharedVertices : staticDataVertices;
            const auto& indices = shared ? bucket.sharedTriangles : staticDataIndices;
            const auto* segments = shared ? &bucket.segments : staticDataSegments.get();

            gfx::VertexAttributeArrayPtr bucketAttrs;
            auto& vertexAttrs = shared ? bucketAttrs : staticAttrs;
            if (!vertexAttrs) {
                vertexAttrs = context.createVertexAttributeArray();

                if (auto& attr = vertexAttrs->add(idPosAttribName)) {
                    attr->setSharedRawData(vertices,
                                           offsetof(RasterLayoutVertex, a1),
                                           /*vertexOffset=*/0,
                                           sizeof(RasterLayoutVertex),
                                           gfx::AttributeDataType::Short2);
                }

                if (auto& attr = vertexAttrs->add(idTexturePosAttribName)) {
                    attr->setSharedRawData(vertices,
                                           offsetof(RasterLayoutVertex, a2),
                                           /*vertexOffset=*/0,
                                           sizeof(RasterLayoutVertex),
                                           gfx::AttributeDataType::Short2);
                }
            }

            assert(!!drawable ^ !!builder);
            if (drawable) {
                drawable->setVertexAttributes(vertexAttrs);
            } else if (builder) {
                builder->setVertexAttributes(vertexAttrs);
                builder->setRawVertices({}, vertices->elements(), gfx::AttributeDataType::Short2);
                builder->setSegments(gfx::Triangles(), indices, segments->data(), segments->size());
            }
            return true;
        };

    gfx::UniqueDrawableBuilder builder;
    if (imageData) {
        // TODO: Can we avoid rebuilding drawables each time in this case as well?
        if (imageLayerGroup) {
            stats.drawablesRemoved += imageLayerGroup->clearDrawables();
        }

        RasterBucket& bucket = *imageData->bucket;
        if (!bucket.vertices.empty()) {
            if (!imageLayerGroup) {
                // Set up a layer group
                imageLayerGroup = context.createLayerGroup(layerIndex, /*initialCapacity=*/64, getID());
                imageLayerGroup->addLayerTweaker(layerTweaker);
                activateLayerGroup(imageLayerGroup, isRenderable, changes);
            }

            // Create a drawable for each transformation
            // TODO: Share textures
            builder = createBuilder();
            for (const auto& matrix_ : imageData->matrices) {
                buildVertexData(builder, /*drawable=*/nullptr, bucket);
                setTextures(builder, bucket);

                // finish
                builder->flush(context);

                for (auto& drawable : builder->clearDrawables()) {
                    drawable->setData(std::make_unique<gfx::ImageDrawableData>(matrix_));
                    drawable->setLayerTweaker(layerTweaker);
                    imageLayerGroup->addDrawable(std::move(drawable));
                    ++stats.drawablesAdded;
                }
            }
        }
    } else if (renderTiles) {
        // Remove existing drawables that are no longer in the cover set
        if (layerGroup) {
            stats.drawablesRemoved += removeLayerGroupDrawablesIf(*layerGroup, [&](gfx::Drawable& drawable) {
                return drawable.getTileID() && !hasRenderTile(*drawable.getTileID());
            });
        } else {
            // Set up a tile layer group
            if (auto layerGroup_ = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64, getID())) {
                layerGroup_->addLayerTweaker(layerTweaker);
                setLayerGroup(std::move(layerGroup_), changes);
            }
        }

        auto* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());

        for (const RenderTile& tile : *renderTiles) {
            const auto& tileID = tile.getOverscaledTileID();

            auto* bucket_ = tile.getBucket(*baseImpl);
            if (!bucket_ || !bucket_->hasData()) {
                removeTile(renderPass, tileID);
                continue;
            }

            bool cleared = false;
            auto& bucket = static_cast<RasterBucket&>(*bucket_);

            // If the bucket data has changed, rebuild the drawables.
            const bool bucketSharedData = (!bucket.vertices.empty() && !bucket.indices.empty() &&
                                           !bucket.segments.empty());
            const bool bucketDirty = bucketSharedData && (bucket.vertices.getDirty() || bucket.indices.getDirty());
            if (bucketDirty) {
                // Note, vertex/index dirty flags will be reset when those buffers are uploaded.
                removeTile(renderPass, tileID);
                cleared = true;
            } else if (setRenderTileBucketID(tileID, bucket.getID())) {
                // Bucket ID changed, we need to rebuild the drawables
                removeTile(renderPass, tileID);
                cleared = true;
            }

            if (!cleared) {
                // Update existing drawables
                bool geometryChanged = false;
                auto updateExisting = [&](gfx::Drawable& drawable) {
                    // Only current drawables are updated, ones produced for
                    // a previous style retain the attribute values for that style.
                    if (drawable.getLayerTweaker() != layerTweaker) {
                        return false;
                    }

                    gfx::UniqueDrawableBuilder none;
                    if (!geometryChanged && !buildVertexData(none, &drawable, bucket)) {
                        // Masking changed, need to rebuild this tile
                        geometryChanged = true;
                    }
                    return true;
                };
                // If we update existing drawables, don't build new ones.
                // But if the geometry has changed, we need to drop and re-build them anyway.
                if (updateTile(renderPass, tileID, std::move(updateExisting)) && !geometryChanged) {
                    continue;
                } else if (geometryChanged) {
                    removeTile(renderPass, tileID);
                }
            }

            // Otherwise, create new ones.
            if (!builder) {
                builder = createBuilder();
            }

            if (bucket.image && !builder->getTexture(idRasterImage0Texture) &&
                !builder->getTexture(idRasterImage1Texture)) {
                setTextures(builder, bucket);
            };

            buildVertexData(builder, /*drawable=*/nullptr, bucket);

            // finish
            builder->flush(context);
            for (auto& drawable : builder->clearDrawables()) {
                drawable->setTileID(tileID);
                drawable->setLayerTweaker(layerTweaker);
                tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                ++stats.drawablesAdded;
            }
        }
    }
}
#endif // MLN_DRAWABLE_RENDERER

} // namespace mbgl
