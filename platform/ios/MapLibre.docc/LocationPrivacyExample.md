# User Location & Location Privacy

Requesting precise location with ``MLNLocationManager``.

> Note: This example uses SwiftUI.

This example shows how to request a precise location with ``MLNLocationManager``.

Let's prepare your `Info.plist`:

First, provide a description why your app needs to access location:

```plist
<key>NSLocationWhenInUseUsageDescription</key>
<string>Dummy Location When In Use Description</string>
```

Second, add a description why your app needs precise location:

```plist
<key>NSLocationTemporaryUsageDescriptionDictionary</key>
<dict>
  <key>MLNAccuracyAuthorizationDescription</key>
  <string>Dummy Precise Location Description</string>
</dict>
```

Third and finally, specify your app only needs reduced location access by default (until you request precise accuracy in the example):

```plist
<key>NSLocationDefaultAccuracyReduced</key>
<true/>
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

@MainActor
class PrivacyExampleViewModel: NSObject, ObservableObject {
    @Published var locationAccuracy: LocationAccuracyState = .unknown
    @Published var showTemporaryLocationAuthorization = false
}

class PrivacyExampleCoordinator: NSObject, MLNMapViewDelegate {
    @ObservedObject private var mapViewModel: PrivacyExampleViewModel
    private var pannedToUserLocation = false

    init(mapViewModel: PrivacyExampleViewModel) {
        self.mapViewModel = mapViewModel
        super.init()
    }

    @MainActor func mapView(_: MLNMapView, didChangeLocationManagerAuthorization manager: MLNLocationManager) {
        guard let accuracySetting = manager.accuracyAuthorization else {
            return
        }

        switch accuracySetting() {
        case .fullAccuracy:
            mapViewModel.locationAccuracy = .fullAccuracy
        case .reducedAccuracy:
            mapViewModel.locationAccuracy = .reducedAccuracy
        @unknown default:
            mapViewModel.locationAccuracy = .unknown
        }
    }

    // when a location is available for the first time, we fly to it
    func mapView(_ mapView: MLNMapView, didUpdate _: MLNUserLocation?) {
        guard !pannedToUserLocation else { return }
        guard let userLocation = mapView.userLocation else {
            print("User location is currently not available.")
            return
        }
        mapView.fly(to: MLNMapCamera(lookingAtCenter: userLocation.coordinate, altitude: 100_000, pitch: 0, heading: 0))
        pannedToUserLocation = true
    }
}

struct PrivacyExampleRepresentable: UIViewRepresentable {
    @ObservedObject var mapViewModel: PrivacyExampleViewModel

    func makeCoordinator() -> PrivacyExampleCoordinator {
        PrivacyExampleCoordinator(mapViewModel: mapViewModel)
    }

    func makeUIView(context: Context) -> MLNMapView {
        let mapView = MLNMapView()

        mapView.delegate = context.coordinator
        mapView.showsUserLocation = true
        return mapView
    }

    func updateUIView(_ mapView: MLNMapView, context _: Context) {
        if mapViewModel.showTemporaryLocationAuthorization {
            let purposeKey = "MLNAccuracyAuthorizationDescription"
            mapView.locationManager.requestTemporaryFullAccuracyAuthorization?(withPurposeKey: purposeKey)
            DispatchQueue.main.async {
                mapViewModel.showTemporaryLocationAuthorization = false
            }
        }
    }
}

struct LocationPrivacyExampleView: View {
    @StateObject private var viewModel = PrivacyExampleViewModel()

    var body: some View {
        VStack {
            PrivacyExampleRepresentable(mapViewModel: viewModel)
                .edgesIgnoringSafeArea(.all)

            if viewModel.locationAccuracy == LocationAccuracyState.reducedAccuracy {
                Button("Request Precise Location") {
                    viewModel.showTemporaryLocationAuthorization.toggle()
                }
                .padding()
                .background(Color.blue)
                .foregroundColor(.white)
                .cornerRadius(8)
            }
        }
    }
}
```
