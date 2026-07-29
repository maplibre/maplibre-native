extra["mapLibreArtifactGroupId"] = providers.gradleProperty("maplibre.group")
    .getOrElse("org.maplibre.gl")
extra["mapLibreArtifactId"] = "android-sdk"
extra["mapLibreArtifactTitle"] = "MapLibre Android"
extra["mapLibreArtifactDescription"] = "MapLibre Android"
extra["mapLibreDeveloperName"] = "MapLibre"
extra["mapLibreDeveloperId"] = "maplibre"
extra["mapLibreArtifactUrl"] = "https://github.com/maplibre/maplibre-native"
extra["mapLibreArtifactScmUrl"] = "scm:git@github.com:maplibre/maplibre-native.git"
extra["mapLibreArtifactLicenseName"] = "BSD"
extra["mapLibreArtifactLicenseUrl"] = "https://opensource.org/licenses/BSD-2-Clause"

val versionFilePath = rootDir.resolve("VERSION")
val versionName = providers.gradleProperty("maplibre.version").orNull
    ?: if (versionFilePath.exists()) {
        versionFilePath.readText().trim()
    } else {
        throw GradleException("VERSION file not found at ${versionFilePath.absolutePath}")
    }

extra["versionName"] = versionName
