plugins {
    alias(libs.plugins.nexusPublishPlugin)
    alias(libs.plugins.kotlinter) apply false
    alias(libs.plugins.kotlinAndroid) apply false
    id("com.jaredsburrows.license") version "0.9.8" apply false
    id("maplibre.dependencies")
    id("maplibre.publish-root")
}


// nexusPublishing {
//     repositories {
//         sonatype {
//             stagingProfileId.set(extra["sonatypeStagingProfileId"] as String?)
//             username.set(extra["ossrhUsername"] as String?)
//             password.set(extra["ossrhPassword"] as String?)
//             nexusUrl.set(uri("https://s01.oss.sonatype.org/service/local/"))
//             snapshotRepositoryUrl.set(uri("https://s01.oss.sonatype.org/content/repositories/snapshots/"))
//         }
//     }
// }
