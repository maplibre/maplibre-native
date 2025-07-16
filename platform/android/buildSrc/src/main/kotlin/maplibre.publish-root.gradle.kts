extra["signing.keyId"] = System.getenv("SIGNING_KEY_ID")
extra["signing.password"] = System.getenv("SIGNING_PASSWORD")
extra["signing.secretKeyRingFile"] = "${projectDir}/../signing-key.gpg"
extra["mavenCentralUsername"] = System.getenv("MAVEN_CENTRAL_USERNAME")
extra["mavenCentralPassword"] = System.getenv("MAVEN_CENTRAL_PASSWORD")
extra["sonatypeStagingProfileId"] = System.getenv("SONATYPE_STAGING_PROFILE_ID")
