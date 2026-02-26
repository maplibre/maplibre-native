import CoreLocation
import MapLibre
import SwiftUI

let maxPitch: Double = 90

struct CameraSliderExample: View {
    @State private var pitch: Double = 0
    @State private var roll: Double = 0
    private let pitchRange: ClosedRange<Double> = 0 ... maxPitch
    private let rollRange: ClosedRange<Double> = -180 ... 180

    var body: some View {
        ZStack(alignment: .top) {
            OrientationAdjustableMap(pitch: $pitch, roll: $roll)
                .edgesIgnoringSafeArea(.all)

            VStack(alignment: .leading, spacing: 12) {
                sliderRow(label: "Pitch", value: $pitch, range: pitchRange)
                sliderRow(label: "Roll", value: $roll, range: rollRange)
            }
            .padding()
            .background(.ultraThinMaterial)
            .cornerRadius(12)
            .shadow(color: Color.black.opacity(0.15), radius: 6, x: 0, y: 3)
            .padding([.top, .horizontal])
        }
        .navigationTitle("Pitch & Roll")
        .navigationBarTitleDisplayMode(.inline)
    }

    private func sliderRow(label: String, value: Binding<Double>, range: ClosedRange<Double>) -> some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("\(label): \(Int(value.wrappedValue)) deg")
                .font(.headline)
            Slider(value: value, in: range, step: 1)
                .accentColor(MapLibreColors.accent)
        }
    }
}

private struct OrientationAdjustableMap: UIViewRepresentable {
    @Binding var pitch: Double
    @Binding var roll: Double

    func makeCoordinator() -> Coordinator {
        Coordinator(pitch: $pitch, roll: $roll)
    }

    func makeUIView(context: Context) -> MLNMapView {
        let styleJSON = """
        {
          "version": 8,
          "name": "Satellite",
          "sources": {
            "satellite-source": {
              "type": "raster",
              "tiles": ["https://versatiles-satellite.b-cdn.net/tiles/orthophotos/{z}/{x}/{y}"],
              "tileSize": 256,
              "attribution": "Versatiles"
            }
          },
          "layers": [
            {
              "id": "satellite-layer",
              "type": "raster",
              "source": "satellite-source"
            }
          ]
        }
        """

        let mapView = MLNMapView(frame: .zero)
        mapView.styleJSON = styleJSON
        mapView.delegate = context.coordinator
        mapView.maximumPitch = CGFloat(maxPitch)
        let center = CLLocationCoordinate2D(latitude: 40.7128, longitude: -74.0060)
        mapView.setCenter(center, zoomLevel: 10, animated: false)
        context.coordinator.sync(from: mapView.camera)
        return mapView
    }

    func updateUIView(_ mapView: MLNMapView, context _: Context) {
        let currentCamera = mapView.camera
        let targetPitch = CGFloat(pitch)
        let targetRoll = CGFloat(roll)

        let pitchDelta = abs(currentCamera.pitch - targetPitch)
        let rollDelta = abs(currentCamera.roll - targetRoll)

        if pitchDelta < 0.1, rollDelta < 0.1 {
            return
        }

        let currentZoom = mapView.zoomLevel
        let targetAltitude = MLNAltitudeForZoomLevel(currentZoom, targetPitch, currentCamera.centerCoordinate.latitude, mapView.bounds.size)

        let updatedCamera = MLNMapCamera(
            lookingAtCenter: currentCamera.centerCoordinate,
            altitude: targetAltitude,
            pitch: targetPitch,
            heading: currentCamera.heading
        )
        updatedCamera.roll = targetRoll
        mapView.setCamera(updatedCamera, animated: false)
    }

    final class Coordinator: NSObject, MLNMapViewDelegate {
        @Binding private var pitch: Double
        @Binding private var roll: Double

        init(pitch: Binding<Double>, roll: Binding<Double>) {
            _pitch = pitch
            _roll = roll
        }

        func sync(from camera: MLNMapCamera) {
            let newPitch = Double(camera.pitch)
            let newRoll = Double(camera.roll)
            if abs(pitch - newPitch) >= 0.1 {
                pitch = newPitch
            }
            if abs(roll - newRoll) >= 0.1 {
                roll = newRoll
            }
        }
    }
}
