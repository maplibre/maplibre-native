extra["mapLibreArtifactGroupId"] = "org.maplibre.gl"
extra["mapLibreArtifactId"] = "android-sdk"
extra["mapLibreArtifactTitle"] = "MapLibre Android"
extra["mapLibreArtifactDescription"] = "MapLibre Android"
extra["mapLibreDeveloperName"] = "MapLibre"
extra["mapLibreDeveloperId"] = "maplibre"
extra["mapLibreArtifactUrl"] = "https://github.com/maplibre/maplibre-native"
extra["mapLibreArtifactScmUrl"] = "scm:git@github.com:maplibre/maplibre-native.git"
extra["mapLibreArtifactLicenseName"] = "BSD"
extra["mapLibreArtifactLicenseUrl"] = "https://opensource.org/licenses/BSD-2-Clause"

// Handling conditional assignment for versionName
extra["versionName"] = if (project.hasProperty("VERSION_NAME")) {
    project.property("VERSION_NAME")
} else {
    System.getenv("VERSION_NAME")
}
