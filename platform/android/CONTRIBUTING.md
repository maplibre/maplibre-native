# Contributing - MapLibre GL Native for Android

## Kotlin

All new code should be written in [Kotlin](https://kotlinlang.org/).

### Style Checking

To check Kotlin style, we use [ktlint](https://pinterest.github.io/ktlint/). This linter is based on the [official Kotlin coding conventions](https://kotlinlang.org/docs/coding-conventions.html). We intergrate with it using the [kotlinder](https://github.com/jeremymailen/kotlinter-gradle) Gradle plugin.

To check the style of all Kotlin source files, use:

```
$ ./gradlew checkStyle
```

To format all Kotlin source files, use:

```
$ ./gradlew formatStyle
```

You might find it helpful to install the [Ktlint](https://plugins.jetbrains.com/plugin/15057-ktlint-unofficial-) Android Studio plugin.
