"""
Compilation flags related to all targets.
Applies to objective-c files that can't take the added CPP_FLAGS.
Included in the CPP flags below.
"""

WARNING_FLAGS = [
    "-Wall",
    "-Werror",
    "-Wextra",
    "-Wno-unused-parameter",
    "-Wno-unused-variable",
    "-Wno-variadic-macros",
] + select({
    "//:ios": [
        "-Wno-newline-eof",
        "-Wno-nested-anon-types",
        "-Wno-c++11-narrowing",
        "-Wno-pointer-to-int-cast",
        "-Wno-tautological-constant-compare",
        "-Wno-gnu-anonymous-struct",
    ],
    "//:linux": [],
})

"""
Compilation flags used for all .cpp and .mm targets.
"""

CPP_FLAGS = WARNING_FLAGS + [
    "-fexceptions",
    "-fno-rtti",
    "-ftemplate-depth=1024",
    "-std=c++17"
]
"""
Compilation flags related to the Maplibre codebase. Relevant for all .cpp .mm and .m code
 - src/*
 - include/*
 - platform/*
Not important for any vendors that are imported.
"""

MAPLIBRE_FLAGS = [
    "-DMBGL_USE_GLES2=1",
    "-DMBGL_RENDER_BACKEND_OPENGL=1",
    "-DGLES_SILENCE_DEPRECATION",
]
