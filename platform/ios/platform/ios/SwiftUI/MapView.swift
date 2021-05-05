//
//  MapView.swift
//
//  Copyright © 2021 MapLibre Contributors. All rights reserved.
//

import SwiftUI
import Mapbox

struct MapView: UIViewRepresentable {
    let style: String
    
    func makeUIView(context: UIViewRepresentableContext<MapView>) -> MGLMapView {
        var frame = CGRect(x: 0, y: 0, width: 10, height: 20)
//        frame = .zero  // TODO: investigate if this case works
        
        let mapView = MGLMapView(frame: frame)

        mapView.styleURL = URL(string: style)
        mapView.logoView.isHidden = true
        
        return mapView
    }
    
    func updateUIView(_ uiView: MGLMapView, context: UIViewRepresentableContext<MapView>) {
    }
}
