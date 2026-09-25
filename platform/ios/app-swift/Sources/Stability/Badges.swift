import SwiftUI

struct CountdownBadge: View {
    let text: String

    init(remainingTime: TimeInterval) {
        let hours = Int(remainingTime) / 3600
        let minutes = (Int(remainingTime) % 3600) / 60
        let seconds = Int(remainingTime) % 60
        text = String(format: "%02dh %02dm %02ds", hours, minutes, seconds)
    }

    init(remainingCycles: Int) {
        text = String(format: "%d cycles", remainingCycles)
    }

    var body: some View {
        StatusBadge {
            Text(text)
        }
    }
}

struct MapStatsBadge: View {
    let framesPerSecond: Double
    let zoomLevel: Double
    let routePointCount: Int

    var body: some View {
        StatusBadge {
            VStack(alignment: .leading, spacing: 4) {
                Text(String(format: "%3.0f FPS", framesPerSecond))
                Text(String(format: "z %5.2f", zoomLevel))
                Text(String(format: "%5d points", routePointCount))
                // Indicate debug builds, frame rates are sometimes much lower
                #if DEBUG
                    Text("DEBUG")
                #endif
            }
        }
    }
}

private struct StatusBadge<Content: View>: View {
    @ViewBuilder var content: () -> Content

    var body: some View {
        content()
            .font(.system(size: 16, weight: .bold, design: .monospaced))
            .foregroundColor(.white)
            .padding(12)
            .background(MapLibreColors.primary)
            .cornerRadius(12)
    }
}
