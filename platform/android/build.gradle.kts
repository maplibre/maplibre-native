plugins {
    alias(libs.plugins.nexusPublishPlugin)
    alias(libs.plugins.kotlinter) apply false
    alias(libs.plugins.kotlinAndroid) apply false
    id("com.jaredsburrows.license") version "0.9.8" apply false
    id("maplibre.dependencies")
    id("maplibre.publish-root")
}


nexusPublishing {
    repositories {
        sonatype {
            stagingProfileId.set(extra["sonatypeStagingProfileId"] as String?)
            username.set(extra["mavenCentralUsername"] as String?)
            password.set(extra["mavenCentralPassword"] as String?)
            nexusUrl.set(uri("https://ossrh-staging-api.central.sonatype.com/service/local/"))
            snapshotRepositoryUrl.set(uri("https://central.sonatype.com/repository/maven-snapshots/"))
        }
    }
}
