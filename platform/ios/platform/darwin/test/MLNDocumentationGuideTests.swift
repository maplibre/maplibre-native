import XCTest
import MapLibre

/**
 Test cases that ensure the inline examples in the jazzy guides compile.

 To add an example:
 1. Add a test case named in the form `testGuideName$ExampleName`.
 2. Wrap the code you’d like to appear in the documentation within the
    following comment blocks:
    ```
    //#-example-code
    ...
    //#-end-example-code
    ```
 3. Insert a call to `guideExample()`  where you’d like the example code to be
    inserted in the guide’s Markdown.
    ```
    <%- guideExample('GuideName', 'ExampleName', 'iOS') %>
    ```
 4. Run `make darwin-style-code` to extract example code from the test method
    below and insert it into the guide.
 */
class MLNDocumentationGuideTests: XCTestCase, MLNMapViewDelegate {
    var mapView: MLNMapView!
    var styleLoadingExpectation: XCTestExpectation!
    
    override func setUp() {
        super.setUp()
        let styleURL = Bundle(for: MLNDocumentationGuideTests.self).url(forResource: "one-liner", withExtension: "json")
        mapView = MLNMapView(frame: CGRect(x: 0, y: 0, width: 256, height: 256), styleURL: styleURL)
        mapView.delegate = self
        styleLoadingExpectation = expectation(description: "Map view should finish loading style")
        waitForExpectations(timeout: 10, handler: nil)
    }
    
    override func tearDown() {
        mapView = nil
        styleLoadingExpectation = nil
        super.tearDown()
    }
    
    func mapView(_ mapView: MLNMapView, didFinishLoading style: MLNStyle) {
        styleLoadingExpectation.fulfill()
    }
    
    func testMigratingToExpressions$Stops() {
        //#-example-code
        // Swift sample on how to populate a stepping expression with multiple stops.
        // Create a color ramp.
        #if os(macOS)
            let stops: [NSNumber: NSColor] = [
                0: .yellow,
                2.5: .orange,
                5: .red,
                7.5: .blue,
                10: .white,
            ]
        #else
            let stops: [NSNumber: UIColor] = [
                0: .yellow,
                2.5: .orange,
                5: .red,
                7.5: .blue,
                10: .white,
            ]
        #endif

        // Based on the zoom and `stops`, change the color.
        var functionExpression = NSExpression(forMLNStepping: .zoomLevelVariable,
                                              from: NSExpression(forConstantValue: stops[0]),
                                              stops: NSExpression(forConstantValue: stops))

        // Based on zoom and `stopsLineWidth`, set the Line width.
        let initialValue = 4.0
        let stopsLineWidth = [
            11.0: initialValue,
            14.0: 6.0,
            20.0: 18.0]
        
        functionExpression = NSExpression(
            forMLNStepping: .zoomLevelVariable,
            from: NSExpression(forConstantValue: initialValue),
            stops: NSExpression(forConstantValue: stopsLineWidth)
        )
        //#-end-example-code
        print(functionExpression)
        print(stopsLineWidth)
    }
    
    func testMigratingToExpressions$Linear() {
        //#-example-code
        let url = URL(string: "https://earthquake.usgs.gov/earthquakes/feed/v1.0/summary/all_week.geojson")!
        let symbolSource = MLNSource(identifier: "source")
        let symbolLayer = MLNSymbolStyleLayer(identifier: "place-city-sm", source: symbolSource)
        
        let source = MLNShapeSource(identifier: "earthquakes", url: url, options: nil)
        let mag = 1.0  // Update based on earthquake GeoJSON data
        mapView.style?.addSource(source)
        
        #if os(macOS)
            let stops: [NSNumber: NSColor] = [
                0: .yellow,
                2.5: .orange,
                5: .red,
                7.5: .blue,
                10: .white,
            ]
        #else
            let stops: [NSNumber: UIColor] = [
                0: .yellow,
                2.5: .orange,
                5: .red,
                7.5: .blue,
                10: .white,
            ]
        #endif
        
        let layer = MLNCircleStyleLayer(identifier: "circles", source: source)
        
        let circleExpression : NSExpression
        if #available(iOS 15, *) {
            circleExpression = NSExpression(
                forMLNInterpolating: NSExpression(forConstantValue: mag),
                curveType: .linear,
                parameters: nil,
                stops: NSExpression(forConstantValue: stops))
        } else {
            // This works up to iOS 14.5
            circleExpression = NSExpression(
                format: "mgl_interpolate:withCurveType:parameters:stops:(mag, 'linear', nil, %@)",
                stops)
        }
        
