import MapLibre
import SwiftUI

struct MapLibreNavigationView: View {
    var body: some View {
        NavigationStack {
            List {
                NavigationLink("SimpleMap") {
                    SimpleMap().edgesIgnoringSafeArea(.all)
                }
                #if MLN_RENDER_BACKEND_METAL
                    NavigationLink("CustomStyleLayer (Metal)") {
                        CustomStyleLayerExample().edgesIgnoringSafeArea(.all)
                    }
                #endif
                NavigationLink("LineTapMap") {
                    LineTapMap().edgesIgnoringSafeArea(.all)
                }
                NavigationLink("LocationPrivacyExample") {
                    LocationPrivacyExampleView()
                }
                NavigationLink("BlockingGesturesExample") {
                    BlockingGesturesExample()
                }
                NavigationLink("MaximumScreenBoundsExample") {
                    MaximumScreenBoundsExample()
                }
                NavigationLink("LineStyleLayerExample") {
                    LineStyleLayerExampleUIViewControllerRepresentable()
                }
                NavigationLink("WebAPIDataExample") {
                    WebAPIDataExampleUIViewControllerRepresentable()
                }
                NavigationLink("AddMarkerExample") {
                    AddMarkerSymbolExampleUIViewControllerRepresentable()
                }
                NavigationLink("ClusteringExample") {
                    ClusteringExampleUIViewControllerRepresentable()
                }
                NavigationLink("ObserverExample") {
                    ObserverExampleViewUIViewControllerRepresentable()
                }
                NavigationLink("LongRunningMap") {
                    LongRunningMapView().edgesIgnoringSafeArea(.all)
                }
                Group {
                    NavigationLink("AnimatedLineExample") {
                        AnimatedLineExampleUIViewControllerRepresentable()
                    }
                    NavigationLink("AnnotationViewExample") {
                        AnnotationViewExampleUIViewControllerRepresentable()
                    }
                    NavigationLink("BuildingLightExample") {
                        BuildingLightExampleUIViewControllerRepresentable()
                    }
                    NavigationLink("StaticSnapshotExample") {
                        StaticSnapshotExampleUIViewControllerRepresentable()
                    }
                    NavigationLink("DDSCircleLayerExample") {
                        DDSCircleLayerExampleUIViewControllerRepresentable().edgesIgnoringSafeArea(.all)
                    }
                    NavigationLink("POIAlongRouteExample") {
                        POIAlongRouteExampleUIViewControllerRepresentable()
                    }
                    NavigationLink("ManageOfflineRegionsExample") {
                        ManageOfflineRegionsExampleUIViewControllerRepresentable()
                    }
                    NavigationLink("OfflinePackExampleUIViewControllerRepresentable") {
                        OfflinePackExampleUIViewControllerRepresentable()
                    }
                    NavigationLink("MultipleImagesExample") {
                        MultipleImagesExampleUIViewControllerRepresentable().edgesIgnoringSafeArea(.all)
                    }
                }
            }
        }
    }
}
