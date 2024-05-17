import MapLibre
import SwiftUI
import UIKit

// #-example-code(LocationPrivacyExample)
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

// #-end-example-code
