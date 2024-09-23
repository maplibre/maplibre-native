buildscript {
    apply(from = "${rootDir}/gradle/dependencies.gradle.kts")

    repositories {
        google()
        maven { url = uri("https://plugins.gradle.org/m2/") }
        mavenCentral()
    }

    dependencies {
        classpath(libs.androidGradlePlugin)
        classpath(libs.licensesPlugin)
        classpath(libs.gradleNexus)
        classpath(libs.kotlinPlugin)
    }
}

plugins {
    alias(libs.plugins.nexusPublishPlugin)
    alias(libs.plugins.kotlinter) apply false
    alias(libs.plugins.kotlinAndroid) apply false
}

allprojects {
    repositories {
        mavenCentral()
        google()
    }
}

subprojects {
    apply(from = "${rootDir}/gradle/dependencies.gradle.kts")
}

apply(from = "${rootDir}/gradle/publish-root.gradle.kts")

nexusPublishing {
    repositories {
        sonatype {
            stagingProfileId.set(sonatypeStagingProfileId)
            username.set(ossrhUsername)
            password.set(ossrhPassword)
            nexusUrl.set(uri("https://s01.oss.sonatype.org/service/local/"))
            snapshotRepositoryUrl.set(uri("https://s01.oss.sonatype.org/content/repositories/snapshots/"))
        }
    }
}
