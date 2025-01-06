# iOS Tests

## Render Tests

To run the render tests, run the `RenderTest` target from iOS.

When running in a simulator, use

```
# check for 'DataContainer' of the app with `*.maplibre.RenderTestApp` id
xcrun simctl listapps booted
```

to get the data directory of the render test app. This allows you to inspect test results. When adding new tests, the generated expectations and `actual.png` file can be copied into the source directory from here.

## C++ Unit Tests

Run the tests from the `CppUnitTests` target in Xcode to run the C++ Unit Tests on iOS.