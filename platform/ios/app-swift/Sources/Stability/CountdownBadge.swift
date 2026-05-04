import SwiftUI

struct CountdownBadge: View {
    let remainingTime: TimeInterval

    var body: some View {
        Text(formatTime(remainingTime))
            .font(.system(size: 16, weight: .bold, design: .monospaced))
            .foregroundColor(.white)
            .padding(12)
            .background(MapLibreColors.primary)
            .cornerRadius(12)
    }

    private func formatTime(_ timeInterval: TimeInterval) -> String {
        let hours = Int(timeInterval) / 3600
        let minutes = (Int(timeInterval) % 3600) / 60
        let seconds = Int(timeInterval) % 60
        return String(format: "%02dh %02dm %02ds", hours, minutes, seconds)
    }
}
