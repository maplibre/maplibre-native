import UIKit

/// Scrolling bar graph of per-frame encoding time (based on `MBXFrameTimeGraphView`).
final class FrameTimeGraphView: UIView {
    private let exaggeration: CGFloat = 10
    private let barWidth: CGFloat = 4

    private let scrollLayer = CAScrollLayer()
    private let thresholdLayer = CAShapeLayer()
    private var currentX: CGFloat = 0
    private var barLayers: [CAShapeLayer] = []

    private let safeColor = UIColor(red: 0 / 255, green: 190 / 255, blue: 123 / 255, alpha: 1)
    private let warningColor = UIColor(red: 255 / 255, green: 154 / 255, blue: 82 / 255, alpha: 1)
    private let dangerColor = UIColor(red: 255 / 255, green: 91 / 255, blue: 86 / 255, alpha: 1)

    override init(frame: CGRect) {
        super.init(frame: frame)
        commonInit()
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        commonInit()
    }

    private func commonInit() {
        isUserInteractionEnabled = false
        layer.opacity = 0.9

        scrollLayer.scrollMode = .horizontally
        scrollLayer.masksToBounds = true
        layer.addSublayer(scrollLayer)

        thresholdLayer.fillColor = UIColor.darkGray.cgColor
        layer.addSublayer(thresholdLayer)
    }

    override func layoutSubviews() {
        super.layoutSubviews()

        guard scrollLayer.frame != bounds else { return }
        scrollLayer.frame = bounds

        let targetSeconds = renderDurationTargetSeconds
        let thresholdY = bounds.height - bounds.height * exaggeration * targetSeconds
        let thresholdRect = CGRect(x: 0, y: thresholdY, width: bounds.width, height: 1)
        thresholdLayer.path = CGPath(rect: thresholdRect, transform: nil)
    }

    func addFrameDuration(_ frameDuration: TimeInterval) {
        CATransaction.begin()
        CATransaction.setDisableActions(true)

        currentX += barWidth

        let bar = barLayer(for: frameDuration)
        bar.position = CGPoint(x: currentX, y: bounds.height)
        scrollLayer.addSublayer(bar)
        barLayers.append(bar)

        scrollLayer.scroll(to: CGPoint(x: currentX - bounds.width, y: 0))
        removeStaleBars()

        CATransaction.commit()
    }

    private var renderDurationTargetSeconds: CGFloat {
        let maxFPS = UIScreen.main.maximumFramesPerSecond
        return CGFloat(1.0 / Double(maxFPS))
    }

    private func barLayer(for frameDuration: TimeInterval) -> CAShapeLayer {
        let bar = CAShapeLayer()
        let height = min(
            CGFloat(frameDuration) * exaggeration * bounds.height,
            bounds.height
        )
        let barRect = CGRect(x: 0, y: 0, width: barWidth, height: -height)
        bar.path = CGPath(rect: barRect, transform: nil)
        bar.fillColor = color(for: frameDuration).cgColor
        return bar
    }

    private func color(for frameDuration: TimeInterval) -> UIColor {
        let greenLevel = renderDurationTargetSeconds
        let yellowLevel = greenLevel * 3

        if frameDuration <= greenLevel {
            return safeColor
        }
        if frameDuration <= yellowLevel {
            return warningColor
        }
        return dangerColor
    }

    private func removeStaleBars() {
        let maxBars = Int(bounds.width / barWidth * 3)
        guard barLayers.count > maxBars else { return }

        let removeCount = Int(bounds.width / barWidth)
        let stale = barLayers.prefix(removeCount)
        stale.forEach { $0.removeFromSuperlayer() }
        barLayers.removeFirst(removeCount)
    }
}
