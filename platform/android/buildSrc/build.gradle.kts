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
    implementation("com.github.spotbugs.snom:spotbugs-gradle-plugin:5.2.1")
    testImplementation("junit:junit:4.13")
    implementation("com.vanniktech:gradle-dependency-graph-generator-plugin:0.3.0")
    implementation("com.android.tools.build:gradle:8.6.0")
    implementation("com.android.application:com.android.application.gradle.plugin:8.6.0")
}