        layer.circleColor = circleExpression
        layer.circleRadius = NSExpression(forConstantValue: 10)
        mapView.style?.insertLayer(layer, below: symbolLayer)
        //#-end-example-code
        print(circleExpression)
    }
    
    func testMigratingToExpressions$LinearConvenience() {
        let source = MLNShapeSource(identifier: "circles", shape: nil, options: nil)
        let layer = MLNCircleStyleLayer(identifier: "circles", source: source)
        
        #if os(macOS)
        let stops: [NSNumber: NSColor] = [
            0: .yellow,
            2.5: .orange,
            5: .red,
            7.5: .blue,
            10: .white,
            ]
        #else
        let stops: [NSNumber: UIColor] = [
            0: .yellow,
            2.5: .orange,
            5: .red,
            7.5: .blue,
            10: .white,
            ]
        #endif
        
        //#-example-code
        layer.circleColor = NSExpression(forMLNInterpolating: NSExpression(forKeyPath: "mag"), curveType: .linear, parameters: nil, stops: NSExpression(forConstantValue: stops))
        //#-end-example-code
        
        layer.circleRadius = NSExpression(forConstantValue: 10)
        mapView.style?.addLayer(layer)
        
    }
    func testMigratingToExpressions$Exponential() {
        let source = MLNShapeSource(identifier: "circles", shape: nil, options: nil)
        let layer = MLNCircleStyleLayer(identifier: "circles", source: source)
        
        //#-example-code
        let stops = [
            12: 0.5,
            14: 2,
            18: 18,
        ]
        
        layer.circleRadius = NSExpression(forMLNInterpolating: .zoomLevelVariable,
                                          curveType: .exponential,
                                          parameters: NSExpression(forConstantValue: 1.5),
                                          stops: NSExpression(forConstantValue: stops))
        //#-end-example-code
    }
    
    func testMigratingToExpressions$ExponentialConvenience() {
        let source = MLNShapeSource(identifier: "circles", shape: nil, options: nil)
        let layer = MLNCircleStyleLayer(identifier: "circles", source: source)
        
        //#-example-code
        let stops = [
            12: 0.5,
            14: 2,
            18: 18,
            ]
        
        layer.circleRadius =  NSExpression(forMLNInterpolating: NSExpression.zoomLevelVariable, curveType: MLNExpressionInterpolationMode.exponential, parameters: NSExpression(forConstantValue: 1.5), stops: NSExpression(forConstantValue: stops))
        //#-end-example-code
    }
    func testMigratingToExpressions$Interval() {
        let source = MLNShapeSource(identifier: "circles", shape: nil, options: nil)
        let layer = MLNCircleStyleLayer(identifier: "circles", source: source)
        
        //#-example-code
        #if os(macOS)
            let stops: [NSNumber: NSColor] = [
                0: .yellow,
                2.5: .orange,
                5: .red,
                7.5: .blue,
                10: .white,
            ]
            
            layer.circleColor = NSExpression(forMLNStepping: .zoomLevelVariable,
                                             from: NSExpression(forConstantValue: NSColor.green),
                                             stops: NSExpression(forConstantValue: stops))
        #else
            let stops: [NSNumber: UIColor] = [
                0: .yellow,
                2.5: .orange,
                5: .red,
                7.5: .blue,
                10: .white,
            ]
            
            layer.circleColor = NSExpression(forMLNStepping: .zoomLevelVariable,
                                             from: NSExpression(forConstantValue: UIColor.green),
                                             stops: NSExpression(forConstantValue: stops))
        #endif
        //#-end-example-code
    }
    
    func testMigratingToExpressions$Categorical() {
        let source = MLNShapeSource(identifier: "circles", shape: nil, options: nil)
        let layer = MLNCircleStyleLayer(identifier: "circles", source: source)
        
        //#-example-code
        // Category type
        let type = NSExpression(forConstantValue: "type")

        // Categories
        let earthquake = NSExpression(forConstantValue: "earthquake")
        let explosion = NSExpression(forConstantValue: "explosion")
        let quarryBlast = NSExpression(forConstantValue: "quarry blast")

        #if os(macOS)
        let defaultColor = NSExpression(forConstantValue: NSColor.blue)
        let orange = NSExpression(forConstantValue: NSColor.orange)
        let red = NSExpression(forConstantValue: NSColor.red)
        let yellow = NSExpression(forConstantValue: NSColor.yellow)
        
        layer.circleColor = NSExpression(forMLNMatchingKey: type,
                                         in: [earthquake:orange, explosion:red, quarryBlast:yellow],
                                         default: defaultColor)
        #else
        let defaultColor = NSExpression(forConstantValue: UIColor.blue)
        let orange = NSExpression(forConstantValue: UIColor.orange)
        let red = NSExpression(forConstantValue: UIColor.red)
        let yellow = NSExpression(forConstantValue: UIColor.yellow)


        XCTExpectFailure("Awaiting unit test refactoring for https://github.com/maplibre/maplibre-native/issues/331")
        layer.circleColor = NSExpression(forMLNMatchingKey: type,
                                         in: [earthquake:orange, explosion:red, quarryBlast:yellow],
                                         default: defaultColor)
        #endif
        //#-end-example-code
    }
    
    func testMigratingToExpressions$CategoricalValue() {
        let source = MLNShapeSource(identifier: "circles", shape: nil, options: nil)
        let layer = MLNCircleStyleLayer(identifier: "circles", source: source)
        
        //#-example-code
        #if os(macOS)
        let stops : [String : NSColor] = ["earthquake" : NSColor.orange,
                                          "explosion" : NSColor.red,
                                          "quarry blast" : NSColor.yellow]
        layer.circleColor = NSExpression(
            format: "FUNCTION(%@, 'valueForKeyPath:', type)",
            stops)
        #else
        let stops : [String : UIColor] = ["earthquake" : UIColor.orange,
                                          "explosion" : UIColor.red,
                                          "quarry blast" : UIColor.yellow]
        layer.circleColor = NSExpression(
            format: "FUNCTION(%@, 'valueForKeyPath:', type)",
            stops)
        #endif
        //#-end-example-code
    }
    func testMigratingToExpressions$Identity() {
        let source = MLNShapeSource(identifier: "circles", shape: nil, options: nil)
        let layer = MLNCircleStyleLayer(identifier: "circles", source: source)
        
        //#-example-code
        layer.circleRadius = NSExpression(forKeyPath: "mag")
        //#-end-example-code
    }
    
    func testMigratingToExpressions$Multiply() {
        let source = MLNShapeSource(identifier: "circles", shape: nil, options: nil)
        let layer = MLNCircleStyleLayer(identifier: "circles", source: source)
        
        //#-example-code
        layer.circleRadius = NSExpression(forFunction: "multiply:by:", arguments: [NSExpression(forKeyPath: "mag"), 3])
        //#-end-example-code
    }
    
    func testMigratingToExpressions$Cast() {
        let source = MLNShapeSource(identifier: "circles", shape: nil, options: nil)
        
        //#-example-code
        let magnitudeLayer = MLNSymbolStyleLayer(identifier: "mag-layer", source: source)
        magnitudeLayer.text = NSExpression(format: "CAST(mag, 'NSString')")
        mapView.style?.addLayer(magnitudeLayer)
        //#-end-example-code
    }
    
    // MARK: Swift Example Code for Expected Failures
    
    func testExpectPassSwift() {
        XCTAssertNotNil(1)
    }

    /// https://developer.apple.com/documentation/xctest/expected_failures
    func testExpectFailureSwift() {
        XCTExpectFailure("Swift example - Anticipate known test failures to prevent failing tests from affecting your workflows.")
        XCTAssertNotNil(nil)
    }
}
