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
        Text(text)
            .font(.system(size: 16, weight: .bold, design: .monospaced))
            .foregroundColor(.white)
            .padding(12)
            .background(MapLibreColors.primary)
            .cornerRadius(12)
    }
}
