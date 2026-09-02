repositories {
  google()
  mavenCentral()
}

plugins {
  `java-gradle-plugin`
  `kotlin-dsl`
}

dependencies {
  implementation("com.android.tools.build:gradle:9.3.1")
}

group = "org.maplibre"
version = "0.0.1"

gradlePlugin {
  plugins {
    create("cmakePlugin") {
      id = "org.maplibre.ccache-plugin"
      implementationClass = "org.maplibre.CcachePlugin"
    }
  }
}
