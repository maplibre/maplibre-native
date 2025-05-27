# Development Apps

There are two iOS apps available in the repo that you can use for MapLibre Native development. One Objective-C based app and a Swift based app.

## Objective-C App

This app is available as "App" in the [generated Xcode project](./README.md).

The source code lives in `platform/ios/app`.

You can also build and run it from the command line with:

```
bazel run --//:renderer=metal //platform/ios:App
```

<p align="center">
  <img width="300" alt="Objective-C app screenshot" src="https://github.com/user-attachments/assets/aeb0cb5e-1f6c-439e-8668-22ee0a0b11f2" />
</p>

## Swift App

The Swift App is mainly used to demo usage patterns in the [example documentation](./ios-documentation.md#examples).

This app is available as "MapLibreApp" in the [generated Xcode project](./README.md).

The source code lives in `platform/ios/swift-app`.

<p align="center">
  <img width="300" alt="Swift app screenshot" src="https://github.com/user-attachments/assets/87f4cea4-40dd-4744-a935-c7cebd6887f1" />
</p>

You can also build and run it from the command line with:

```
bazel run --//:renderer=metal //platform/ios/app-swift:MapLibreApp
```
