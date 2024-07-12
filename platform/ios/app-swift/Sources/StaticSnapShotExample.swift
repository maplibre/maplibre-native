import MapLibre
import SwiftUI
import UIKit

// #-example-code(StaticSnapshotExample)
class StaticSnapshotExample: UIViewController, MLNMapViewDelegate {
    var mapView: MLNMapView!
    var button: UIButton!
    var imageView: UIImageView!

    override func viewDidLoad() {
        super.viewDidLoad()

        mapView = MLNMapView(frame: CGRect(x: 0, y: 0, width: view.bounds.width, height: view.bounds.height / 2), styleURL: AMERICANA_STYLE)
        mapView.autoresizingMask = [.flexibleHeight, .flexibleWidth]

        // Center map on the Giza Pyramid Complex in Egypt.
        let center = CLLocationCoordinate2D(latitude: 29.9773, longitude: 31.1325)
        mapView.setCenter(center, zoomLevel: 14, animated: false)
        view.addSubview(mapView)

        // Create a button to take a map snapshot.
        button = UIButton(frame: CGRect(x: mapView.bounds.width / 2 - 40, y: mapView.bounds.height - 40, width: 80, height: 30))
        button.layer.cornerRadius = 15
        button.backgroundColor = UIColor(red: 0.96, green: 0.65, blue: 0.14, alpha: 1.0)
        button.setImage(UIImage(named: "camera"), for: .normal)
        button.addTarget(self, action: #selector(createSnapshot), for: .touchUpInside)
        view.addSubview(button)

        // Create a UIImageView that will store the map snapshot.
        imageView = UIImageView(frame: CGRect(x: 0, y: view.bounds.height / 2, width: view.bounds.width, height: view.bounds.height / 2))
        imageView.backgroundColor = .black
        imageView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        view.addSubview(imageView)
    }

    @objc func createSnapshot() {
        // Use the map's style, camera, size, and zoom level to set the snapshot's options.
        let options = MLNMapSnapshotOptions(styleURL: mapView.styleURL, camera: mapView.camera, size: mapView.bounds.size)
        options.zoomLevel = mapView.zoomLevel

        // Add an activity indicator to show that the snapshot is loading.
        let indicator = UIActivityIndicatorView(frame: CGRect(x: imageView.center.x - 30, y: imageView.center.y - 30, width: 60, height: 60))
        view.addSubview(indicator)
        indicator.startAnimating()

        // Create the map snapshot.
        let snapshotter: MLNMapSnapshotter? = MLNMapSnapshotter(options: options)
        snapshotter?.start { snapshot, error in
            if error != nil {
                print("Unable to create a map snapshot.")
            } else if let snapshot {
                // Add the map snapshot's image to the image view.
                indicator.stopAnimating()
                self.imageView.image = snapshot.image
            }
        }
    }
}

// #-end-example-code

struct StaticSnapshotExampleUIViewControllerRepresentable: UIViewControllerRepresentable {
    typealias UIViewControllerType = StaticSnapshotExample

    func makeUIViewController(context _: Context) -> StaticSnapshotExample {
        StaticSnapshotExample()
    }

    func updateUIViewController(_: StaticSnapshotExample, context _: Context) {}
}
