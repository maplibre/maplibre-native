import MapLibre
import Sentry
import SwiftUI
import UIKit

@main
struct MapLibreApp: App {
    init() {
        if let dsn = Bundle.main.object(forInfoDictionaryKey: "SENTRY_DSN") as? String {
            SentrySDK.start { options in
                options.dsn = dsn
                options.enableCrashHandler = true
                options.enabled = true
            }

            print("Sentry initialized")
        }
    }

    var body: some Scene {
        WindowGroup {
            MapLibreNavigationView()
        }
    }
}
