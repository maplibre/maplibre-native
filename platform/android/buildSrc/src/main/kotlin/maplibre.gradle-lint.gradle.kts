tasks.register<Copy>("ciLint") {
    if (isLocalBuild()) {
        from(file("$projectDir/lint/lint-baseline-local.xml"))
        into("$projectDir")
        rename { fileName ->
            fileName.replace("lint-baseline-local.xml", "lint-baseline.xml")
        }
    } else {
        from(file("$projectDir/lint/lint-baseline-ci.xml"))
        into("$projectDir")
        rename { fileName ->
            fileName.replace("lint-baseline-ci.xml", "lint-baseline.xml")
        }
    }
}

fun isLocalBuild(): Boolean {
    return System.getenv("IS_LOCAL_DEVELOPMENT")?.toBoolean() ?: true
}
