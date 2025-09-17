#include <mbgl/webgpu/tile_layer_group.hpp>

#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/drawable.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

TileLayerGroup::TileLayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : mbgl::TileLayerGroup(layerIndex_, initialCapacity, std::move(name_)) {
}

void TileLayerGroup::upload(gfx::UploadPass& uploadPass) {
    if (!enabled || !getDrawableCount()) {
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroup = uploadPass.createDebugGroup(getName() + "-upload");
#endif

    visitDrawables([&](gfx::Drawable& drawable) {
        if (drawable.getEnabled()) {
            auto& drawableWebGPU = static_cast<Drawable&>(drawable);
            drawableWebGPU.upload(uploadPass);
        }
    });
}

void TileLayerGroup::render(RenderOrchestrator&, PaintParameters& parameters) {
    if (!enabled || !getDrawableCount() || !parameters.renderPass) {
        return;
    }

    auto& renderPass = static_cast<RenderPass&>(*parameters.renderPass);

    // TODO: Handle 3D features and stencil clipping when needed
    // For now, WebGPU handles stenciling through the render pipeline state

#if !defined(NDEBUG)
    const auto debugGroup = parameters.encoder->createDebugGroup(getName() + "-render");
#endif

    // Handle stencil tiles if present (2D stenciling)
    if (stencilTiles && !stencilTiles->empty()) {
        parameters.renderTileClippingMasks(stencilTiles);
    }

    // Get shader for this layer group if needed
    gfx::ShaderProgramBasePtr layerShader = nullptr;
    if (getDrawableCount() > 0) {
        // Determine shader type based on layer name
        // This is a temporary solution - ideally shaders should be set when drawables are created
        std::string shaderGroupName;

        // Determine shader group name based on layer name
        const std::string& layerName = getName();
        if (layerName.find("fill") != std::string::npos || layerName.find("Fill") != std::string::npos) {
            if (layerName.find("outline") != std::string::npos || layerName.find("Outline") != std::string::npos) {
                if (layerName.find("pattern") != std::string::npos || layerName.find("Pattern") != std::string::npos) {
                    shaderGroupName = "FillOutlinePatternShader";
                } else if (layerName.find("triangulated") != std::string::npos) {
                    shaderGroupName = "FillOutlineTriangulatedShader";
                } else {
                    shaderGroupName = "FillOutlineShader";
                }
            } else if (layerName.find("pattern") != std::string::npos || layerName.find("Pattern") != std::string::npos) {
                shaderGroupName = "FillPatternShader";
            } else if (layerName.find("extrusion") != std::string::npos || layerName.find("Extrusion") != std::string::npos) {
                if (layerName.find("pattern") != std::string::npos || layerName.find("Pattern") != std::string::npos) {
                    shaderGroupName = "FillExtrusionPatternShader";
                } else {
                    shaderGroupName = "FillExtrusionShader";
                }
            } else {
                shaderGroupName = "FillShader";
            }
        } else if (layerName.find("line") != std::string::npos || layerName.find("Line") != std::string::npos) {
            if (layerName.find("gradient") != std::string::npos || layerName.find("Gradient") != std::string::npos) {
                shaderGroupName = "LineGradientShader";
            } else if (layerName.find("pattern") != std::string::npos || layerName.find("Pattern") != std::string::npos) {
                shaderGroupName = "LinePatternShader";
            } else if (layerName.find("sdf") != std::string::npos || layerName.find("SDF") != std::string::npos) {
                shaderGroupName = "LineSDFShader";
            } else {
                shaderGroupName = "LineShader";
            }
        } else if (layerName.find("circle") != std::string::npos || layerName.find("Circle") != std::string::npos) {
            shaderGroupName = "CircleShader";
        } else if (layerName.find("symbol") != std::string::npos || layerName.find("Symbol") != std::string::npos) {
            if (layerName.find("icon") != std::string::npos || layerName.find("Icon") != std::string::npos) {
                if (layerName.find("text") != std::string::npos || layerName.find("Text") != std::string::npos) {
                    shaderGroupName = "SymbolTextAndIconShader";
                } else {
                    shaderGroupName = "SymbolIconShader";
                }
            } else if (layerName.find("sdf") != std::string::npos || layerName.find("SDF") != std::string::npos) {
                shaderGroupName = "SymbolSDFShader";
            } else {
                shaderGroupName = "SymbolSDFShader"; // Default for symbols
            }
        } else if (layerName.find("background") != std::string::npos || layerName.find("Background") != std::string::npos) {
            if (layerName.find("pattern") != std::string::npos || layerName.find("Pattern") != std::string::npos) {
                shaderGroupName = "BackgroundPatternShader";
            } else {
                shaderGroupName = "BackgroundShader";
            }
        }

        // Get the shader group from the registry
        if (!shaderGroupName.empty()) {
            auto shaderGroup = parameters.shaders.getShaderGroup(shaderGroupName);
            if (shaderGroup) {
                // Now get or create the shader from the group
                // The ShaderGroup's getOrCreateShader is likely a virtual method
                // For now, we'll use getShader() which gets the default shader from the group
                StringIDSetsPair propertiesAsUniforms;  // Empty for now
                auto groupPtr = shaderGroup.get();
                if (groupPtr) {
                    // Call the virtual getOrCreateShader method
                    auto shader = groupPtr->getOrCreateShader(
                        parameters.context,
                        propertiesAsUniforms,
                        ""  // No first attribute name override
                    );
                    // Cast from ShaderPtr to ShaderProgramBasePtr
                    // This assumes that the shader returned is actually a ShaderProgramBase
                    if (shader) {
                        layerShader = std::static_pointer_cast<gfx::ShaderProgramBase>(shader);
                    }
                    if (layerShader) {
                        mbgl::Log::Info(mbgl::Event::Render, "Got shader from group " + shaderGroupName +
                                       " for layer group " + getName());
                    } else {
                        mbgl::Log::Warning(mbgl::Event::Render, "Failed to get shader from group " + shaderGroupName +
                                         " for layer group " + getName());
                    }
                }
            } else {
                mbgl::Log::Warning(mbgl::Event::Render, "No shader group found: " + shaderGroupName);
            }
        }
    }

    bool bindUBOs = false;
    int visitCount = 0;
    int drawCount = 0;
    mbgl::Log::Info(mbgl::Event::Render, "TileLayerGroup::render starting visitDrawables for " + getName());
    visitDrawables([&](gfx::Drawable& drawable) {
        visitCount++;
        mbgl::Log::Info(mbgl::Event::Render, "  Visiting drawable " + std::to_string(visitCount) +
                       " enabled=" + std::to_string(drawable.getEnabled()) +
                       " hasRenderPass=" + std::to_string(drawable.hasRenderPass(parameters.pass)));
        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }
        drawCount++;

        // Assign shader to drawable if it doesn't have one
        if (!drawable.getShader() && layerShader) {
            drawable.setShader(layerShader);
        }

        if (!bindUBOs) {
            // Bind uniform buffers once for all drawables
            static_cast<webgpu::UniformBufferArray&>(uniformBuffers).bindWebgpu(renderPass);
            bindUBOs = true;
        }

        // Copy shared uniform buffers from layer group to drawable before tweakers
        // These are typically global paint params and evaluated props
        auto& drawableWebGPU = static_cast<webgpu::Drawable&>(drawable);

        // Copy any layer-provided uniform buffers so drawables can build bind groups from them.
        // This includes consolidated per-drawable data (e.g. idFillDrawableUBO) which lives in
        // the layer UBO array and is selected via the UBO index each drawable writes later.
        auto& drawableUniforms = drawableWebGPU.mutableUniformBuffers();
        for (size_t i = 0; i < uniformBuffers.allocatedSize(); ++i) {
            const auto& uniformBuffer = uniformBuffers.get(i);
            if (uniformBuffer) {
                drawableUniforms.set(i, uniformBuffer);
            }
        }

        // Execute tweakers - they create drawable-specific uniform buffers like FillDrawableUBO
        for (const auto& tweaker : drawable.getTweakers()) {
            tweaker->execute(drawable, parameters);
        }

        // Log uniform buffer status after tweakers
        if (drawCount <= 3) {
            for (size_t i = 0; i < 4; ++i) {
                const auto& uniformBuffer = drawableUniforms.get(i);
                if (uniformBuffer) {
                    mbgl::Log::Info(mbgl::Event::Render, getName() + " drawable " + std::to_string(drawCount) +
                                   " has UBO[" + std::to_string(i) + "] size=" + std::to_string(uniformBuffer->getSize()));
                }
            }
        }

        drawable.draw(parameters);
    });

    mbgl::Log::Info(mbgl::Event::Render, "TileLayerGroup::render finished for " + getName() +
                   " visited=" + std::to_string(visitCount) + " drawn=" + std::to_string(drawCount));

    if (visitCount > 0 && drawCount == 0) {
        mbgl::Log::Warning(mbgl::Event::Render, getName() + " visited " + std::to_string(visitCount) +
                          " drawables but drew none!");
    }
}

} // namespace webgpu
} // namespace mbgl
