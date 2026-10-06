pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
    }
}

dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
    }
}

rootProject.name = "MapLibrePluginExample"
include(":app", ":ngon-layer")
project(":ngon-layer").projectDir = file("../ngon-layer/android")

// Enable the experimental API in the local SDK, including when VERSION is stable.
// Forward it as a project property because included builds have their own gradle.properties.
gradle.startParameter.projectProperties = gradle.startParameter.projectProperties +
    ("maplibre.with_plugins" to providers.gradleProperty("maplibre.with_plugins").orElse("true").get())

includeBuild("../../platform/android") {
    dependencySubstitution {
        substitute(module("org.maplibre.gl:android-sdk")).using(project(":MapLibreAndroid"))
    }
}
