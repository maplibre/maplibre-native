apply(from = "${rootDir}/gradle/dependencies.gradle.kts")

tasks.register("androidNitpick") {
    doLast {
        println("Running android nitpick script")

        verifyLicenseGeneration()
    }
}

fun verifyLicenseGeneration() {
    println("Verify license generation with git diff...")
    exec {
        workingDir = file(rootDir)
        commandLine("python3", "scripts/validate-license.py")
    }
}
