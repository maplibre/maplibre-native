plugins {
    id("com.android.library")
}

val invokedFromIde = project.hasProperty("android.injected.invoked.from.ide")
val mapLibreAbis = if (!invokedFromIde && project.hasProperty("maplibre.abis")) {
    project.property("maplibre.abis") as String
} else {
    "all"
}
val shouldBuildNative = mapLibreAbis != "none"

android {
    namespace = "org.maplibre.maplibrepluginsexamplelibrary"
    compileSdk = 34

    defaultConfig {
        minSdk = 23

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        consumerProguardFiles("consumer-rules.pro")
        if (shouldBuildNative) {
            externalNativeBuild {
                cmake {
                    cppFlags("")
                    if (mapLibreAbis != "all") {
                        abiFilters.addAll(mapLibreAbis.split(" "))
                    } else if (invokedFromIde && project.hasProperty("android.injected.build.abi")) {
                        abiFilters.add((project.property("android.injected.build.abi") as String).split(",").first())
                    } else {
                        abiFilters.addAll(listOf("armeabi-v7a", "x86", "arm64-v8a", "x86_64"))
                    }
                }
            }
        }
        missingDimensionStrategy("renderer", "opengl")
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
            if (shouldBuildNative) {
                externalNativeBuild {
                    cmake {
                        arguments(
                            "-DMAPLIBRE_LIB_DIR=${rootDir}/MapLibreAndroid/build/intermediates/library_jni/openglRelease/copyOpenglReleaseJniLibsProjectOnly/jni"
                        )
                    }
                }
            }
        }
        debug {
            if (shouldBuildNative) {
                externalNativeBuild {
                    cmake {
                        arguments(
                            "-DMAPLIBRE_LIB_DIR=${rootDir}/MapLibreAndroid/build/intermediates/library_jni/openglDebug/copyOpenglDebugJniLibsProjectOnly/jni"
                        )
                    }
                }
            }
        }
    }
    if (shouldBuildNative) {
        externalNativeBuild {
            cmake {
                path("src/main/cpp/CMakeLists.txt")
                version = Versions.cmakeVersion
            }
        }
    }
    packaging {
        jniLibs {
            excludes += "**/libmaplibre.so"
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
    ndkVersion = Versions.ndkVersion
}

dependencies {
    implementation(project(":MapLibreAndroid"))

    testImplementation(libs.junit)
    androidTestImplementation(libs.testRunner)
    androidTestImplementation(libs.testEspressoCore)
}

if (shouldBuildNative) {
    afterEvaluate {
        tasks.matching { it.name.contains("CMakeDebug") || it.name.contains("externalNativeBuildDebug") }.configureEach {
            dependsOn(":MapLibreAndroid:copyOpenglDebugJniLibsProjectOnly")
        }
        tasks.matching { it.name.contains("CMakeRelease") || it.name.contains("externalNativeBuildRelease") }.configureEach {
            dependsOn(":MapLibreAndroid:copyOpenglReleaseJniLibsProjectOnly")
        }
    }
}
