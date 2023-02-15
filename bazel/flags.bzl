"""
Compilation flags related to all targets.
Applies to objective-c files that can't take the added CPP_FLAGS.
Included in the CPP flags below.
"""

WARNING_FLAGS = [
    "-Wall",
    "-Werror",
    "-Wextra",
    "-Wno-c++11-narrowing",
    "-Wno-gnu-anonymous-struct",
    "-Wno-pointer-to-int-cast",
    "-Wno-tautological-constant-compare",
    "-Wno-unused-parameter",
    "-Wno-unused-variable",
    "-Wno-variadic-macros",
    "-Wno-nested-anon-types",
    "-Wno-newline-eof",
]

"""
Compilation flags used for all .cpp and .mm targets.
"""

CPP_FLAGS = WARNING_FLAGS + [
    "-fexceptions",
    "-frtti",
    "-ftemplate-depth=1024",
    "-std=c++17",
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
