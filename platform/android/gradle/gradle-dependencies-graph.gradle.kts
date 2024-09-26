buildscript {
    repositories {
        mavenCentral()
    }

    dependencies {
        classpath("com.vanniktech:gradle-dependency-graph-generator-plugin:0.3.0")
    }
}

import com.vanniktech.dependency.graph.generator.DependencyGraphGeneratorExtension.Generator
        import com.vanniktech.dependency.graph.generator.dot.GraphFormattingOptions
        import com.vanniktech.dependency.graph.generator.dot.Color
        import com.vanniktech.dependency.graph.generator.dot.Shape
        import com.vanniktech.dependency.graph.generator.dot.Style

        plugins {
            id("com.vanniktech.dependency.graph.generator")
        }

val mapboxGenerator = Generator(
    suffix = "mapboxLibraries",  // Suffix for our Gradle task.
    rootSuffix = "",  // Root suffix that we don't want in this case.
    include = { dependency -> dependency.moduleGroup.startsWith("com.mapbox.mapboxsdk") }, // Only want Mapbox libs.
    transitive = { true } // Include transitive dependencies.
)

dependencyGraphGenerator {
    generators.add(mapboxGenerator)
}
