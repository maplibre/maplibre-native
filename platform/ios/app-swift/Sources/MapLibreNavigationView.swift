import MapLibre
import SwiftUI

struct MapLibreNavigationView: View {
    var body: some View {
        NavigationStack {
            List {
                Section {
                    NavigationLink("Start Long Running Test") {
                        LongRunningMapView()
                    }
                    .listRowBackground(MapLibreColors.primary)
                    .foregroundColor(.white)
                    .fontWeight(.bold)
                }

                NavigationLink("SimpleMap") {
                    SimpleMap().edgesIgnoringSafeArea(.all)
                }
                NavigationLink("Change Camera Pitch & Roll") {
                    CameraSliderExample()
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
