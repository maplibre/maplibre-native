import MapLibre
import SwiftUI
import UIKit

// #-example-code(LocationPrivacyExample)
enum LocationAccuracyState {
    case unknown
    case reducedAccuracy
    case fullAccuracy
}

@MainActor
class MapViewModel: NSObject, ObservableObject {
    @Published var locationAccuracy: LocationAccuracyState = .unknown
    @Published var showTemporaryLocationAuthorization = false
}

class MapLibreRepresentableCoordinator: NSObject, MLNMapViewDelegate {
    @ObservedObject private var mapViewModel: MapViewModel
    private var pannedToUserLocation = false
    
    init(mapViewModel: MapViewModel) {
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

struct MapLibreViewRepresentable: UIViewRepresentable {
    @ObservedObject var mapViewModel: MapViewModel
    
    func makeCoordinator() -> MapLibreRepresentableCoordinator {
        MapLibreRepresentableCoordinator(mapViewModel: mapViewModel)
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
        }
    }
}

struct LocationPrivacyExampleView: View {
    @StateObject private var mapViewModel = MapViewModel()
    
    var body: some View {
        VStack {
            MapLibreViewRepresentable(mapViewModel: mapViewModel)
                .edgesIgnoringSafeArea(.all)

            if mapViewModel.locationAccuracy == LocationAccuracyState.reducedAccuracy {
                Button("Request Precise Location") {
                    mapViewModel.showTemporaryLocationAuthorization.toggle()
                }
                .padding()
                .background(Color.blue)
                .foregroundColor(.white)
                .cornerRadius(8)
            }
        }
    }
}

// #-end-example-code
