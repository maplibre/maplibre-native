import MapLibre
import SwiftUI
import UIKit

// #-example-code(LocationPrivacyExample)
enum LocationAccuracyState {
    case unknown
    case reducedAccuracy
    case fullAccuracy
}

class MapViewModel: NSObject, ObservableObject {
    // Weak because the coordinator has a strong reference to the view model
    weak var mapCoordinater: MapLibreRepresentableCoordinator?
    @MainActor @Published var locationAccuracy: LocationAccuracyState = .unknown

    @MainActor func requestTemporaryLocationAuthorization() {
        print("Requesting precice location")

        switch locationAccuracy {
        case .reducedAccuracy:
            let purposeKey = "MLNAccuracyAuthorizationDescription"
            mapCoordinater?.mapView?.locationManager.requestTemporaryFullAccuracyAuthorization!(withPurposeKey: purposeKey)
        default:
            break
        }
    }
}

class MapLibreRepresentableCoordinator: NSObject, MLNMapViewDelegate {
    private var mapViewModel: MapViewModel
    // Weak reference because SwiftUI owns the strong reference
    private(set) weak var mapView: MLNMapView?
    private var pannedToUserLocation = false

    init(mapViewModel: MapViewModel) {
        self.mapViewModel = mapViewModel
        super.init()
        self.mapViewModel.mapCoordinater = self
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

    func mapView(_ mapView: MLNMapView, didFinishLoading _: MLNStyle) {
        if self.mapView != mapView {
            self.mapView = mapView
        }
    }

    func mapViewDidFinishLoadingMap(_ mapView: MLNMapView) {
        if self.mapView != mapView {
            self.mapView = mapView
        }
    }

    deinit {
        // Ensure the coordinator is deallocated if MapLibreViewRepresentable.dismantleUIView was called
    }
}

struct MapLibreViewRepresentable: UIViewRepresentable {
    let mapViewModel: MapViewModel

    func makeCoordinator() -> MapLibreRepresentableCoordinator {
        MapLibreRepresentableCoordinator(mapViewModel: mapViewModel)
    }

    func makeUIView(context: Context) -> MLNMapView {
        let mapView = MLNMapView()
        // Set delegate first, otherwise delegate callbacks can be missed. For example, setting showsUserLocation calls mapView:didUpdate:userLocation
        mapView.delegate = context.coordinator
        mapView.showsUserLocation = true
        return mapView
    }

    func updateUIView(_: MLNMapView, context _: Context) {
        // Be careful with doing any real work in updateUIView, this is called a lot.
    }

    static func dismantleUIView(_: MLNMapView, coordinator _: Coordinator) {
        // Verify that dismantleUIView is called when MLNMapView is out of the view hierarchy and deallocated.
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
                    mapViewModel.requestTemporaryLocationAuthorization()
                }
                .padding()
                .background(Color.blue)
                .foregroundColor(.white)
                .cornerRadius(8)
            }
        }
    }
}

#if DEBUG
    #Preview {
        LocationPrivacyExampleView()
    }
#endif

// #-end-example-code
