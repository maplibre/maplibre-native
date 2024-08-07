# Manage Offline Regions

Query, delete and download offline regions

> Note: This example uses UIKit.

This example is similar to <doc:OfflinePackExample>, but shows how offline regions can be managed.

- ``MLNOfflineStorage/addPackForRegion:withContext:completionHandler:`` is used to kick off downloads for offline regions, as before.
- ``MLNOfflineStorage/packs`` returns an array of packs that have been downloaded. In this example they are shown in an `UITableView`.
- ``MLNOfflineStorage/resetDatabaseWithCompletionHandler:`` can be used to reset the (offline) database. Note that this includes the ambient cache at the time of writing. In this example, this method is used on view initialization.

When selecting one of the packs in the table view, the map moves to the bounds of the corresponding region.

<!-- include-example(ManageOfflineRegionsExample) -->

```swift
class ManageOfflineRegionsExample: UIViewController, MLNMapViewDelegate {
    let jsonDecoder = JSONDecoder()

    struct UserData: Codable {
        var name: String
    }

    lazy var mapView: MLNMapView = {
        let mapView = MLNMapView(frame: CGRect.zero, styleURL: AMERICANA_STYLE)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.tintColor = .gray
        mapView.delegate = self
        mapView.translatesAutoresizingMaskIntoConstraints = false
        return mapView
    }()

    lazy var downloadButton: UIButton = {
        let downloadButton = UIButton(frame: CGRect.zero)
        downloadButton.backgroundColor = UIColor.systemBlue
        downloadButton.setTitleColor(UIColor.white, for: .normal)
        downloadButton.setTitle("Download Region", for: .normal)
        downloadButton.addTarget(self, action: #selector(startOfflinePackDownload), for: .touchUpInside)
        downloadButton.layer.cornerRadius = view.bounds.width / 30
        downloadButton.translatesAutoresizingMaskIntoConstraints = false
        return downloadButton
    }()

    lazy var tableView: UITableView = {
        let tableView = UITableView(frame: CGRect.zero)
        tableView.delegate = self
        tableView.dataSource = self
        tableView.translatesAutoresizingMaskIntoConstraints = false
        return tableView
    }()

    override func viewDidLoad() {
        super.viewDidLoad()
        view.addSubview(mapView)
        view.addSubview(tableView)
        mapView.addSubview(downloadButton)
        let centerCoordinate = CLLocationCoordinate2D(latitude: 22.27933, longitude: 114.16281)
        mapView.setCenter(centerCoordinate, zoomLevel: 13, animated: false)

        // Set up constraints for map view, table view, and download button.
        installConstraints()
    }

    func setupOfflinePackHandler() {
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(offlinePackProgressDidChange),
                                               name: NSNotification.Name.MLNOfflinePackProgressChanged,
                                               object: nil)
    }

    func installConstraints() {
        NSLayoutConstraint.activate([
            mapView.topAnchor.constraint(equalTo: view.topAnchor),
            mapView.leadingAnchor.constraint(equalTo: view.leadingAnchor),
            mapView.trailingAnchor.constraint(equalTo: view.trailingAnchor),
            mapView.heightAnchor.constraint(equalTo: view.heightAnchor, multiplier: 0.5),
            tableView.topAnchor.constraint(equalTo: mapView.bottomAnchor),
            tableView.leadingAnchor.constraint(equalTo: view.leadingAnchor),
            tableView.trailingAnchor.constraint(equalTo: view.trailingAnchor),
            tableView.bottomAnchor.constraint(equalTo: view.bottomAnchor),
            downloadButton.topAnchor.constraint(equalTo: view.topAnchor, constant: 100),
            downloadButton.leadingAnchor.constraint(equalTo: view.leadingAnchor, constant: 5),
            downloadButton.widthAnchor.constraint(equalTo: view.widthAnchor, multiplier: 0.45),
            downloadButton.heightAnchor.constraint(equalTo: downloadButton.widthAnchor, multiplier: 0.2),
        ])
    }

    /*
       For the purposes of this example, remove any offline packs
       that exist before the example is re-loaded.
     */
    override func viewWillAppear(_: Bool) {
        MLNOfflineStorage.shared.resetDatabase { error in
            if let error {
                // Handle the error here if packs can't be removed.
                print(error)
            } else {
                MLNOfflineStorage.shared.reloadPacks()
            }
        }
    }

    @objc func startOfflinePackDownload(selector _: NSNotification) {
        // Setup offline pack notification handlers.
        setupOfflinePackHandler()

        /**
         Create a region that includes the current map camera, to be captured
         in an offline map. Note: Because tile count grows exponentially as zoom level
         increases, you should be conservative with your `toZoomLevel` setting.
         */
        let region = MLNTilePyramidOfflineRegion(styleURL: mapView.styleURL,
                                                 bounds: mapView.visibleCoordinateBounds,
                                                 fromZoomLevel: mapView.zoomLevel,
                                                 toZoomLevel: mapView.zoomLevel + 2)
        // Store some data for identification purposes alongside the offline pack.
        let userInfo = UserData(name: "\(region.bounds)")
        let jsonEncoder = JSONEncoder()
        let optionalContext = try? jsonEncoder.encode(userInfo)
        guard let context = optionalContext else {
            print("Error: failed to get context")
            return
        }

        // Create and register an offline pack with the shared offline storage object.
        MLNOfflineStorage.shared.addPack(for: region, withContext: context) { pack, error in
            guard error == nil else {
                // Handle the error if the offline pack couldn’t be created.
                print("Error: \(error?.localizedDescription ?? "unknown error")")
                return
            }

            // Begin downloading the map for offline use.
            pack!.resume()
        }
    }

    // MARK: - MLNOfflinePack notification handlers

    @objc func offlinePackProgressDidChange(notification: NSNotification) {
        /**
         Get the offline pack this notification is referring to,
         along with its associated metadata.
         */
        if let pack = notification.object as? MLNOfflinePack,
           let userInfo = try? jsonDecoder.decode(UserData.self, from: pack.context)
        {
            // At this point, the offline pack has finished downloading.

            if pack.state == .complete {
                let byteCount = ByteCountFormatter.string(fromByteCount: Int64(pack.progress.countOfBytesCompleted), countStyle: ByteCountFormatter.CountStyle.memory)

                let packName = userInfo.name

                print("""
                  Offline pack “\(packName)” completed download:
                    - Bytes: \(byteCount)
                    - Resource count: \(pack.progress.countOfResourcesCompleted)")
                """)

                NotificationCenter.default.removeObserver(self, name: NSNotification.Name.MLNOfflinePackProgressChanged,
                                                          object: nil)
            }
        }
        // Reload the table to update the progress percentage for each offline pack.
        tableView.reloadData()
    }
}

private extension MLNOfflinePackProgress {
    var percentCompleted: Float {
        guard countOfResourcesExpected != 0 else {
            return 0
        }

        let percentage = Float((countOfResourcesCompleted / countOfResourcesExpected) * 100)
        return percentage
    }

    var formattedCountOfBytesCompleted: String {
        ByteCountFormatter.string(fromByteCount: Int64(countOfBytesCompleted),
                                  countStyle: .memory)
    }
}

extension ManageOfflineRegionsExample: UITableViewDelegate, UITableViewDataSource {
    // Create the table view which will display the downloaded regions.
    func tableView(_: UITableView, numberOfRowsInSection _: Int) -> Int {
        if let packs = MLNOfflineStorage.shared.packs {
            return packs.count
        } else {
            return 0
        }
    }

    func tableView(_: UITableView, viewForHeaderInSection _: Int) -> UIView? {
        let label = UILabel()

        label.backgroundColor = UIColor.systemBlue
        label.textColor = UIColor.white
        label.font = UIFont.preferredFont(forTextStyle: .headline)
        label.textAlignment = .center

        if MLNOfflineStorage.shared.packs != nil {
            label.text = "Offline maps"
        } else {
            label.text = "No offline maps"
        }

        return label
    }

    func tableView(_: UITableView, heightForHeaderInSection _: Int) -> CGFloat {
        50.0
    }

    func tableView(_: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = UITableViewCell(style: .subtitle, reuseIdentifier: "cell")

        if let packs = MLNOfflineStorage.shared.packs {
            let pack = packs[indexPath.row]

            cell.textLabel?.text = "Region \(indexPath.row + 1): size: \(pack.progress.formattedCountOfBytesCompleted)"
            cell.detailTextLabel?.text = "Percent completion: \(pack.progress.percentCompleted)%"
        }

        return cell
    }

    func tableView(_: UITableView, didSelectRowAt indexPath: IndexPath) {
        guard let packs = MLNOfflineStorage.shared.packs else { return }

        if let selectedRegion = packs[indexPath.row].region as? MLNTilePyramidOfflineRegion {
            mapView.setVisibleCoordinateBounds(selectedRegion.bounds, animated: true)
        }
    }
}
```