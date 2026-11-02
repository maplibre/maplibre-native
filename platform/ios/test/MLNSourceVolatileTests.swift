import MapLibre
import XCTest

/// Tests that ``MLNSource/isVolatile`` defaults to `false` and keeps its value
/// when the source is added to and removed from the style.
class MLNSourceVolatileTests: XCTestCase, MLNMapViewDelegate {
    var mapView: MLNMapView!
    var styleLoadingExpectation: XCTestExpectation?

    override func setUp() {
        super.setUp()

        guard let styleURL = Bundle(for: type(of: self)).url(forResource: "one-liner", withExtension: "json") else {
            XCTFail("Missing one-liner.json style")
            return
        }

        mapView = MLNMapView(frame: CGRect(x: 0, y: 0, width: 256, height: 256), styleURL: styleURL)
        mapView.delegate = self

        if mapView.style == nil {
            styleLoadingExpectation = expectation(description: "Map view should finish loading style")
            wait(for: [styleLoadingExpectation!], timeout: 10)
        }
    }

    override func tearDown() {
        styleLoadingExpectation = nil
        mapView = nil
        super.tearDown()
    }

    func mapView(_: MLNMapView, didFinishLoading _: MLNStyle) {
        styleLoadingExpectation?.fulfill()
        styleLoadingExpectation = nil
    }

    func testVolatileIsOffByDefaultAndSurvivesAddingToTheStyle() {
        guard let style = mapView.style else {
            XCTFail("Style should be loaded")
            return
        }

        let source = MLNRasterTileSource(
            identifier: "volatile-raster",
            tileURLTemplates: ["https://example.com/{z}/{x}/{y}.png"],
            options: nil
        )
        XCTAssertFalse(source.isVolatile)

        source.isVolatile = true
        XCTAssertTrue(source.isVolatile)

        style.addSource(source)
        let added = style.source(withIdentifier: "volatile-raster")
        XCTAssertEqual(added?.isVolatile, true)

        added?.isVolatile = false
        XCTAssertFalse(source.isVolatile)

        style.removeSource(source)
        XCTAssertFalse(source.isVolatile)
    }
}
