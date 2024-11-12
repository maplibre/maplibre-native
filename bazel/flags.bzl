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
    "-Wno-unknown-warning-option",
    "-Wno-psabi",
    "-Wno-pragmas",
]

MSVC_FLAGS = [
    "/Wall",
    "/WX",
    "/wd4068",  # Unknown pragma
    "/wd4820",  # Padding
    "/wd5045",  # Compiler will insert Spectre mitigation for memory load
    "/wd4643",  # Non-conformant forward decl
    "/wd4623",  # Default ctor implicitly deleted
    "/wd5027",  # Default move ctor implicitly deleted
    "/wd4626",  # Assignment operator implicitly deleted
    "/wd4514",  # Unreferenced inline function has been removed
    "/wd4582",  # Constructor is not implicitly called
    "/wd4365",  # Signed/unsigned mismatch
    "/wd4388",  # Signed/unsigned mismatch
    "/wd4244",  # Conversion, possible loss of data
    "/wd4242",  # Conversion, possible loss of data
    "/wd4625",  # Copy constructor was implicitly defined as deleted
    "/wd5026",  # Move constructor was implicitly defined as deleted
    "/wd5219",  # Implicit conversion, possible loss of data
    "/wd5246",  # Initialization of a subobject should be wrapped in braces
    "/wd4201",  # Nonstandard extension used: nameless struct/union
    "/wd4868",  # Compiler may not enforce left-to-right evaluation order in braced initializer list
    "/wd4061",  # Eenumerator in switch of enum is not explicitly handled by a case label
    "/wd4464",  # Relative include path contains '..'
    "/wd4668",  # <x> is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
    "/wd4800",  # Implicit conversion from 'unsigned __int64' to bool. Possible information loss
    "/wd4355",  # 'this': used in base member initializer list
    "/wd5204",  # Class has virtual functions, but its trivial destructor is not virtual; instances of objects derived from this class may not be destructed
    "/wd5220",  # A non-static data member with a volatile qualified type no longer implies
    "/wd4619",  # (boost) #pragma warning: there is no warning number <x>
    "/wd5031",  # (boost) #pragma warning(pop): likely mismatch, popping warning state pushed in different file
    "/wd5243",  # (boost) using incomplete class <x> can cause potential one definition rule violation due to ABI limitation
    "/wd4371",  # (boost) layout of class may have changed from a previous version of the compiler due to better packing of member <x>
    "/wd4435",  # (boost) Object layout under /vd2 will change due to virtual base
    "/wd4702",  # Unreachable code
    "/wd4710",  # Function not inlined
    "/wd5041",  # out-of-line definition for constexpr static data member is not needed and is deprecated since C++17
    "/wd4946",  # reinterpret_cast used between related classes
    "/wd4459",  # declaration of 'x' hides global declaration
    "/wd4373",  # virtual function overrides 'x', previous versions of the compiler did not override when parameters only differed by const/volatile qualifiers
]

WARNING_FLAGS = {
    "ios": [
        "-Wno-newline-eof",
        "-Wno-nested-anon-types",
        "-Wno-c++11-narrowing",
        "-Wno-pointer-to-int-cast",
        "-Wno-tautological-constant-compare",
        "-Wno-gnu-anonymous-struct",
    ] + GCC_CLANG_COMMON_FLAGS,
    "macos": [
        "-Wno-newline-eof",
        "-Wno-nested-anon-types",
        "-Wno-c++11-narrowing",
        "-Wno-pointer-to-int-cast",
        "-Wno-tautological-constant-compare",
        "-Wno-gnu-anonymous-struct",
    ] + GCC_CLANG_COMMON_FLAGS,
    "linux": GCC_CLANG_COMMON_FLAGS,
    "windows": MSVC_FLAGS,
}

# Compilation flags used for all .cpp and .mm targets.

GCC_CLANG_CPP_FLAGS = [
    "-fexceptions",
    "-fno-rtti",
    "-ftemplate-depth=1024",
    "-std=c++20",
]

MSVC_CPP_FLAGS = [
    "/EHsc",
    "/std:c++20",
    "/GR-",
    "/permissive-",
    "/utf-8",
]

CPP_FLAGS = select({
    "//conditions:default": GCC_CLANG_CPP_FLAGS,
    "@platforms//os:ios": GCC_CLANG_CPP_FLAGS + WARNING_FLAGS["ios"] + ["-fvisibility=hidden"],
    "@platforms//os:macos": GCC_CLANG_CPP_FLAGS + WARNING_FLAGS["macos"] + ["-fvisibility=hidden"],
    "@platforms//os:linux": GCC_CLANG_CPP_FLAGS + WARNING_FLAGS["linux"],
    "@platforms//os:windows": MSVC_CPP_FLAGS + WARNING_FLAGS["windows"],
})

# Compilation flags related to the Maplibre codebase. Relevant for all .cpp .mm and .m code
#  - src/*
#  - include/*
#  - platform/*
# Not important for any vendors that are imported.

MAPLIBRE_FLAGS = select({
    "@platforms//os:windows": [
        "/DMBGL_USE_GLES2=1",
        "/DMBGL_RENDER_BACKEND_OPENGL=1",
        "/D_USE_MATH_DEFINES",
        "/DMLN_USE_UNORDERED_DENSE",
    ],
    "//conditions:default": [
        "-DMBGL_USE_GLES2=1",
        "-DMBGL_RENDER_BACKEND_OPENGL=1",
        "-DGLES_SILENCE_DEPRECATION",
        "-DMLN_USE_UNORDERED_DENSE",
    ],
})
