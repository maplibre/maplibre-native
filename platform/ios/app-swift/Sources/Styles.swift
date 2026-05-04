import Foundation

// #-example-code(ExampleStyles)

let DEMOTILES_STYLE = URL(string: "https://demotiles.maplibre.org/style.json")
let AMERICANA_STYLE = URL(string: "https://americanamap.org/style.json")
let OPENFREEMAP_LIBERTY_STYLE = URL(string: "https://tiles.openfreemap.org/styles/liberty")
let OPENFREEMAP_BRIGHT_STYLE = URL(string: "https://tiles.openfreemap.org/styles/bright")
let VERSATILES_COLORFUL_STYLE = URL(string: "https://tiles.versatiles.org/assets/styles/colorful.json")

// #-end-example-code

let AWS_OPEN_DATA_STANDARD_LIGHT_STYLE = URL(string: "https://maps.geo.us-east-2.amazonaws.com/maps/v0/maps/OpenDataStyle/style-descriptor?key=v1.public.eyJqdGkiOiI1NjY5ZTU4My0yNWQwLTQ5MjctODhkMS03OGUxOTY4Y2RhMzgifR_7GLT66TNRXhZJ4KyJ-GK1TPYD9DaWuc5o6YyVmlikVwMaLvEs_iqkCIydspe_vjmgUVsIQstkGoInXV_nd5CcmqRMMa-_wb66SxDdbeRDvmmkpy2Ow_LX9GJDgL2bbiCws0wupJPFDwWCWFLwpK9ICmzGvNcrPbX5uczOQL0N8V9iUvziA52a1WWkZucIf6MUViFRf3XoFkyAT15Ll0NDypAzY63Bnj8_zS8bOaCvJaQqcXM9lrbTusy8Ftq8cEbbK5aMFapXRjug7qcrzUiQ5sr0g23qdMvnKJQFfo7JuQn8vwAksxrQm6A0ByceEXSfyaBoVpFcTzEclxUomhY.NjAyMWJkZWUtMGMyOS00NmRkLThjZTMtODEyOTkzZTUyMTBi")

private func protomaps(_ style: String) -> URL? {
    URL(string: "https://api.protomaps.com/styles/v2/\(style).json?key=e761cc7daedf832a")
}

let PROTOMAPS_LIGHT_STYLE = protomaps("light")
let PROTOMAPS_DARK_STYLE = protomaps("dark")
let PROTOMAPS_GRAYSCALE_STYLE = protomaps("grayscale")
let PROTOMAPS_WHITE_STYLE = protomaps("white")
let PROTOMAPS_BLACK_STYLE = protomaps("black")
