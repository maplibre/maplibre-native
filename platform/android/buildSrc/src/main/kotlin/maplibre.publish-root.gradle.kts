extra["signing.keyId"] = System.getenv("SIGNING_KEY_ID")
extra["signing.password"] = System.getenv("SIGNING_PASSWORD")
extra["signing.secretKeyRingFile"] = "${projectDir}/../signing-key.gpg"
extra["ossrhUsername"] = System.getenv("OSSRH_USERNAME")
extra["ossrhPassword"] = System.getenv("OSSRH_PASSWORD")
extra["sonatypeStagingProfileId"] = System.getenv("SONATYPE_STAGING_PROFILE_ID")