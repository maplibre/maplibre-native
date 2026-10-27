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
include(":app")
// One generated module per plugin that supports Android, see plugins/tools/generate.py.
file("../../android").listFiles { module -> File(module, "build.gradle.kts").isFile }!!.sorted().forEach {
    include(":${it.name}")
    project(":${it.name}").projectDir = it
}

includeBuild("../../../platform/android") {
    dependencySubstitution {
        substitute(module("org.maplibre.gl:android-sdk")).using(project(":MapLibreAndroid"))
    }
}
