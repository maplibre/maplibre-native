import com.android.build.api.variant.LibraryAndroidComponentsExtension
import com.android.build.gradle.LibraryExtension
import org.gradle.api.Task
import org.gradle.api.tasks.compile.JavaCompile
import org.gradle.external.javadoc.StandardJavadocDocletOptions
import org.gradle.kotlin.dsl.get
import java.util.Locale

plugins {
    `maven-publish`
    signing
    id("com.android.library")
    id("com.vanniktech.maven.publish.base")
    id("maplibre.artifact-settings")
}

val androidExt = extensions.getByType<LibraryExtension>()
val androidComponents = extensions.getByType<LibraryAndroidComponentsExtension>()

afterEvaluate {
    mavenPublishing {
        publishToMavenCentral(true)
        signAllPublications()
    }
}

// Configure task dependencies after all tasks are created
gradle.projectsEvaluated {
    // Explicitly configure publish tasks to depend on their corresponding signing tasks
    // This fixes Gradle's implicit dependency validation warnings
    // Since some publications may share components (e.g., defaultdebug and opengldebug both use openglDebug),
    // we ensure all signing tasks complete before any publish task
    tasks.filter { it.name.startsWith("publish") && it.name.endsWith("PublicationToMavenCentralRepository") }.forEach { publishTask ->
        tasks.filter { it.name.startsWith("sign") && it.name.endsWith("Publication") }.forEach { signingTask ->
            publishTask.dependsOn(signingTask)
        }
    }
}

tasks.register<Javadoc>("androidJavadocs") {
    source = fileTree(androidExt.sourceSets.getByName("main").java.srcDirs)
    classpath = files(androidExt.bootClasspath)
    isFailOnError = false
}

tasks.register<Jar>("androidJavadocsJar") {
    archiveClassifier.set("javadoc")
    from(tasks.named("androidJavadocs", Javadoc::class.java).map { it.destinationDir!! })
    dependsOn(tasks.named("androidJavadocs"))
}

tasks.register<Jar>("androidSourcesJar") {
    archiveClassifier.set("sources")
    from(androidExt.sourceSets.getByName("main").java.srcDirs)
}

tasks.withType<Javadoc>().configureEach {
    options.encoding = "UTF-8"
    (options as StandardJavadocDocletOptions).apply {
        charSet = "UTF-8"
        docEncoding = "UTF-8"
    }
}

artifacts {
    add("archives", tasks.named("androidSourcesJar"))
    add("archives", tasks.named("androidJavadocsJar"))
}

project.logger.lifecycle(project.extra["versionName"].toString())

version = project.extra["versionName"] as String
group = project.extra["mapLibreArtifactGroupId"] as String

fun configureMavenPublication(
    renderer: String,
    publicationName: String,
    artifactIdPostfix: String,
    descriptionPostfix: String,
    buildType: String = "Release"
) {
    publishing {
        publications {
            create<MavenPublication>(publicationName) {
                groupId = project.group.toString()
                artifactId = "${project.extra["mapLibreArtifactId"]}$artifactIdPostfix"
                version = project.version.toString()

                from(components["${renderer}${buildType}"])

                pom {
                    name.set("${project.extra["mapLibreArtifactTitle"]}$descriptionPostfix")
                    description.set("${project.extra["mapLibreArtifactTitle"]}$descriptionPostfix")
                    url.set(project.extra["mapLibreArtifactUrl"].toString())
                    licenses {
                        license {
                            name.set(project.extra["mapLibreArtifactLicenseName"].toString())
                            url.set(project.extra["mapLibreArtifactLicenseUrl"].toString())
                        }
                    }
                    developers {
                        developer {
                            id.set(project.extra["mapLibreDeveloperId"].toString())
                            name.set(project.extra["mapLibreDeveloperName"].toString())
                            email.set("team@maplibre.org")
                        }
                    }
                    scm {
                        connection.set(project.extra["mapLibreArtifactScmUrl"].toString())
                        developerConnection.set(project.extra["mapLibreArtifactScmUrl"].toString())
                        url.set(project.extra["mapLibreArtifactUrl"].toString())
                    }
                }
            }
        }
    }
}

afterEvaluate {
    configureMavenPublication("vulkan", "defaultrelease", "", "")
    configureMavenPublication("vulkan", "defaultdebug", "-debug", " (Debug)", "Debug")
    configureMavenPublication("vulkan", "vulkanrelease", "-vulkan", "(Vulkan)")
    configureMavenPublication("vulkan", "vulkandebug", "-vulkan-debug", "(Vulkan, Debug)", "Debug")
    // Right now this is the same as the first, but in the future we might release a major version
    // which defaults to Vulkan (or has support for multiple backends). We will keep using only
    // OpenGL ES with this artifact ID if that happens.
    configureMavenPublication("opengl", "openglrelease", "-opengl", " (OpenGL ES)")
    configureMavenPublication("opengl", "opengldebug", "-opengl-debug", " (OpenGL ES, Debug)", "Debug")
}

// Wire per-variant compile classpaths into the Javadoc task.
// Replaces the removed `android.libraryVariants` API with the new AndroidComponents API,
// looking up the generated JavaCompile task by its conventional name.
androidComponents.onVariants { variant ->
    val capitalizedName = variant.name.replaceFirstChar {
        if (it.isLowerCase()) it.titlecase(Locale.ROOT) else it.toString()
    }
    val javaCompileTaskName = "compile${capitalizedName}JavaWithJavac"

    tasks.named("androidJavadocs", Javadoc::class.java).configure {
        val javaCompile = tasks.named(javaCompileTaskName, JavaCompile::class.java)
        dependsOn(javaCompile)
        doFirst {
            classpath = classpath.plus(files(javaCompile.get().classpath))
        }
    }
}