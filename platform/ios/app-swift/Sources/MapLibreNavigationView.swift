import MapLibre
import SwiftUI

struct MapLibreNavigationView: View {
    var body: some View {
        NavigationStack {
            List {
                NavigationLink("SimpleMap") {
                    SimpleMap().edgesIgnoringSafeArea(.all)
                }
                NavigationLink("LineTapMap") {
                    LineTapMap().edgesIgnoringSafeArea(.all)
                }
                NavigationLink("LocationPrivacyExample") {
                    LocationPrivacyExampleView()
                }
                NavigationLink("BlockingGesturesExample") {
                    BlockingGesturesExample()
                }
                NavigationLink("MaximumBoundsExample") {
                    MaximumBoundsExample()
                }  
                NavigationLink("LineStyleLayerExample") {
                    LineStyleLayerExampleUIViewControllerRepresentable()
                }
            }
        }
    }
}
