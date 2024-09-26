plugins {
    id("jacoco")
}

apply(from = "${rootDir}/gradle/dependencies.gradle.kts")

jacoco {
    toolVersion = versions.jacoco
}

tasks.register<JacocoReport>("jacocoTestReport") {
    group = "Reporting"
    description = "Combine code coverage to unified report."
    dependsOn("testDebugUnitTest")

    reports {
        xml.isEnabled = true
        html.isEnabled = true
    }

    val fileExcludes = listOf("**/R.class", "**/R$*.class", "**/BuildConfig.*", "**/Manifest*.*", "**/*Test*.*", "android/**/*.*")
    val debugTree = fileTree(mapOf("dir" to "${project.buildDir}/intermediates/javac/debug/classes", "excludes" to fileExcludes))
    val mainSrc = "${project.projectDir}/src/main/java"
    val ecSrc = fileTree(mapOf("dir" to "$project.buildDir", "include" to listOf("**/*.ec")))
    val execSrc = fileTree(mapOf("dir" to "$project.buildDir", "include" to listOf("**/*.exec")))

    doFirst {
        val files = files(ecSrc, execSrc).files
        println("Creating Jacoco Report for ${files.size} coverage files")
        files.forEach { file -> println(file) }
    }

    sourceDirectories.setFrom(files(mainSrc))
    classDirectories.setFrom(files(debugTree))
    executionData.setFrom(files(ecSrc, execSrc))
}
