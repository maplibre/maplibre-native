plugins {
    id("com.android.library")
    id("maven-publish")
    id("maplibre.artifact-settings")
}

group = project.extra["mapLibreArtifactGroupId"] as String
version = project.extra["versionName"] as String

android {
    namespace = "org.maplibre.android.plugins.api"
    compileSdk = 34
    ndkVersion = Versions.ndkVersion

    defaultConfig {
        minSdk = 23
        externalNativeBuild {
            cmake {
                arguments("-DANDROID_STL=none")
                targets("plugin_api")
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = Versions.cmakeVersion
        }
    }

    buildFeatures { prefabPublishing = true }
    prefab {
        create("plugin_api") {
            headers = "../prefab-plugin-api-headers"
            libraryName = "libplugin_api"
        }
    }

    publishing {
        singleVariant("release") {
            withSourcesJar()
        }
    }
}

dependencies {
    api(libs.supportAnnotations)
}

val syncPluginApiHeaders by tasks.registering(Sync::class) {
    val nativeRoot = rootProject.rootDir.resolve("../..")
    from(nativeRoot.resolve("include")) {
        include("mbgl/plugin/plugin_api.h")
    }
    into(project.rootDir.resolve("prefab-plugin-api-headers"))
}

tasks.configureEach {
    if (name == "syncPluginApiHeaders") return@configureEach
    if (name.contains("Prefab", ignoreCase = true) || name.contains("bundleLibRuntimeTo", ignoreCase = true)) {
        dependsOn(syncPluginApiHeaders)
    }
}

publishing {
    publications {
        register<MavenPublication>("release") {
            groupId = project.group.toString()
            artifactId = "android-plugin-api"
            version = project.version.toString()
            afterEvaluate { from(components["release"]) }
            pom {
                name.set("MapLibre Android Plugin API")
                description.set("Pure-C native plugin ABI and Android registry contract for MapLibre Native")
                url.set(project.extra["mapLibreArtifactUrl"].toString())
            }
        }
    }
    repositories {
        val target = providers.gradleProperty("reposiliteUrl")
            .orElse(providers.environmentVariable("REPOSILITE_URL"))
        if (target.isPresent) {
            val repositoryUri = uri(target.get())
            val repositoryUsername = providers.gradleProperty("reposiliteUsername")
                .orElse(providers.environmentVariable("REPOSILITE_USERNAME"))
            val repositoryPassword = providers.gradleProperty("reposilitePassword")
                .orElse(providers.environmentVariable("REPOSILITE_PASSWORD"))
            maven {
                name = "reposilite"
                url = repositoryUri
                if (repositoryUri.scheme in setOf("http", "https") &&
                    repositoryUsername.isPresent && repositoryPassword.isPresent
                ) {
                    credentials {
                        username = repositoryUsername.get()
                        password = repositoryPassword.get()
                    }
                }
            }
        }
    }
}
