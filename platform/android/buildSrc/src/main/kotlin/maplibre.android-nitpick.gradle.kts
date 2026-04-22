import org.gradle.process.ExecOperations
import javax.inject.Inject

plugins {
    id("maplibre.dependencies")
}

abstract class AndroidNitpickTask @Inject constructor(
    private val execOperations: ExecOperations
) : DefaultTask() {

    @get:Internal
    abstract val workingDirectory: DirectoryProperty

    @TaskAction
    fun run() {
        logger.lifecycle("Running android nitpick script")
        logger.lifecycle("Verify license generation with git diff...")
        execOperations.exec {
            workingDir = workingDirectory.get().asFile
            commandLine("python3", "scripts/validate-license.py")
        }
    }
}

tasks.register<AndroidNitpickTask>("androidNitpick") {
    workingDirectory.set(rootDir)
}
