val signingKeyId: String? = System.getenv("SIGNING_KEY_ID")
val signingPassword: String? = System.getenv("SIGNING_PASSWORD")
val signingSecretKeyRingFile: String = "${projectDir}/../signing-key.gpg"
val ossrhUsername: String? = System.getenv("OSSRH_USERNAME")
val ossrhPassword: String? = System.getenv("OSSRH_PASSWORD")
val sonatypeStagingProfileId: String? = System.getenv("SONATYPE_STAGING_PROFILE_ID")

ext["signing.keyId"] = signingKeyId
ext["signing.password"] = signingPassword
ext["signing.secretKeyRingFile"] = signingSecretKeyRingFile
ext["ossrhUsername"] = ossrhUsername
ext["ossrhPassword"] = ossrhPassword
ext["sonatypeStagingProfileId"] = sonatypeStagingProfileId
