import Mapbox
import UIKit

let loc1 = CLLocationCoordinate2D(latitude: 37.77554, longitude: -122.41248)
let loc2 = CLLocationCoordinate2D(latitude: 37.74958, longitude: -122.46823)

class ViewController: UIViewController, MGLMapViewDelegate {

    private var mapView: MGLMapView!
    private var markerSource: MGLShapeSource?
    private var contentInsetsSource: MGLShapeSource?
    private var cameraEdgeInsetsSource: MGLShapeSource?
    private var buttons = [UIButton]()

    override func viewDidLoad() {
        super.viewDidLoad()

        // retrieve MapTiler key from property list
        let mapTilerKey = getMapTilerkey()

        // construct style URL
        let styleURL = URL(string: "https://api.maptiler.com/maps/streets/style.json?key=\(mapTilerKey)")

        self.mapView = MGLMapView(frame: view.bounds, styleURL: styleURL)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.delegate = self
        mapView.logoView.isHidden = true
        mapView.allowsRotating = false
        view.addSubview(mapView)

        let button1 = UIButton()
        button1.setTitle("symmetrical", for: .normal)
        button1.setTitleColor(.systemBlue, for: .normal)
        button1.backgroundColor = .systemBackground
        button1.addTarget(self, action: #selector(symmetricalPaddingTapped), for: .touchUpInside)

        let button2 = UIButton()
        button2.setTitle("asymmetrical", for: .normal)
        button2.setTitleColor(.systemBlue, for: .normal)
        button2.backgroundColor = .systemBackground
        button2.addTarget(self, action: #selector(asymmetricalPaddingTapped), for: .touchUpInside)

        self.buttons = [button1, button2]
        let buttons = UIStackView(arrangedSubviews: self.buttons)
        buttons.translatesAutoresizingMaskIntoConstraints = false
        buttons.spacing = 32.0
        self.view.addSubview(buttons)
        NSLayoutConstraint.activate([
            buttons.bottomAnchor.constraint(equalTo: self.view.layoutMarginsGuide.bottomAnchor),
            buttons.centerXAnchor.constraint(equalTo: self.view.centerXAnchor)
        ])

        self.buttons.forEach { $0.isEnabled = false }

        // Set the map’s center coordinate and zoom level.
        mapView.setCenter(
            loc1,
            zoomLevel: 10,
            animated: false
        )
    }

    @objc
    func symmetricalPaddingTapped() {
        let padding = UIEdgeInsets(top: 10, left: 10, bottom: 10, right: 10)
        self.setCameraWithPaddings(padding: padding)
    }

    @objc
    func asymmetricalPaddingTapped() {
        let padding = UIEdgeInsets(top: 10, left: 50, bottom: 200, right: 10)
        self.setCameraWithPaddings(padding: padding)
    }

    func mapView(_ mapView: MGLMapView, didFinishLoading style: MGLStyle) {
        self.createCenterMarker(mapView, style)
        self.drawTestRect(style)
        self.drawMapPaddings(style)
    }

    func mapViewDidFinishLoadingMap(_ mapView: MGLMapView) {
        mapView.setContentInset(UIEdgeInsets(top: 50.0, left: 30, bottom: 300, right: 30),
                                animated: true, completionHandler: nil)
        self.buttons.forEach { $0.isEnabled = true }
    }

    func mapViewDidFinishRenderingFrame(_ mapView: MGLMapView, fullyRendered: Bool) {
        let point = MGLPointAnnotation()
        point.coordinate = mapView.centerCoordinate
        self.markerSource?.shape = point

        self.contentInsetsSource?.shape = self.contentInsetsPolyline()
        self.cameraEdgeInsetsSource?.shape = self.cameraEdgeInsetsPolyline()
    }

    func setCameraWithPaddings(padding: UIEdgeInsets) {
        let b = CoordinateBounds(including: [loc1, loc2])
        let bounds = MGLCoordinateBounds(sw: b.southWest, ne: b.northEast)

        mapView.setVisibleCoordinateBounds(bounds,
                                           edgePadding: padding,
                                           animated: true,
                                           completionHandler: nil)
//        self.mapView.setContentInset(padding, animated: true, completionHandler: nil)
//
//        self.mapView.setCamera(
//            camera,
//            withDuration: 1,
//            animationTimingFunction: nil
//        )
    }

    func createCenterMarker(_ mapView: MGLMapView, _ style: MGLStyle) {
        // Create point to represent where the symbol should be placed
        let point = MGLPointAnnotation()
        point.coordinate = loc1

        // Create a data source to hold the point data
        let shapeSource = MGLShapeSource(identifier: "marker-source1", shape: point, options: nil)

        // Create a style layer for the symbol
        let shapeLayer = MGLSymbolStyleLayer(identifier: "marker-style1", source: shapeSource)

        // Add the image to the style's sprite
        if let image = UIImage(named: "_1") {
            style.setImage(image, forName: "home-symbol1")
        }

        // Tell the layer to use the image in the sprite
        shapeLayer.iconImageName = NSExpression(forConstantValue: "home-symbol1")

        // Add the source and style layer to the map
        style.addSource(shapeSource)
        style.addLayer(shapeLayer)

        self.markerSource = shapeSource
    }

    func contentInsetsPolyline() -> MGLPolyline {
        let paddingsRect = self.mapView.bounds.inset(by: self.mapView.contentInset)
        let b = self.mapView.convert(paddingsRect, toCoordinateBoundsFrom: self.mapView)
        let coordinates = [
            b.sw,
            CLLocationCoordinate2D(latitude: b.sw.latitude, longitude: b.ne.longitude),
            b.ne,
            CLLocationCoordinate2D(latitude: b.ne.latitude, longitude: b.sw.longitude),
            b.sw
        ]
        return MGLPolyline(coordinates: coordinates, count: UInt(coordinates.count))
    }

    func cameraEdgeInsetsPolyline() -> MGLPolyline {
        let paddingsRect = self.mapView.bounds.inset(by: self.mapView.cameraEdgeInsets)
        let b = self.mapView.convert(paddingsRect, toCoordinateBoundsFrom: self.mapView)
        let coordinates = [
            b.sw,
            CLLocationCoordinate2D(latitude: b.sw.latitude, longitude: b.ne.longitude),
            b.ne,
            CLLocationCoordinate2D(latitude: b.ne.latitude, longitude: b.sw.longitude),
            b.sw
        ]
        return MGLPolyline(coordinates: coordinates, count: UInt(coordinates.count))
    }

    func drawMapPaddings(_ style: MGLStyle) {
        // Content insets
        var polyline = self.contentInsetsPolyline()
        var source = MGLShapeSource(identifier: "contentInset", shape: polyline, options: nil)
        self.contentInsetsSource = source
        style.addSource(source)

        var layer = MGLLineStyleLayer(identifier: "contentInset", source: source)
        layer.lineColor = NSExpression(forConstantValue: UIColor.red)
        layer.lineWidth = NSExpression(forConstantValue: 1.0)
        style.addLayer(layer)


        // Camera edge insets
        polyline = self.cameraEdgeInsetsPolyline()
        source = MGLShapeSource(identifier: "cameraEdgeInsets", shape: polyline, options: nil)
        self.cameraEdgeInsetsSource = source
        style.addSource(source)

        layer = MGLLineStyleLayer(identifier: "cameraEdgeInsets", source: source)
        layer.lineColor = NSExpression(forConstantValue: UIColor.blue)
        layer.lineWidth = NSExpression(forConstantValue: 1.0)
        style.addLayer(layer)
    }

    func drawTestRect(_ style: MGLStyle) {
        let b = MGLCoordinateBounds(sw: loc1, ne: loc2)
        let coordinates = [
            b.sw,
            CLLocationCoordinate2D(latitude: b.sw.latitude, longitude: b.ne.longitude),
            b.ne,
            CLLocationCoordinate2D(latitude: b.ne.latitude, longitude: b.sw.longitude),
            b.sw
        ]

        let polyline = MGLPolyline(coordinates: coordinates, count: UInt(coordinates.count))
        let source = MGLShapeSource(identifier: "test", shape: polyline, options: nil)
        style.addSource(source)

        let layer = MGLLineStyleLayer(identifier: "test", source: source)
        layer.lineColor = NSExpression(forConstantValue: UIColor.black)
        layer.lineWidth = NSExpression(forConstantValue: 1.0)
        style.addLayer(layer)
    }
}

func getMapTilerkey() -> String {
    let mapTilerKey = Bundle.main.object(forInfoDictionaryKey: "MapTilerKey") as? String
    validateKey(mapTilerKey)
    return mapTilerKey!
}

func validateKey(_ mapTilerKey: String?) {
    if (mapTilerKey == nil) {
        preconditionFailure("Failed to read MapTiler key from info.plist")
    }
    let result: ComparisonResult = mapTilerKey!.compare("placeholder", options: NSString.CompareOptions.caseInsensitive, range: nil, locale: nil)
    if result == .orderedSame {
        preconditionFailure("Please enter correct MapTiler key in info.plist[MapTilerKey] property")
    }
}
