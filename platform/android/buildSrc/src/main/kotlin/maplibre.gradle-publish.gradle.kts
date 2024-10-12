import org.gradle.kotlin.dsl.get

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

afterEvaluate {
    publishing {
        publications {
            create<MavenPublication>("release") {
                groupId = this@afterEvaluate.group.toString()
                artifactId = project.extra["mapLibreArtifactId"].toString()
                version = this@afterEvaluate.version.toString()

                // Conditional component selection based on environment variable
                from(components[if (System.getenv("RENDERER")?.lowercase() == "vulkan") "vulkanRelease" else "drawableRelease"])

                pom {
                    name.set(project.extra["mapLibreArtifactTitle"].toString())
                    description.set(project.extra["mapLibreArtifactTitle"].toString())
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
                            email.set("maplibre@maplibre.org")
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
    android.libraryVariants.forEach { variant ->
        tasks.named("androidJavadocs", Javadoc::class.java).configure {
            doFirst {
                classpath = classpath.plus(files(variant.javaCompileProvider.get().classpath))
            }
        }
    }
}

signing {
    sign(publishing.publications)
}
