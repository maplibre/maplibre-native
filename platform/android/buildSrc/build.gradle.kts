plugins {
    `kotlin-dsl`
}

repositories {
    gradlePluginPortal()
    google()
    mavenCentral()
    maven("https://plugins.gradle.org/m2/")
}

dependencies {
    implementation("com.vanniktech:gradle-dependency-graph-generator-plugin:0.3.0")
    implementation("com.vanniktech:gradle-maven-publish-plugin:0.34.0")
    implementation("com.android.tools.build:gradle:8.6.0")
    implementation("com.android.application:com.android.application.gradle.plugin:8.6.0")
    implementation("commons-collections:commons-collections:3.2.2")
    implementation("io.sentry:sentry-android-gradle-plugin:5.9.0")
}
