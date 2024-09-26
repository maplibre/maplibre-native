val nativeBuild: (List<String>) -> Unit = { nativeTargets ->
    android {
        ndkVersion = androidSdkVersions.ndkVersion

        // Determine which ABIs to build based on project properties
        var abi = "all"
        if (!project.hasProperty("android.injected.invoked.from.ide") && project.hasProperty("maplibre.abis")) {
            abi = project.property("maplibre.abis") as String
        }

        if (abi != "none") {
            externalNativeBuild {
                cmake {
                    path = "../MapLibreAndroid/src/cpp/CMakeLists.txt"
                    version = androidSdkVersions.cmakeVersion
                }
            }
        }

        // Determine the C++ STL being used
        var stl = "c++_static"
        if (project.hasProperty("mapbox.stl")) {
            stl = project.property("mapbox.stl") as String
        }

        defaultConfig {
            if (abi != "none") {
                externalNativeBuild {
                    cmake {
                        arguments("-DANDROID_TOOLCHAIN=clang")
                        arguments("-DANDROID_STL=$stl")
                        arguments("-DANDROID_CPP_FEATURES=exceptions")
                        arguments("-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON")

                        // Enable ccache if installed
                        if (file("/usr/bin/ccache").exists()) {
                            arguments("-DANDROID_CCACHE=/usr/bin/ccache")
                        } else if (file("/usr/local/bin/ccache").exists()) {
                            arguments("-DANDROID_CCACHE=/usr/local/bin/ccache")
                        }

                        cFlags("-Qunused-arguments")
                        cppFlags("-Qunused-arguments")

                        for (target in nativeTargets) {
                            targets(target)
                        }

                        if (abi != "all") {
                            abiFilters(abi.split(" "))
                        } else {
                            if (project.hasProperty("android.injected.invoked.from.ide") && project.hasProperty("android.injected.build.abi")) {
                                // Only build the target ABI when running from Android Studio
                                abi = project.property("android.injected.build.abi") as String
                                abiFilters(abi.split(",").first())
                            } else {
                                abiFilters("armeabi-v7a", "x86", "arm64-v8a", "x86_64")
                            }
                        }
                    }
                }
            }
        }
    }
}
