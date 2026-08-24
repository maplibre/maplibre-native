load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "tinyobjloader",
    hdrs = ["tiny_obj_loader.h"],
    copts = select({
        "@platforms//os:windows": [],
        "//conditions:default": ["-Wno-maybe-uninitialized"],
    }),
    visibility = ["//visibility:public"],
)
