import com.android.build.api.dsl.LibraryExtension
import com.android.build.api.variant.LibraryAndroidComponentsExtension
import org.gradle.api.Task
import org.gradle.api.artifacts.repositories.PasswordCredentials
import org.gradle.api.publish.maven.tasks.PublishToMavenRepository
import org.gradle.api.tasks.compile.JavaCompile
import org.gradle.authentication.http.BasicAuthentication
import org.gradle.external.javadoc.StandardJavadocDocletOptions
import org.gradle.kotlin.dsl.get
import org.gradle.plugins.signing.Sign
import java.util.Locale

plugins {
    `maven-publish`
    signing
    id("com.android.library")
    id("com.vanniktech.maven.publish.base")
    id("maplibre.artifact-settings")
}

val androidComponents = extensions.getByType<LibraryAndroidComponentsExtension>()
val androidLibrary = extensions.getByType<LibraryExtension>()
val publicationRepositoryUrl = providers.gradleProperty("publicationRepositoryUrl")
    .orElse(providers.environmentVariable("MAVEN_REPOSITORY_URL"))
val publicationRepositoryUsername = providers.gradleProperty("publicationRepositoryUsername")
    .orElse(providers.environmentVariable("MAVEN_REPOSITORY_USERNAME"))
val publicationRepositoryPassword = providers.gradleProperty("publicationRepositoryPassword")
    .orElse(providers.environmentVariable("MAVEN_REPOSITORY_PASSWORD"))

/**
 * A build variant published as `android-sdk<rendererArtifactIdPostfix><postfix>`,
 * e.g. `android-sdk-opengl-with-plugin-api-debug`.
 */
class PublishedVariant(
    val renderer: String,
    val rendererArtifactIdPostfix: String,
    val rendererTitle: String,
    val withPluginApi: Boolean,
    val buildType: String
) {
    val name = renderer + (if (withPluginApi) "WithPluginApi" else "") + buildType
    val postfix = (if (withPluginApi) "-with-plugin-api" else "") + (if (buildType == "Debug") "-debug" else "")
    val titleDetails = listOfNotNull("Plugin API".takeIf { withPluginApi }, "Debug".takeIf { buildType == "Debug" })
}

// The default renderer is also published without a renderer postfix, as `android-sdk`.
val defaultRenderer = "vulkan"
val publishedVariants = listOf(
    Triple("vulkan", "-vulkan", "Vulkan"),
    Triple("opengl", "-opengl", "OpenGL ES"),
    Triple("multiBackend", "-vulkan-opengl", "Vulkan + OpenGL ES")
).flatMap { (renderer, artifactIdPostfix, title) ->
    listOf(false, true).flatMap { withPluginApi ->
        listOf("Release", "Debug").map { buildType ->
            PublishedVariant(renderer, artifactIdPostfix, title, withPluginApi, buildType)
        }
    }
}

androidLibrary.publishing {
    publishedVariants.forEach { singleVariant(it.name) }
}

afterEvaluate {
    val configuredRepositoryUrl = publicationRepositoryUrl.orNull
    if (configuredRepositoryUrl == null) {
        mavenPublishing {
            publishToMavenCentral(true)
        }
    } else {
        publishing.repositories.maven {
            name = "Configured"
            url = uri(configuredRepositoryUrl)

            val configuredUsername = publicationRepositoryUsername.orNull
            val configuredPassword = publicationRepositoryPassword.orNull
            require(configuredUsername.isNullOrBlank() == configuredPassword.isNullOrBlank()) {
                "Both publicationRepositoryUsername and publicationRepositoryPassword must be configured together"
            }
            if (!configuredUsername.isNullOrBlank()) {
                credentials(PasswordCredentials::class) {
                    username = configuredUsername
                    password = configuredPassword
                }
                authentication {
                    create<BasicAuthentication>("basic")
                }
            }
        }
    }

    if (configuredRepositoryUrl == null || providers.gradleProperty("signingInMemoryKey").isPresent) {
        mavenPublishing {
            signAllPublications()
        }
    }
}

tasks.register<Javadoc>("androidJavadocs") {
    source = fileTree("src/main/java")
    classpath = files()
    isFailOnError = false
}

tasks.register<Jar>("androidJavadocsJar") {
    archiveClassifier.set("javadoc")
    from(tasks.named("androidJavadocs", Javadoc::class.java).map { it.destinationDir!! })
    dependsOn(tasks.named("androidJavadocs"))
}

tasks.register<Jar>("androidSourcesJar") {
    archiveClassifier.set("sources")
    from("src/main/java")
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
    componentName: String,
    publicationName: String,
    artifactIdPostfix: String,
    descriptionPostfix: String
) {
    publishing {
        publications {
            create<MavenPublication>(publicationName) {
                groupId = project.group.toString()
                artifactId = "${project.extra["mapLibreArtifactId"]}$artifactIdPostfix"
                version = project.version.toString()

                val component = components.findByName(componentName)
                    ?: components.find { it.name.equals(componentName, ignoreCase = true) }
                if (component != null) {
                    from(component)
                } else {
                    project.logger.warn(
                        "Skipping publication '$publicationName' because component '$componentName' was not found. " +
                            "Available components: ${components.map { it.name }.sorted()}"
                    )
                }

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
    publishedVariants.forEach { variant ->
        val publicationNames = mutableListOf(variant.name)
        val details = listOf(variant.rendererTitle) + variant.titleDetails
        configureMavenPublication(
            variant.name,
            variant.name,
            variant.rendererArtifactIdPostfix + variant.postfix,
            " (${details.joinToString()})"
        )
        if (variant.renderer == defaultRenderer) {
            val publicationName = variant.name.replaceFirst(variant.renderer, "default")
            publicationNames += publicationName
            configureMavenPublication(
                variant.name,
                publicationName,
                variant.postfix,
                if (variant.titleDetails.isEmpty()) "" else " (${variant.titleDetails.joinToString()})"
            )
        }

        // Publications of the same variant share the artifact and its signature file, so each of
        // their publish tasks has to wait for all of their signing tasks.
        val taskNames = publicationNames.map { it.replaceFirstChar(Char::uppercaseChar) }
        val signingTasks = tasks.withType<Sign>().matching { task ->
            taskNames.any { task.name == "sign${it}Publication" }
        }
        val publishTasks = tasks.withType<PublishToMavenRepository>().matching { task ->
            taskNames.any { task.name.startsWith("publish${it}PublicationTo") }
        }
        publishTasks.configureEach { dependsOn(signingTasks) }

        tasks.register("publish${taskNames.first()}Publications") {
            group = "publishing"
            description = "Publishes ${publicationNames.joinToString(" and ")} to the configured Maven repository."
            dependsOn(publishTasks)
        }
    }
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
