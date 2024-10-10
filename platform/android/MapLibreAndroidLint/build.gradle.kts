plugins {
    alias(libs.plugins.kotlinter)
    `java-library`
    kotlin("jvm")
}

dependencies {
    implementation(fileTree(mapOf("dir" to "libs", "include" to listOf("*.jar"))))

    compileOnly(libs.lint)
    compileOnly(libs.lintApi)
    compileOnly(libs.lintChecks)
    compileOnly(libs.supportAnnotations)

    testImplementation(libs.junit)
    testImplementation(libs.robolectric)
    testImplementation(libs.lintTests)
}

kotlin {
    jvmToolchain(17)
}
