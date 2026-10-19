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

includeBuild("../../platform/android") {
    dependencySubstitution {
        substitute(module("org.maplibre.gl:android-sdk")).using(project(":MapLibreAndroid"))
    }
}
