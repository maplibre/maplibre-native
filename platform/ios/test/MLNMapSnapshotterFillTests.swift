import MapLibre
import XCTest

/// A translucent fill is clipped to its tiles with the stencil buffer. Without a stencil target
/// in the snapshotter's render pass, overlapping tiles each draw it, and its alpha accumulates.
class MLNMapSnapshotterFillTests: XCTestCase {
    /// How many pixels have each alpha value in a snapshot of one `rgba(255,0,0,0.5)` polygon
    /// larger than the view, at the given center and span in degrees of latitude.
    private func alphaHistogram(longitude: Double, latitude: Double, span: Double) throws -> [UInt8: Int] {
        let k = cos(latitude * .pi / 180)
        let (west, south) = (longitude - span / 2 / k, latitude - span / 2)
        let (east, north) = (longitude + span / 2 / k, latitude + span / 2)
        let g = span * 0.3
        let ring = [[west - g, south - g], [east + g, south - g], [east + g, north + g], [west - g, north + g], [west - g, south - g]]
        let style: [String: Any] = [
            "version": 8,
            "sources": ["p": ["type": "geojson", "data": ["type": "Polygon", "coordinates": [ring]]]],
            "layers": [["id": "fill", "type": "fill", "source": "p", "paint": ["fill-color": "rgba(255,0,0,0.5)"]]],
        ]
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("style-\(UUID()).json")
        try JSONSerialization.data(withJSONObject: style).write(to: url)
        defer { try? FileManager.default.removeItem(at: url) }

        let options = MLNMapSnapshotOptions(styleURL: url, camera: MLNMapCamera(), size: CGSize(width: 180, height: 180))
        options.coordinateBounds = MLNCoordinateBounds(
            sw: CLLocationCoordinate2D(latitude: south, longitude: west),
            ne: CLLocationCoordinate2D(latitude: north, longitude: east)
        )
        options.showsLogo = false

        let done = expectation(description: "snapshot")
        var image: UIImage?
        let snapshotter = MLNMapSnapshotter(options: options)
        snapshotter.start { snapshot, error in
            XCTAssertNil(error)
            image = snapshot?.image
            done.fulfill()
        }
        wait(for: [done], timeout: 30)

        // Redrawn into a known premultiplied RGBA8 buffer, so that alpha reads directly.
        let cgImage = try XCTUnwrap(image?.cgImage)
        let (width, height) = (cgImage.width, cgImage.height)
        var pixels = [UInt8](repeating: 0, count: width * height * 4)
        let context = try XCTUnwrap(CGContext(
            data: &pixels, width: width, height: height, bitsPerComponent: 8, bytesPerRow: width * 4,
            space: CGColorSpaceCreateDeviceRGB(), bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue
        ))
        context.draw(cgImage, in: CGRect(x: 0, y: 0, width: width, height: height))
        var counts: [UInt8: Int] = [:]
        for i in stride(from: 3, to: pixels.count, by: 4) {
            counts[pixels[i], default: 0] += 1
        }
        return counts
    }

    func testTranslucentFillIsDrawnOnce() throws {
        // Spans that overlap more than one tile, at the zooms where the issue was seen.
        let cases: [(Double, Double, Double)] = [
            (2.3522, 48.8566, 0.002),
            (2.3522, 48.8566, 0.25),
            (2.3001, 48.8901, 0.008),
            (2.3001, 48.8901, 0.03),
        ]
        for (longitude, latitude, span) in cases {
            let histogram = try alphaHistogram(longitude: longitude, latitude: latitude, span: span)
            let wrong = histogram.filter { abs(Int($0.key) - 128) > 2 }.values.reduce(0, +)
            XCTAssertEqual(wrong, 0, "span \(span) at (\(longitude), \(latitude)): alpha histogram \(histogram)")
        }
    }
}
