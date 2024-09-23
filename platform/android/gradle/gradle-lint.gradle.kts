tasks.register<Copy>("ciLint") {
    val isLocal = isLocalBuild()

    from(if (isLocal) {
        "$projectDir/lint/lint-baseline-local.xml"
    } else {
        "$projectDir/lint/lint-baseline-ci.xml"
    })

    into("$projectDir")

    rename { fileName: String ->
        fileName.replace(if (isLocal) "lint-baseline-local.xml" else "lint-baseline-ci.xml", "lint-baseline.xml")
    }
}

fun isLocalBuild(): Boolean {
    return System.getenv("IS_LOCAL_DEVELOPMENT")?.toBoolean() ?: true
}
