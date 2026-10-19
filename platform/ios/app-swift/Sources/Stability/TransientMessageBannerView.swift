import UIKit

/// Short-lived status text shown over a host view (toast-style, non-modal).
final class TransientMessageBannerView: UIView {
    private let label = UILabel()
    private var visibilityGeneration = 0

    var message: String {
        get { label.text ?? "" }
        set {
            label.text = newValue
            setNeedsLayout()
        }
    }

    override init(frame: CGRect) {
        super.init(frame: frame)
        isUserInteractionEnabled = false
        backgroundColor = UIColor(red: 0x28 / 255.0, green: 0x5D / 255.0, blue: 0xAA / 255.0, alpha: 0.92)
        layer.cornerRadius = 12
        layer.masksToBounds = true

        label.font = .systemFont(ofSize: 15, weight: .semibold)
        label.textColor = .white
        label.numberOfLines = 0
        label.textAlignment = .center
        addSubview(label)
    }

    @available(*, unavailable)
    required init?(coder _: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    override func sizeThatFits(_ size: CGSize) -> CGSize {
        let horizontalPadding: CGFloat = 16
        let verticalPadding: CGFloat = 12
        let maxLabelWidth = min(size.width - horizontalPadding * 2, 320)
        let labelSize = label.sizeThatFits(CGSize(width: maxLabelWidth, height: .greatestFiniteMagnitude))
        return CGSize(
            width: labelSize.width + horizontalPadding * 2,
            height: labelSize.height + verticalPadding * 2
        )
    }

    override func layoutSubviews() {
        super.layoutSubviews()
        let horizontalPadding: CGFloat = 16
        let verticalPadding: CGFloat = 12
        label.frame = bounds.insetBy(dx: horizontalPadding, dy: verticalPadding)
    }

    /// `generation` identifies this show or hide. A completion from an older
    /// animation does not hide a banner that has since been shown again.
    func setVisible(_ visible: Bool, animated: Bool, generation: Int) {
        visibilityGeneration = generation
        layer.removeAllAnimations()

        if visible {
            let fullyVisible = !isHidden && alpha >= 0.99
            isHidden = false
            if fullyVisible || !animated {
                alpha = 1
                return
            }
            if alpha <= 0.01 {
                alpha = 0
            }
            UIView.animate(withDuration: 0.2) {
                self.alpha = 1
            }
        } else if !animated {
            alpha = 0
            isHidden = true
        } else {
            let generation = visibilityGeneration
            UIView.animate(withDuration: 0.2, animations: {
                self.alpha = 0
            }, completion: { [weak self] _ in
                guard let self, visibilityGeneration == generation else { return }
                isHidden = true
            })
        }
    }
}
