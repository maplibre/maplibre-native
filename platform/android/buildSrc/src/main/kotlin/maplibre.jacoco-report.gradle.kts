plugins {
    jacoco
    id("maplibre.dependencies")
}

jacoco {
    toolVersion = "0.8.12"
}

tasks.register<JacocoReport>("jacocoTestReport") {
    group = "Reporting"
    description = "Combine code coverage to unified report."
    dependsOn("testDebugUnitTest")

    reports {
        xml.required = true
        html.required = true
    }

    val fileExcludes = listOf("**/R.class", "**/R$*.class", "**/BuildConfig.*", "**/Manifest*.*", "**/*Test*.*", "android/**/*.*")
    val debugTree = fileTree("${project.buildDir}/intermediates/javac/debug/classes") { exclude(fileExcludes) }
    val mainSrc = "${project.projectDir}/src/main/java"
    val ecSrc = fileTree("${project.buildDir}") { include("**/*.ec") }
    val execSrc = fileTree("${project.buildDir}") { include("**/*.exec") }

    doFirst {
        val files = files(ecSrc, execSrc).files
        println("Creating Jacoco Report for ${files.size} coverage files")
        files.forEach { file -> println(file) }
    }

    sourceDirectories.setFrom(files(mainSrc))
    classDirectories.setFrom(files(debugTree))
    executionData.setFrom(files(ecSrc, execSrc))
}
