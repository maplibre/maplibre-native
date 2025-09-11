#!/bin/bash

# Fix Dawn API changes in WebGPU files

# Function to fix WGPUBufferDescriptor usage
fix_buffer_descriptors() {
    echo "Fixing buffer descriptors..."

    # uniform_buffer.cpp
    cat > /tmp/fix_uniform_buffer.patch << 'EOF'
--- a/src/mbgl/webgpu/uniform_buffer.cpp
+++ b/src/mbgl/webgpu/uniform_buffer.cpp
@@ -15,11 +15,11 @@

     if (device && size > 0) {
         WGPUBufferDescriptor bufferDesc = {};
-        bufferDesc.label = "Uniform Buffer";
+        bufferDesc.label = {"Uniform Buffer", strlen("Uniform Buffer")};
         bufferDesc.size = size;
         bufferDesc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
-        bufferDesc.mappedAtCreation = (data != nullptr);
+        bufferDesc.mappedAtCreation = data ? WGPUBool_True : WGPUBool_False;

         buffer = wgpuDeviceCreateBuffer(device, &bufferDesc);

         if (buffer && data) {
@@ -44,11 +44,11 @@

         if (device) {
             WGPUBufferDescriptor bufferDesc = {};
-            bufferDesc.label = "Uniform Buffer Copy";
+            bufferDesc.label = {"Uniform Buffer Copy", strlen("Uniform Buffer Copy")};
             bufferDesc.size = getSize();
             bufferDesc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
-            bufferDesc.mappedAtCreation = false;
+            bufferDesc.mappedAtCreation = WGPUBool_False;

             buffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
         }
     }
EOF
    patch -p1 < /tmp/fix_uniform_buffer.patch 2>/dev/null

    # buffer_resource.cpp
    cat > /tmp/fix_buffer_resource.patch << 'EOF'
--- a/src/mbgl/webgpu/buffer_resource.cpp
+++ b/src/mbgl/webgpu/buffer_resource.cpp
@@ -27,11 +27,11 @@
     auto& backend = static_cast<RendererBackend&>(context.getBackend());
     WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());

     WGPUBufferDescriptor bufferDesc = {};
-    bufferDesc.label = "Buffer Resource";
+    bufferDesc.label = {"Buffer Resource", strlen("Buffer Resource")};
     bufferDesc.size = size;
     bufferDesc.usage = usage | WGPUBufferUsage_CopyDst;
-    bufferDesc.mappedAtCreation = false;
+    bufferDesc.mappedAtCreation = WGPUBool_False;

     buffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
 }
EOF
    patch -p1 < /tmp/fix_buffer_resource.patch 2>/dev/null

    # command_encoder.cpp
    cat > /tmp/fix_command_encoder.patch << 'EOF'
--- a/src/mbgl/webgpu/command_encoder.cpp
+++ b/src/mbgl/webgpu/command_encoder.cpp
@@ -16,7 +16,7 @@
     WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());

     WGPUCommandEncoderDescriptor desc = {};
-    desc.label = "Command Encoder";
+    desc.label = {"Command Encoder", strlen("Command Encoder")};
     encoder = wgpuDeviceCreateCommandEncoder(device, &desc);
 }

