pluginManagement {
    repositories {
        google {
            content {
                includeGroupByRegex("com\\.android.*")
                includeGroupByRegex("com\\.google.*")
                includeGroupByRegex("androidx.*")
            }
        }
        mavenCentral()
        gradlePluginPortal()
    }
}
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
        maven("https://plugins.gradle.org/m2/")
    }
}

plugins {
    id("org.gradle.toolchains.foojay-resolver-convention") version "0.8.0"
}

include(":MapLibreAndroid", ":MapLibreAndroidTestApp", ":MapLibreAndroidLint")

rootProject.name = "MapLibre Native for Android"

val renderTestProjectDir = file("$rootDir/../../render-test/android")
includeBuild(renderTestProjectDir) {
    name = "renderTestApp"
}

val cppTestProjectDir = file("$rootDir/../../test/android")
includeBuild(cppTestProjectDir) {
    name = "cppUnitTestsApp"
}
