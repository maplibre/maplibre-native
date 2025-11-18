import com.vanniktech.dependency.graph.generator.DependencyGraphGeneratorExtension.Generator

plugins {
    id("com.vanniktech.dependency.graph.generator")
}

val mapboxGenerator = Generator("mapboxLibraries", // Suffix for our Gradle task.
    "", // Root suffix that we don't want in this case.
    { dependency -> dependency.moduleGroup.startsWith("com.mapbox.mapboxsdk") }, // Only want Mapbox libs.
    { dependency -> true } // Include transitive dependencies.
)

dependencyGraphGenerator {
    generators = listOf(mapboxGenerator)
}