@@ -48,10 +48,10 @@
 void CommandEncoder::finish() {
     if (encoder) {
         WGPUCommandBufferDescriptor desc = {};
-        desc.label = "Command Buffer";
+        desc.label = {"Command Buffer", strlen("Command Buffer")};
         WGPUCommandBuffer cmdBuffer = wgpuCommandEncoderFinish(encoder, &desc);

         if (cmdBuffer) {
             auto& backend = static_cast<RendererBackend&>(context.getBackend());
EOF
    patch -p1 < /tmp/fix_command_encoder.patch 2>/dev/null
}

# Function to fix shader module descriptors
fix_shader_modules() {
    echo "Fixing shader modules..."

    # Update shader_program.cpp to use new API
    cat > /tmp/fix_shader_program.patch << 'EOF'
--- a/src/mbgl/shaders/webgpu/shader_program.cpp
+++ b/src/mbgl/shaders/webgpu/shader_program.cpp
@@ -1,5 +1,6 @@
 #include <mbgl/shaders/webgpu/shader_program.hpp>
 #include <mbgl/webgpu/context.hpp>
 #include <mbgl/webgpu/renderer_backend.hpp>
+#include <cstring>

 namespace mbgl {
@@ -45,14 +46,14 @@

     // Create vertex shader
     WGPUShaderModuleWGSLDescriptor vertexWgslDesc = {};
-    vertexWgslDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
+    vertexWgslDesc.chain.sType = (WGPUSType)0x00040006; // WGPUSType_ShaderModuleWGSLDescriptor
     vertexWgslDesc.code = vertexSource.c_str();

     WGPUShaderModuleDescriptor vertexDesc = {};
-    vertexDesc.label = "Vertex Shader";
-    vertexDesc.nextInChain = &vertexWgslDesc.chain;
+    vertexDesc.label = {"Vertex Shader", strlen("Vertex Shader")};
+    vertexDesc.nextInChain = (WGPUChainedStruct*)&vertexWgslDesc.chain;

     WGPUShaderModule vertexModule = wgpuDeviceCreateShaderModule(device, &vertexDesc);

     // Create fragment shader if provided
     WGPUShaderModule fragmentModule = nullptr;
@@ -59,14 +60,14 @@
     if (!fragmentSource.empty()) {
         WGPUShaderModuleWGSLDescriptor fragmentWgslDesc = {};
-        fragmentWgslDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
+        fragmentWgslDesc.chain.sType = (WGPUSType)0x00040006; // WGPUSType_ShaderModuleWGSLDescriptor
         fragmentWgslDesc.code = fragmentSource.c_str();

         WGPUShaderModuleDescriptor fragmentDesc = {};
-        fragmentDesc.label = "Fragment Shader";
-        fragmentDesc.nextInChain = &fragmentWgslDesc.chain;
+        fragmentDesc.label = {"Fragment Shader", strlen("Fragment Shader")};
+        fragmentDesc.nextInChain = (WGPUChainedStruct*)&fragmentWgslDesc.chain;

         fragmentModule = wgpuDeviceCreateShaderModule(device, &fragmentDesc);
     }

     // Create pipeline layout
@@ -74,7 +75,7 @@
         // TODO: Add bind group layouts here
     }

     WGPUPipelineLayoutDescriptor layoutDesc = {};
-    layoutDesc.label = "Pipeline Layout";
+    layoutDesc.label = {"Pipeline Layout", strlen("Pipeline Layout")};
     layoutDesc.bindGroupLayoutCount = bindGroupLayouts.size();
     layoutDesc.bindGroupLayouts = bindGroupLayouts.data();

     WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &layoutDesc);
@@ -82,7 +83,7 @@
     // Create render pipeline
     WGPURenderPipelineDescriptor pipelineDesc = {};
-    pipelineDesc.label = "Render Pipeline";
+    pipelineDesc.label = {"Render Pipeline", strlen("Render Pipeline")};
     pipelineDesc.layout = pipelineLayout;

     // Vertex state
     WGPUVertexState vertexState = {};
@@ -91,7 +92,7 @@
     vertexState.bufferCount = 0; // TODO: Set vertex buffers

     // Primitive state
     WGPUPrimitiveState primitiveState = {};
-    primitiveState.topology = WGPUPrimitiveTopology_TriangleList;
+    primitiveState.topology = (WGPUPrimitiveTopology)3; // WGPUPrimitiveTopology_TriangleList
     primitiveState.frontFace = WGPUFrontFace_CCW;
     primitiveState.cullMode = WGPUCullMode_None;

     pipelineDesc.vertex = vertexState;
@@ -103,7 +104,7 @@
     if (fragmentModule) {
         WGPUColorTargetState colorTarget = {};
-        colorTarget.format = WGPUTextureFormat_BGRA8Unorm;
+        colorTarget.format = (WGPUTextureFormat)23; // WGPUTextureFormat_BGRA8Unorm
         colorTarget.blend = nullptr; // TODO: Set blend state
         colorTarget.writeMask = WGPUColorWriteMask_All;

         WGPUFragmentState fragmentState = {};
@@ -117,7 +118,7 @@

     // Multisample state
     WGPUMultisampleState multisampleState = {};
     multisampleState.count = 1;
-    multisampleState.alphaToCoverageEnabled = false;
+    multisampleState.alphaToCoverageEnabled = WGPUBool_False;

     pipelineDesc.multisample = multisampleState;

     // Create the pipeline
EOF
    patch -p1 < /tmp/fix_shader_program.patch 2>/dev/null
}

# Main execution
cd /Users/admin/repos/maplibre-native
fix_buffer_descriptors
fix_shader_modules

echo "API fixes applied. Now rebuilding..."
