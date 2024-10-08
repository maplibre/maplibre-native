plugins {
    checkstyle
}


checkstyle {
    toolVersion = "7.1.1" // 7.3
    configFile = file("../checkstyle.xml")
}

tasks.register<Checkstyle>("checkstyle") {
    description = "Checks if the code adheres to coding standards"
    group = "verification"
    configFile = file("../checkstyle.xml")
    source("src")
    include("**/*.java")
    exclude("**/gen/**")
    exclude("**/style/*LayerTest.java")
    exclude("**/style/LightTest.java")
    exclude("**/style/layers/Property.java")
    exclude("**/style/layers/PropertyFactory.java")
    exclude("**/style/layers/*Layer.java")
    exclude("**/style/light/Light.java")
    exclude("**/log/LoggerDefinition.java")
    exclude("**/log/Logger.java")
    exclude("**/Expression.java")
    exclude("**/location/LocationPropertyFactory.java")
    exclude("**/location/LocationIndicatorLayer.java")
    exclude("**/location/LocationIndicatorLayerTest.java")
    classpath = files()
    isIgnoreFailures = false
}
