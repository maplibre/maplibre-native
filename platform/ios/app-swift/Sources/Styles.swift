import Foundation

// #-example-code(ExampleStyles)

let DEMOTILES_STYLE = URL(string: "https://demotiles.maplibre.org/style.json")
let AMERICANA_STYLE = URL(string: "https://americanamap.org/style.json")
let OPENFREEMAP_LIBERTY_STYLE = URL(string: "https://tiles.openfreemap.org/styles/liberty")
let OPENFREEMAP_BRIGHT_STYLE = URL(string: "https://tiles.openfreemap.org/styles/bright")
let VERSATILES_COLORFUL_STYLE = URL(string: "https://tiles.versatiles.org/assets/styles/colorful.json")

// #-end-example-code

private func protomaps(_ style: String) -> URL? {
    URL(string: "https://api.protomaps.com/styles/v2/\(style).json?key=e761cc7daedf832a")
}

let PROTOMAPS_LIGHT_STYLE = protomaps("light")
let PROTOMAPS_DARK_STYLE = protomaps("dark")
let PROTOMAPS_GRAYSCALE_STYLE = protomaps("grayscale")
let PROTOMAPS_WHITE_STYLE = protomaps("white")
let PROTOMAPS_BLACK_STYLE = protomaps("black")
