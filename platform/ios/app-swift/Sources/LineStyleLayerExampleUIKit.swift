import MapLibre
import SwiftUI
import UIKit

// #-example-code(LineStyleLayerExample)
class LineStyleLayerExample: UIViewController, MLNMapViewDelegate {
    var mapView: MLNMapView!

    override func viewDidLoad() {
        super.viewDidLoad()

        mapView = MLNMapView(frame: view.bounds, styleURL: VERSATILES_COLORFUL_STYLE)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]

        mapView.setCenter(
            CLLocationCoordinate2D(latitude: 45.5076, longitude: -122.6736),
            zoomLevel: 11,
            animated: false
        )
        view.addSubview(mapView)

        mapView.delegate = self
    }

    // Wait until the map is loaded before adding to the map.
    func mapView(_: MLNMapView, didFinishLoading _: MLNStyle) {
        loadGeoJson()
    }

    func loadGeoJson() {
        DispatchQueue.global().async {
            // Get the path for example.geojson in the app’s bundle.
            guard let jsonUrl = Bundle.main.url(forResource: "example", withExtension: "geojson") else {
                preconditionFailure("Failed to load local GeoJSON file")
            }

            guard let jsonData = try? Data(contentsOf: jsonUrl) else {
                preconditionFailure("Failed to parse GeoJSON file")
            }

            DispatchQueue.main.async {
                self.drawPolyline(geoJson: jsonData)
            }
        }
    }

    func drawPolyline(geoJson: Data) {
        // Add our GeoJSON data to the map as an MLNGeoJSONSource.
        // We can then reference this data from an MLNStyleLayer.

        // MLNMapView.style is optional, so you must guard against it not being set.
        guard let style = mapView.style else { return }

        guard let shapeFromGeoJSON = try? MLNShape(data: geoJson, encoding: String.Encoding.utf8.rawValue) else {
            fatalError("Could not generate MLNShape")
        }

        let source = MLNShapeSource(identifier: "polyline", shape: shapeFromGeoJSON, options: nil)
        style.addSource(source)

        // Create new layer for the line.
        let layer = MLNLineStyleLayer(identifier: "polyline", source: source)

        // Set the line join and cap to a rounded end.
        layer.lineJoin = NSExpression(forConstantValue: "round")
        layer.lineCap = NSExpression(forConstantValue: "round")

        // Set the line color to a constant blue color.
        layer.lineColor = NSExpression(forConstantValue: UIColor(red: 59 / 255, green: 178 / 255, blue: 208 / 255, alpha: 1))

        // Use `NSExpression` to smoothly adjust the line width from 2pt to 20pt between zoom levels 14 and 18. The `interpolationBase` parameter allows the values to interpolate along an exponential curve.
        layer.lineWidth = NSExpression(forMLNInterpolating: NSExpression.zoomLevelVariable, curveType: MLNExpressionInterpolationMode.linear, parameters: nil, stops: NSExpression(forConstantValue: [14: 2, 18: 20]))

        // We can also add a second layer that will draw a stroke around the original line.
        let casingLayer = MLNLineStyleLayer(identifier: "polyline-case", source: source)
        // Copy these attributes from the main line layer.
        casingLayer.lineJoin = layer.lineJoin
        casingLayer.lineCap = layer.lineCap
        // Line gap width represents the space before the outline begins, so should match the main line’s line width exactly.
        casingLayer.lineGapWidth = layer.lineWidth
        // Stroke color slightly darker than the line color.
        casingLayer.lineColor = NSExpression(forConstantValue: UIColor(red: 41 / 255, green: 145 / 255, blue: 171 / 255, alpha: 1))
        // Use `NSExpression` to gradually increase the stroke width between zoom levels 14 and 18.
        casingLayer.lineWidth = NSExpression(forMLNInterpolating: NSExpression.zoomLevelVariable, curveType: MLNExpressionInterpolationMode.linear, parameters: nil, stops: NSExpression(forConstantValue: [14: 1, 18: 4]))

        // Just for fun, let’s add another copy of the line with a dash pattern.
        let dashedLayer = MLNLineStyleLayer(identifier: "polyline-dash", source: source)
        dashedLayer.lineJoin = layer.lineJoin
        dashedLayer.lineCap = layer.lineCap
        dashedLayer.lineColor = NSExpression(forConstantValue: UIColor.white)
        dashedLayer.lineOpacity = NSExpression(forConstantValue: 0.5)
        dashedLayer.lineWidth = layer.lineWidth
        // Dash pattern in the format [dash, gap, dash, gap, ...]. You’ll want to adjust these values based on the line cap style.
        dashedLayer.lineDashPattern = NSExpression(forConstantValue: [0, 1.5])

        style.addLayer(layer)
        style.addLayer(dashedLayer)
        style.insertLayer(casingLayer, below: layer)
    }
}

// #-end-example-code

struct LineStyleLayerExampleUIViewControllerRepresentable: UIViewControllerRepresentable {
    typealias UIViewControllerType = LineStyleLayerExample

    func makeUIViewController(context _: Context) -> LineStyleLayerExample {
        LineStyleLayerExample()
    }

    func updateUIViewController(_: LineStyleLayerExample, context _: Context) {}
}
