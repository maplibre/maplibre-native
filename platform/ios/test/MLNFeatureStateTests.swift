import MapLibre
import XCTest

/// Tests the feature state API of ``MLNShapeSource`` against a map view with a
/// live renderer. Feature state lives on the render source, which only exists
/// once a layer uses the source and a render pass has processed it.
class MLNFeatureStateTests: XCTestCase, MLNMapViewDelegate {
    var mapView: MLNMapView!
    var window: UIWindow!
    var styleLoadingExpectation: XCTestExpectation?
    var renderFrameExpectation: XCTestExpectation?

    override func setUp() {
        super.setUp()

        guard let styleURL = Bundle(for: type(of: self)).url(forResource: "one-liner", withExtension: "json") else {
            XCTFail("Missing one-liner.json style")
            return
        }

        mapView = MLNMapView(frame: CGRect(x: 0, y: 0, width: 256, height: 256), styleURL: styleURL)
        mapView.delegate = self

        window = UIWindow(frame: mapView.bounds)
        window.addSubview(mapView)
        window.makeKeyAndVisible()

        if mapView.style == nil {
            styleLoadingExpectation = expectation(description: "Map view should finish loading style")
            wait(for: [styleLoadingExpectation!], timeout: 10)
        }
    }

    override func tearDown() {
        styleLoadingExpectation = nil
        renderFrameExpectation = nil
        mapView = nil
        window = nil
        super.tearDown()
    }

    func mapView(_: MLNMapView, didFinishLoading _: MLNStyle) {
        styleLoadingExpectation?.fulfill()
        styleLoadingExpectation = nil
    }

    func mapViewDidFinishRenderingFrame(_: MLNMapView, fullyRendered _: Bool) {
        renderFrameExpectation?.fulfill()
        renderFrameExpectation = nil
    }

    private func waitForRenderedFrame() {
        renderFrameExpectation = expectation(description: "Map view should render a frame")
        wait(for: [renderFrameExpectation!], timeout: 10)
    }

    func testFeatureStateRoundTrip() {
        guard let style = mapView.style else {
            XCTFail("Style should be loaded")
            return
        }

        let feature = MLNPointFeature()
        feature.identifier = "feature-1"
        feature.coordinate = CLLocationCoordinate2D(latitude: 0, longitude: 0)

        let source = MLNShapeSource(identifier: "states", features: [feature], options: nil)
        style.addSource(source)
        style.addLayer(MLNCircleStyleLayer(identifier: "states-circles", source: source))

        // Wait for two full frames so the render source exists even if a frame
        // was already in flight when the layer was added.
        waitForRenderedFrame()
        waitForRenderedFrame()

        // Set state and read it back through the renderer.
        XCTAssertTrue(source.setFeatureState(featureID: "feature-1", state: ["selected": true]))
        XCTAssertEqual(source.featureState(featureID: "feature-1")?["selected"] as? Bool, true)

        // Merging keeps existing keys.
        XCTAssertTrue(source.setFeatureState(featureID: "feature-1", state: ["hovered": true]))
        let merged = source.featureState(featureID: "feature-1")
        XCTAssertEqual(merged?["selected"] as? Bool, true)
        XCTAssertEqual(merged?["hovered"] as? Bool, true)

        // Removing a single key leaves the rest in place. Unlike updates,
        // removals are only reflected once the next render pass has coalesced
        // the state changes, so wait for a frame before reading back.
        XCTAssertTrue(source.removeFeatureState(featureID: "feature-1", stateKey: "selected"))
        waitForRenderedFrame()
        let afterKeyRemoval = source.featureState(featureID: "feature-1")
        XCTAssertNil(afterKeyRemoval?["selected"])
        XCTAssertEqual(afterKeyRemoval?["hovered"] as? Bool, true)

        // Removing all state for the feature clears it.
        XCTAssertTrue(source.removeFeatureState(featureID: "feature-1"))
        waitForRenderedFrame()
        XCTAssertNil(source.featureState(featureID: "feature-1"))

        // Resetting removes state for every feature in the source.
        XCTAssertTrue(source.setFeatureState(featureID: "feature-1", state: ["selected": true]))
        XCTAssertTrue(source.resetFeatureStates())
        waitForRenderedFrame()
        XCTAssertNil(source.featureState(featureID: "feature-1"))
    }
}
