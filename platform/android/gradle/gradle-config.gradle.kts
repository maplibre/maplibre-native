// Configuration file for gradle build execution.

tasks.register("apiKey") {
    doLast {
        val tokenFile = file("${projectDir}/src/main/res/values/developer-config.xml")
        if (!tokenFile.exists()) {
            var apiKey = System.getenv("MLN_API_KEY") ?: "null"
            if (apiKey == "null") {
                println("You should set the MLN_API_KEY environment variable.")
                apiKey = "YOUR_API_KEY_GOES_HERE"
            }
            val tokenFileContents = """
                <?xml version="1.0" encoding="utf-8"?>
                <resources>
                    <string name="api_key">$apiKey</string>
                </resources>
            """.trimIndent()
            tokenFile.writeText(tokenFileContents)
        }
    }
}

gradle.projectsEvaluated {
    tasks.named("preBuild").configure {
        dependsOn(tasks.named("apiKey"))
    }
}
