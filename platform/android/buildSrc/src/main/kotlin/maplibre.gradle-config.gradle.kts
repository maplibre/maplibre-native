tasks.register("apiKey") {
    val tokenFile = file("${projectDir}/src/main/res/values/developer-config.xml")
    if (!tokenFile.exists()) {
        var apiKey = System.getenv("MLN_API_KEY") ?: "null"
        if (apiKey == "null") {
            println("You should set the MLN_API_KEY environment variable.")
            apiKey = "YOUR_API_KEY_GOES_HERE"
        }
        val tokenFileContents = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" +
                "<resources>\n" +
                "    <string name=\"api_key\">$apiKey</string>\n" +
                "</resources>"
        tokenFile.writeText(tokenFileContents)
    }
}

gradle.projectsEvaluated {
    tasks.named("preBuild").configure {
        dependsOn("apiKey")
    }
}
