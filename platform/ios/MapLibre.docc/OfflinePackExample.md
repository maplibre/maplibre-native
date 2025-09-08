# Download Offline Pack

Download a region as an offline pack

> Note: This example uses UIKit.

This example shows how to download a region as an offline pack. This particular example kicks off the download as soon as the map finished loading. For more control when and how to manage offline regions, see <doc:ManageOfflineRegionsExample>.

First, you need to define a ``MLNTilePyramidOfflineRegion``. Note that you should be conservative with your zoom level, because each individual tile in the tile pyramid will be individually downloaded. If you want to make a large area available offline, you should prepare a bundle, download this manually and use ``MLNOfflineStorage/addContentsOfURL:withCompletionHandler:``. However, this is outside of the scope of this example.

You can pass along some user data / context with ``MLNOfflineStorage/addPackForRegion:withContext:completionHandler:``. This can be read in the notififications that are emitted. You should listen for these notifications using the names defined by

- ``MLNOfflinePackProgressChangedNotification``
- ``MLNOfflinePackErrorNotification``, and
- ``MLNOfflinePackMaximumMapboxTilesReachedNotification``

as is shown in the example. Note that this last notification is a historical artifact, your tile provider may not have a maximum number of tiles you are allowed to store.


<!-- include-example(OfflinePackExample) -->

```swift
class OfflinePackExample: UIViewController, MLNMapViewDelegate {
    var mapView: MLNMapView!
    var progressView: UIProgressView!
    let jsonDecoder = JSONDecoder()

    struct UserData: Codable {
        var name: String
    }

    override func viewDidLoad() {
        super.viewDidLoad()

        mapView = MLNMapView(frame: view.bounds, styleURL: AMERICANA_STYLE)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.tintColor = .gray
        mapView.delegate = self
        view.addSubview(mapView)

        mapView.setCenter(CLLocationCoordinate2D(latitude: 22.27933, longitude: 114.16281),
                          zoomLevel: 13, animated: false)

        // Setup offline pack notification handlers.
        NotificationCenter.default.addObserver(self, selector: #selector(offlinePackProgressDidChange), name: NSNotification.Name.MLNOfflinePackProgressChanged, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(offlinePackDidReceiveError), name: NSNotification.Name.MLNOfflinePackError, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(offlinePackDidReceiveMaximumAllowedMapboxTiles), name: NSNotification.Name.MLNOfflinePackMaximumMapboxTilesReached, object: nil)
    }

    func mapViewDidFinishLoadingMap(_: MLNMapView) {
        // Start downloading tiles and resources for z13-14.
        startOfflinePackDownload()
    }

    override func viewDidDisappear(_ animated: Bool) {
        super.viewDidDisappear(animated)

        // When leaving this view controller, suspend offline downloads.
        guard let packs = MLNOfflineStorage.shared.packs else { return }
        for pack in packs {
            if let userInfo = try? jsonDecoder.decode(UserData.self, from: pack.context) {
                print("Suspending download of offline pack: '\(userInfo.name)'")
            }

            pack.suspend()
        }
    }

    func startOfflinePackDownload() {
        // Create a region that includes the current viewport and any tiles needed to view it when zoomed further in.
        // Because tile count grows exponentially with the maximum zoom level, you should be conservative with your `toZoomLevel` setting.
        let region = MLNTilePyramidOfflineRegion(styleURL: mapView.styleURL, bounds: mapView.visibleCoordinateBounds, fromZoomLevel: mapView.zoomLevel, toZoomLevel: 14)

        // Store some data for identification purposes alongside the downloaded resources.
        let jsonEncoder = JSONEncoder()

        let userInfo = UserData(name: "My Offline Pack")
        let encodedUserInfo = try! jsonEncoder.encode(userInfo)
        print(encodedUserInfo)

        // Create and register an offline pack with the shared offline storage object.

        MLNOfflineStorage.shared.addPack(for: region, withContext: encodedUserInfo) { pack, error in
            guard error == nil else {
                // The pack couldn’t be created for some reason.
                print("Error: \(error?.localizedDescription ?? "unknown error")")
                return
            }

            // Start downloading.
            pack!.resume()
        }
    }

    // MARK: - MLNOfflinePack notification handlers

    @objc func offlinePackProgressDidChange(notification: NSNotification) {
        // Get the offline pack this notification is regarding,
        // and the associated user info for the pack; in this case, `name = My Offline Pack`
        if let pack = notification.object as? MLNOfflinePack,
           let userInfo = try? jsonDecoder.decode(UserData.self, from: pack.context)
        {
            let progress = pack.progress
            // or notification.userInfo![MLNOfflinePackProgressUserInfoKey]!.MLNOfflinePackProgressValue
            let completedResources = progress.countOfResourcesCompleted
            let expectedResources = progress.countOfResourcesExpected

            // Calculate current progress percentage.
            let progressPercentage = Float(completedResources) / Float(expectedResources)

            // Setup the progress bar.
            if progressView == nil {
                progressView = UIProgressView(progressViewStyle: .default)
                let frame = view.bounds.size
                progressView.frame = CGRect(x: frame.width / 4, y: frame.height * 0.75, width: frame.width / 2, height: 10)
                view.addSubview(progressView)
            }

            progressView.progress = progressPercentage

            // If this pack has finished, print its size and resource count.
            if completedResources == expectedResources {
                let byteCount = ByteCountFormatter.string(fromByteCount: Int64(pack.progress.countOfBytesCompleted), countStyle: ByteCountFormatter.CountStyle.memory)
                print("Offline pack “\(userInfo.name)” completed: \(byteCount), \(completedResources) resources")
            } else {
                // Otherwise, print download/verification progress.
                print("Offline pack “\(userInfo.name)” has \(completedResources) of \(expectedResources) resources — \(String(format: "%.2f", progressPercentage * 100))%.")
            }
        }
    }

    @objc func offlinePackDidReceiveError(notification: NSNotification) {
        if let pack = notification.object as? MLNOfflinePack,
           let userInfo = try? jsonDecoder.decode(UserData.self, from: pack.context),
           let error = notification.userInfo?[MLNOfflinePackUserInfoKey.error] as? NSError
        {
            print("Offline pack “\(userInfo.name)” received error: \(error.localizedFailureReason ?? "unknown error")")
        }
    }

    @objc func offlinePackDidReceiveMaximumAllowedMapboxTiles(notification: NSNotification) {
        if let pack = notification.object as? MLNOfflinePack,
           let userInfo = try? jsonDecoder.decode(UserData.self, from: pack.context),
           let maximumCount = (notification.userInfo?[MLNOfflinePackUserInfoKey.maximumCount] as AnyObject).uint64Value
        {
            print("Offline pack “\(userInfo.name)” reached limit of \(maximumCount) tiles.")
        }
    }
}
```
