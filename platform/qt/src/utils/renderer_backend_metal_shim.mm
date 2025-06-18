// Copyright (C) 2024 MapLibre contributors
// SPDX-License-Identifier: BSD-2-Clause
//
// Thin Objective-C++ translation unit that provides the three linker symbols
// MapRenderer expects from QMapLibre::RendererBackend when we build the
// Metal backend only.

#if defined(MLN_RENDER_BACKEND_METAL) && !defined(MLN_RENDER_BACKEND_OPENGL)

#include <TargetConditionals.h>
#include <QtCore/QtGlobal>
#include "renderer_backend.hpp"

using namespace QMapLibre;

RendererBackend::RendererBackend(mbgl::gfx::ContextMode mode)
    : MetalRendererBackend(mode) {}

RendererBackend::~RendererBackend() = default;

void RendererBackend::updateFramebuffer(uint32_t /*fbo*/, const mbgl::Size& /*size*/) {}

#endif // Metal-only shim 