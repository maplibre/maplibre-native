import MapLibre
import SQLite3
import XCTest

/// Tests ``MLNSource/isVolatile`` against a map view with a live renderer. Tile
/// requests are answered by ``VolatileTileURLProtocol`` with a cacheable
/// response, and the ambient cache is then read back from its database.
class MLNSourceVolatileTests: XCTestCase, MLNMapViewDelegate, MLNNetworkConfigurationDelegate {
    var mapView: MLNMapView!
    var window: UIWindow!
    var styleLoadingExpectation: XCTestExpectation?
    /// Sources whose tiles had been requested when the map was last fully rendered.
    var fullyRenderedSourceIDs: Set<String> = []
    let session: URLSession = {
        let configuration = URLSessionConfiguration.ephemeral
        configuration.protocolClasses = [VolatileTileURLProtocol.self]
        return URLSession(configuration: configuration)
    }()

    override func setUp() {
        super.setUp()

        MLNNetworkConfiguration.sharedManager.delegate = self

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
        fullyRenderedSourceIDs = []
        mapView = nil
        window = nil
        MLNNetworkConfiguration.sharedManager.delegate = nil
        session.invalidateAndCancel()
        super.tearDown()
    }

    @objc(sessionForNetworkConfiguration:)
    func session(for _: MLNNetworkConfiguration) -> URLSession {
        session
    }

    func mapView(_: MLNMapView, didFinishLoading _: MLNStyle) {
        styleLoadingExpectation?.fulfill()
        styleLoadingExpectation = nil
    }

    func mapViewDidFinishRenderingFrame(_ mapView: MLNMapView, fullyRendered: Bool) {
        guard fullyRendered, let sources = mapView.style?.sources else {
            return
        }
        // A frame is only fully rendered once every requested tile has loaded.
        for source in sources where VolatileTileURLProtocol.hasRequested(containing: source.identifier) {
            fullyRenderedSourceIDs.insert(source.identifier)
        }
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

    func testVolatileSourceIsNotWrittenToTheAmbientCache() {
        guard let style = mapView.style else {
            XCTFail("Style should be loaded")
            return
        }

        // Load a tile of the volatile source and wait until it is on the map,
        // which means its response has passed the cache.
        let volatileID = UUID().uuidString
        let volatileSource = addRasterSource(volatileID, to: style, isVolatile: true)
        XCTAssertTrue(volatileSource.isVolatile)
        let rendered = expectation(for: NSPredicate { _, _ in
            self.fullyRenderedSourceIDs.contains(volatileID)
        }, evaluatedWith: nil)
        wait(for: [rendered], timeout: 10)

        // The cache database writes on a single thread in order, so once the
        // tile of a later, regular source is stored, a write for the volatile
        // tile would have happened already.
        let regularID = UUID().uuidString
        addRasterSource(regularID, to: style, isVolatile: false)
        waitForTileRequest(regularID)
        let stored = expectation(for: NSPredicate { _, _ in
            self.countOfCachedTiles(matching: regularID) > 0
        }, evaluatedWith: nil)
        wait(for: [stored], timeout: 10)

        XCTAssertEqual(countOfCachedTiles(matching: volatileID), 0)
    }

    @discardableResult
    private func addRasterSource(_ id: String, to style: MLNStyle, isVolatile: Bool) -> MLNRasterTileSource {
        let source = MLNRasterTileSource(
            identifier: id,
            tileURLTemplates: ["https://\(VolatileTileURLProtocol.host)/\(id)/{z}/{x}/{y}.png"],
            options: nil
        )
        source.isVolatile = isVolatile
        style.addSource(source)
        style.addLayer(MLNRasterStyleLayer(identifier: id, source: source))
        return source
    }

    private func waitForTileRequest(_ id: String) {
        let requested = expectation(for: NSPredicate { _, _ in
            VolatileTileURLProtocol.hasRequested(containing: id)
        }, evaluatedWith: nil)
        wait(for: [requested], timeout: 10)
    }

    /// Counts the tiles in the ambient cache whose URL template contains `id`.
    private func countOfCachedTiles(matching id: String) -> Int {
        var db: OpaquePointer?
        defer { sqlite3_close(db) }
        guard sqlite3_open_v2(MLNOfflineStorage.shared.databasePath, &db, SQLITE_OPEN_READONLY, nil) == SQLITE_OK else {
            return 0
        }
        sqlite3_busy_timeout(db, 1000)

        var statement: OpaquePointer?
        defer { sqlite3_finalize(statement) }
        guard sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM tiles WHERE url_template LIKE ?", -1, &statement, nil) == SQLITE_OK else {
            return 0
        }
        let transient = unsafeBitCast(-1, to: sqlite3_destructor_type.self)
        sqlite3_bind_text(statement, 1, "%\(id)%", -1, transient)
        guard sqlite3_step(statement) == SQLITE_ROW else {
            return 0
        }
        return Int(sqlite3_column_int64(statement, 0))
    }
}

/// Answers tile requests to ``host`` with a PNG that may be cached for a day.
class VolatileTileURLProtocol: URLProtocol {
    static let host = "volatile-source.test"

    private static let lock = NSLock()
    private static var requestedURLs: [String] = []

    private static let tileData = UIGraphicsImageRenderer(size: CGSize(width: 1, height: 1)).pngData { context in
        UIColor.red.setFill()
        context.fill(CGRect(x: 0, y: 0, width: 1, height: 1))
    }

    static func hasRequested(containing id: String) -> Bool {
        lock.lock()
        defer { lock.unlock() }
        return requestedURLs.contains { $0.contains(id) }
    }

    override class func canInit(with request: URLRequest) -> Bool {
        request.url?.host == host
    }

    override class func canonicalRequest(for request: URLRequest) -> URLRequest {
        request
    }

    override func startLoading() {
        guard let url = request.url,
              let response = HTTPURLResponse(
                  url: url,
                  statusCode: 200,
                  httpVersion: "HTTP/1.1",
                  headerFields: ["Content-Type": "image/png", "Cache-Control": "max-age=86400"]
              )
        else {
            client?.urlProtocol(self, didFailWithError: URLError(.badURL))
            return
        }

        Self.lock.lock()
        Self.requestedURLs.append(url.absoluteString)
        Self.lock.unlock()

        client?.urlProtocol(self, didReceive: response, cacheStoragePolicy: .notAllowed)
        client?.urlProtocol(self, didLoad: Self.tileData)
        client?.urlProtocolDidFinishLoading(self)
    }

    override func stopLoading() {}
}
