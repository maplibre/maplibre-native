# User Location & Location Privacy

Requesting precise location with ``MLNLocationManager``.

## Overview

This example shows how to request a precise location with ``MLNLocationManager``.

First of all you need to prepare your `Info.plist`. You need to provide a description why your app needs to access location:

```plist
<key>NSLocationWhenInUseUsageDescription</key>
<string>Dummy Location When In Use Description</string>
```

As well as a description why your app needs precise location.

```plist
<key>NSLocationTemporaryUsageDescriptionDictionary</key>
<dict>
  <key>MLNAccuracyAuthorizationDescription</key>
  <string>Dummy Precise Location Description</string>
</dict>
```

The `Coordinator` defined below is also the ``MLNMapViewDelegate``. When the location manager authorization changes it will call the relevant method. If precise location has not been granted, a button is shown at the bottom of the map.

![](ImpreciseLocation.png)

When the button is pressed a pop-up with the description we set in `Info.plist` will be shown:

![](PreciseLocationRequestPopup.png)

<!-- include-example(LocationPrivacyExample) -->

```swift
enum LocationAccuracyState {
    case unknown
    case reducedAccuracy
    case fullAccuracy
}

class Coordinator: NSObject, MLNMapViewDelegate {
    @Binding var mapView: MLNMapView
    @Binding var locationAccuracy: LocationAccuracyState
    var pannedToUserLocation = false

    init(mapView: Binding<MLNMapView>, locationAccuracy: Binding<LocationAccuracyState>) {
        _mapView = mapView
        _locationAccuracy = locationAccuracy
    }

    func mapView(_: MLNMapView, didChangeLocationManagerAuthorization manager: MLNLocationManager) {
        guard let accuracySetting = manager.accuracyAuthorization else {
            return
        }

        switch accuracySetting() {
        case .fullAccuracy:
            locationAccuracy = .fullAccuracy
        case .reducedAccuracy:
            locationAccuracy = .reducedAccuracy
        @unknown default:
            locationAccuracy = .unknown
        }
    }

    // when a location is available for the first time, we fly to it
    func mapView(_ mapView: MLNMapView, didUpdate _: MLNUserLocation?) {
        if pannedToUserLocation {
            return
        }
        guard let userLocation = mapView.userLocation else {
            print("User location is currently not available.")
            return
        }
        mapView.fly(to: MLNMapCamera(lookingAtCenter: userLocation.coordinate, altitude: 100_000, pitch: 0, heading: 0))
    }
}

struct LocationPrivacyExample: UIViewRepresentable {
    @Binding var mapView: MLNMapView
    @Binding var locationAccuracy: LocationAccuracyState

    func makeCoordinator() -> Coordinator {
        Coordinator(mapView: $mapView, locationAccuracy: $locationAccuracy)
    }

    func makeUIView(context: Context) -> MLNMapView {
        let mapView = MLNMapView()
        mapView.showsUserLocation = true
        mapView.delegate = context.coordinator

        return mapView
    }

    func updateUIView(_: MLNMapView, context _: Context) {}
}

struct LocationPrivacyExampleView: View {
    @State private var mapView = MLNMapView()
    @State var locationAccuracy: LocationAccuracyState = .unknown

    var body: some View {
        VStack {
            LocationPrivacyExample(mapView: $mapView, locationAccuracy: $locationAccuracy)
                .edgesIgnoringSafeArea(.all)

            if locationAccuracy == LocationAccuracyState.reducedAccuracy {
                Button("Request Precise Location") {
                    handleButtonPress(mapView: mapView)
                }
                .padding()
                .background(Color.blue)
                .foregroundColor(.white)
                .cornerRadius(8)
            }
        }
    }

    private func handleButtonPress(mapView: MLNMapView) {
        print("Requesting precice location")
        switch locationAccuracy {
        case .reducedAccuracy:
            let purposeKey = "MLNAccuracyAuthorizationDescription"
            mapView.locationManager.requestTemporaryFullAccuracyAuthorization!(withPurposeKey: purposeKey)
        default:
            break
        }
    }
}
```