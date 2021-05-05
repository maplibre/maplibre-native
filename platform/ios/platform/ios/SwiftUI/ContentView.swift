//
//  ContentView.swift
//
//  Copyright © 2021 MapLibre Contributors. All rights reserved.
//

import SwiftUI

struct ContentView: View {
    var body: some View {
        MapView(style: "https://raw.githubusercontent.com/roblabs/openmaptiles-ios-demo/master/OSM2VectorTiles/styles/geography-class.GitHub.json")
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
