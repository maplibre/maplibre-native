plugins {
    id("maven-publish")
    id("signing")
}

apply(from = "${rootDir}/gradle/artifact-settings.gradle.kts")
apply(from = "${rootDir}/gradle/publish-root.gradle.kts")

tasks.register<Javadoc>("androidJavadocs") {
    source = android.sourceSets["main"].java.sourceFiles
    classpath = files(android.bootClasspath)
    isFailOnError = false
}

tasks.register<Jar>("androidJavadocsJar") {
    dependsOn(tasks.named("androidJavadocs"))
    archiveClassifier.set("javadoc")
    from(tasks.named("androidJavadocs").get().destinationDir)
}

tasks.register<Jar>("androidSourcesJar") {
    archiveClassifier.set("sources")
    from(android.sourceSets["main"].java.sourceFiles)
}

tasks.withType<Javadoc> {
    options.encoding = "UTF-8"
    options.addStringOption("docencoding", "UTF-8")
    options.addStringOption("charset", "UTF-8")
}

artifacts {
    archives(tasks.named("androidSourcesJar"))
    archives(tasks.named("androidJavadocsJar"))
}

logger.lifecycle(project.ext["versionName"].toString())

version = project.ext["versionName"].toString()
group = project.ext["mapLibreArtifactGroupId"].toString()

afterEvaluate {
    publishing {
        publications {
            create<MavenPublication>("release") {
                groupId = this@afterEvaluate.group.toString()
                artifactId = project.ext["mapLibreArtifactId"].toString()
                version = this@afterEvaluate.version.toString()

                from(components[if (System.getenv("RENDERER")?.lowercase() == "vulkan") "vulkanRelease" else "drawableRelease"])

                pom {
                    name.set(project.ext["mapLibreArtifactTitle"].toString())
                    description.set(project.ext["mapLibreArtifactDescription"].toString())
                    url.set(project.ext["mapLibreArtifactUrl"].toString())
                    licenses {
                        license {
                            name.set(project.ext["mapLibreArtifactLicenseName"].toString())
                            url.set(project.ext["mapLibreArtifactLicenseUrl"].toString())
                        }
                    }
                    developers {
                        developer {
                            id.set(project.ext["mapLibreDeveloperId"].toString())
                            name.set(project.ext["mapLibreDeveloperName"].toString())
                            email.set("maplibre@maplibre.org")
                        }
                    }
                    scm {
                        connection.set(project.ext["mapLibreArtifactScmUrl"].toString())
                        developerConnection.set(project.ext["mapLibreArtifactScmUrl"].toString())
                        url.set(project.ext["mapLibreArtifactUrl"].toString())
                    }
                }
            }
        }
    }
}

afterEvaluate { project ->
    android.libraryVariants.all { variant ->
        tasks.named("androidJavadocs").configure {
            doFirst {
                classpath += files(variant.javaCompileProvider.get().classpath.files)
            }
        }
    }
}

signing {
    sign(publishing.publications)
}
