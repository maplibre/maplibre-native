tasks.register<Exec>("makeClean") {
    workingDir = file("../../")
    commandLine("make", "clean")
}

tasks.register<Exec>("makeAndroid") {
    workingDir = file("../../")
    commandLine("make", "android")
}

tasks.register<Exec>("makeAndroidAll") {
    workingDir = file("../../")
    commandLine("make", "apackage")
}
