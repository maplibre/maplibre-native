import com.android.build.gradle.BaseExtension
import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.kotlin.dsl.getByType
import java.io.File

open class NativeBuildPlugin : Plugin<Project> {
    override fun apply(project: Project) {
        val extension = project.extensions.create("nativeBuild", NativeBuildExtension::class.java)
        project.afterEvaluate {
            val targets = extension.nativeTargets
            project.nativeBuild(nativeTargets = targets)
        }
    }
}

open class NativeBuildExtension {
    var nativeTargets: List<String> = listOf()
}

fun Project.nativeBuild(nativeTargets: List<String>) =
    this.extensions.getByType<BaseExtension>().run {

        // We sometimes want to invoke Gradle without building a native dependency, e.g. when we just want
        // to invoke the Java tests. When we explicitly specify an ABI of 'none', no native dependencies are
        // added. When another ABI is specified explicitly, we're just going to build that ABI. In all other
        // cases, all ABIs are built.
        //
        // When invoking from the command line or to override the device default, set `-Pmaplibre.abis=...` to
        // only build the desired architectures.
        //
        // When building from Android Studio, gradle.properties sets `android.buildOnlyTargetAbi=true` so that
        // only the architecture for the device you're running on gets built.


        var abi = "all"
        if (!project.hasProperty("android.injected.invoked.from.ide") && project.hasProperty("maplibre.abis")) {
            abi = project.property("maplibre.abis") as String
        }

        if (abi != "none") {
            externalNativeBuild {
                cmake {
                    path = file("../MapLibreAndroid/src/cpp/CMakeLists.txt")
                    version = Versions.cmakeVersion
                }
            }
        }

        // Determine the C++ STL being used.
        var stl = "c++_static"
        if (project.hasProperty("mapbox.stl")) {
            stl = project.property("mapbox.stl") as String
        }

        defaultConfig {
            if (abi != "none") {
                externalNativeBuild {
                    cmake {
                        arguments.addAll(
                            listOf(
                                "-DANDROID_TOOLCHAIN=clang",
                                "-DANDROID_STL=$stl",
                                "-DANDROID_CPP_FEATURES=exceptions",
                                "-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON"
                            )
                        )

                        cFlags.add("-Qunused-arguments")
                        cppFlags.add("-Qunused-arguments")

                        for (target in nativeTargets) {
                            targets(target)
                        }

                        if (abi != "all") {
                            abiFilters.addAll(abi.split(" "))
                        } else {
                            if (project.hasProperty("android.injected.invoked.from.ide") && project.hasProperty(
                                    "android.injected.build.abi"
                                )
                            ) {
                                abi = project.property("android.injected.build.abi") as String
                                abiFilters.add(abi.split(",").first())
                            } else {
                                abiFilters.addAll(
                                    listOf(
                                        "armeabi-v7a",
                                        "x86",
                                        "arm64-v8a",
                                        "x86_64"
                                    )
                                )
                            }
                        }
                    }
                }
            }
        }
    }
