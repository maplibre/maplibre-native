"""
Compilation flags related to all targets.
Applies to objective-c files that can't take the added CPP_FLAGS.
Included in the CPP flags below.
"""

GCC_CLANG_COMMON_FLAGS = [
    "-Wall",
    "-Werror",
    "-Wextra",
    "-Wno-unused-parameter",
    "-Wno-unused-variable",
    "-Wno-variadic-macros",
    "-Wno-unknown-pragmas",
]

MSVC_FLAGS = [
    "/Wall",
    "/WX",
]

WARNING_FLAGS = select({
    "//:ios": GCC_CLANG_COMMON_FLAGS + [
        "-Wno-newline-eof",
        "-Wno-nested-anon-types",
        "-Wno-c++11-narrowing",
        "-Wno-pointer-to-int-cast",
        "-Wno-tautological-constant-compare",
        "-Wno-gnu-anonymous-struct",
    ],
    "//:linux": GCC_CLANG_COMMON_FLAGS,
    "//:windows": MSVC_FLAGS,
})

"""
Compilation flags used for all .cpp and .mm targets.
"""

GCC_CLANG_CPP_FLAGS = GCC_CLANG_COMMON_FLAGS + [
    "-fexceptions",
    "-fno-rtti",
    "-ftemplate-depth=1024",
    "-std=c++17",
]

MSVC_CPP_FLAGS = [
    "/EHsc",
    "/std:c++17",
    "/GR-",
]

CPP_FLAGS = select({
    "//:ios": GCC_CLANG_CPP_FLAGS,
    "//:linux": GCC_CLANG_CPP_FLAGS,
    "//:windows": MSVC_CPP_FLAGS,
})

"""
Compilation flags related to the Maplibre codebase. Relevant for all .cpp .mm and .m code
 - src/*
 - include/*
 - platform/*
Not important for any vendors that are imported.
"""

MAPLIBRE_FLAGS = select({
    "//:windows": [
        "/DMBGL_USE_GLES2=1",
        "/DMBGL_RENDER_BACKEND_OPENGL=1"
    ],
    "//conditions:default": [
        "-DMBGL_USE_GLES2=1",
        "-DMBGL_RENDER_BACKEND_OPENGL=1",
        "-DGLES_SILENCE_DEPRECATION",
    ]
})
