//
//  MapView.swift
//
//  Copyright © 2021 MapLibre Contributors. All rights reserved.
//

import SwiftUI
import Mapbox
import OSLog
import MapViewOSLogExtensions

struct MapView: UIViewRepresentable {
    let style: String
    
    func makeCoordinator() -> MapView.Coordinator {
        Coordinator(self)
    }
    
    func makeUIView(context: UIViewRepresentableContext<MapView>) -> MGLMapView {
        OSLog.mapView(.event)
        var frame = CGRect(x: 0, y: 0, width: 10, height: 20)
//        frame = .zero  // TODO: investigate if this case works
        
        let mapView = MGLMapView(frame: frame)

        mapView.styleURL = URL(string: style)
        mapView.logoView.isHidden = true
        mapView.delegate = context.coordinator
        
        OSLog.mapView(.event, "frame: \(mapView.frame)")
        OSLog.mapView(.event, "style: \(mapView.styleURL)")
        return mapView
    }
    
    func updateUIView(_ uiView: MGLMapView, context: UIViewRepresentableContext<MapView>) {
        OSLog.mapView(.event)
    }
    
    // MARK: - Implementing MGLMapViewDelegate
    
    final class Coordinator: NSObject, MGLMapViewDelegate {
        var control: MapView
        
        init(_ control: MapView) {
            OSLog.mapView(.event, "🎬 Coordinator init")
            self.control = control
        }

        /// Log events:  WillStartLoadingMap
        func mapViewWillStartLoadingMap(_ mapView: MGLMapView) {
            OSLog.mapView(.event, OSLog.mapEvents.WillStartLoadingMap.rawValue)
        }

        /// Log events:  WillStartRenderingMap
        func mapViewWillStartRenderingMap(_ mapView: MGLMapView) {
            OSLog.mapView(.event, OSLog.mapEvents.WillStartRenderingMap.rawValue)
        }

        /// Log events:  DidFinishLoadingStyle
        func mapView(_ mapView: MGLMapView, didFinishLoading style: MGLStyle) {
            OSLog.mapView(.event, OSLog.mapEvents.DidFinishLoadingStyle.rawValue)
        }
        
        /// Log events:  DidFinishRenderingMap
        func mapViewDidFinishRenderingMap(_ mapView: MGLMapView, fullyRendered: Bool) {
            OSLog.mapView(.event, OSLog.mapEvents.DidFinishRenderingMap.rawValue)
        }

        /// Log events:  DidFinishLoadingMap
        func mapViewDidFinishLoadingMap(_ mapView: MGLMapView) {
            OSLog.mapView(.event, OSLog.mapEvents.DidFinishLoadingMap.rawValue)
        }
        
        /// Log events:  DidBecomeIdle
        func mapViewDidBecomeIdle(_ mapView: MGLMapView) {
            OSLog.mapView(.event, OSLog.mapEvents.DidBecomeIdle.rawValue)
        }
        
        func mapView(_ mapView: MGLMapView, viewFor annotation: MGLAnnotation) -> MGLAnnotationView? {
            return nil
        }
         
        func mapView(_ mapView: MGLMapView, annotationCanShowCallout annotation: MGLAnnotation) -> Bool {
            return true
        }
    }
}
