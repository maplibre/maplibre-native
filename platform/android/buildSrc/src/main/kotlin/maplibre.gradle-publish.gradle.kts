import org.gradle.kotlin.dsl.get
import java.util.Locale

plugins {
    `maven-publish`
    signing
    id("com.android.library")
    id("maplibre.artifact-settings")
    id("maplibre.publish-root")
}

tasks.register<Javadoc>("androidJavadocs") {
    source = fileTree(android.sourceSets.getByName("main").java.srcDirs)
    classpath = files(android.bootClasspath)
    isFailOnError = false
}

tasks.register<Jar>("androidJavadocsJar") {
    archiveClassifier.set("javadoc")
    from(tasks.named("androidJavadocs", Javadoc::class.java).get().destinationDir)
}

tasks.register<Jar>("androidSourcesJar") {
    archiveClassifier.set("sources")
    from(android.sourceSets.getByName("main").java.srcDirs)
}

tasks.withType<Javadoc> {
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

fun PublishingExtension.configureMavenPublication(
    renderer: String,
    publicationName: String,
    artifactIdPostfix: String,
    descriptionPostfix: String,
    buildType: String = "Release"
) {
    publications {
        create<MavenPublication>(publicationName) {
            groupId = project.group.toString().replace("org.maplibre.gl", "org.hudhud.maplibre.gl")
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

afterEvaluate {
    publishing {
        configureMavenPublication("drawable", "opengl", "", "")
        configureMavenPublication("drawable", "opengldebug", "-debug", " (Debug)", "Debug")
        configureMavenPublication("vulkan", "vulkan", "-vulkan", "(Vulkan)")
        configureMavenPublication("vulkan", "vulkandebug", "-vulkan-debug", "(Vulkan, Debug)", "Debug")

        repositories {
            maven {
                name = "GithubPackages"
                url = uri("https://maven.pkg.github.com/HudHud-Maps/maplibre-native")
                credentials {
                    username = System.getenv("GITHUB_ACTOR")
                    password = System.getenv("GITHUB_TOKEN")
                }
            }
        }
    }
}

afterEvaluate {
    android.libraryVariants.forEach { variant ->
        tasks.named("androidJavadocs", Javadoc::class.java).configure {
            doFirst {
                classpath = classpath.plus(files(variant.javaCompileProvider.get().classpath))
            }
        }
    }
}

// signing {
//     sign(publishing.publications)
// }
