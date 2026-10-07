import ObjectiveC
import UIKit

private final class TransientMessageHostState {
    var banner: TransientMessageBannerView?
    var autoHideWorkItem: DispatchWorkItem?
    var visibilityGeneration = 0
}

private enum TransientMessageAssociatedKeys {
    static var hostState: UInt8 = 0
}

extension UIView {
    private var transientMessageHostState: TransientMessageHostState {
        if let state = objc_getAssociatedObject(self, &TransientMessageAssociatedKeys.hostState) as? TransientMessageHostState {
            return state
        }
        let state = TransientMessageHostState()
        objc_setAssociatedObject(
            self,
            &TransientMessageAssociatedKeys.hostState,
            state,
            .OBJC_ASSOCIATION_RETAIN_NONATOMIC
        )
        return state
    }

    /// - Parameter autoHideAfter: When set, hides the banner after this many seconds. Pass `nil` to keep it visible until `hideTransientMessage()`.
    func showTransientMessage(
        _ message: String,
        animated: Bool = true,
        autoHideAfter: TimeInterval? = nil
    ) {
        cancelTransientMessageAutoHide()
        let state = transientMessageHostState
        let banner = transientMessageBanner(in: state)
        banner.message = message
        layoutTransientMessage()
        bringSubviewToFront(banner)
        banner.setVisible(true, animated: animated, generation: nextVisibilityGeneration(in: state))

        guard let autoHideAfter, autoHideAfter > 0 else { return }

        let work = DispatchWorkItem { [weak self] in
            self?.hideTransientMessage(animated: animated)
        }
        state.autoHideWorkItem = work
        DispatchQueue.main.asyncAfter(deadline: .now() + autoHideAfter, execute: work)
    }

    func hideTransientMessage(animated: Bool = true) {
        cancelTransientMessageAutoHide()
        let state = transientMessageHostState
        state.banner?.setVisible(false, animated: animated, generation: nextVisibilityGeneration(in: state))
    }

    func removeTransientMessage() {
        cancelTransientMessageAutoHide()
        let state = transientMessageHostState
        state.visibilityGeneration += 1
        state.banner?.layer.removeAllAnimations()
        state.banner?.removeFromSuperview()
        state.banner = nil
    }

    /// Positions the banner below the host’s safe area, centered horizontally.
    func layoutTransientMessage(topInset: CGFloat = 12) {
        guard let banner = transientMessageHostState.banner,
              !banner.isHidden || banner.alpha > 0
        else { return }

        let size = banner.sizeThatFits(bounds.size)
        let x = (bounds.width - size.width) / 2
        let y = safeAreaInsets.top + topInset
        banner.frame = CGRect(x: x, y: y, width: size.width, height: size.height)
    }

    func bringTransientMessageToFrontIfNeeded() {
        guard let banner = transientMessageHostState.banner, !banner.isHidden else { return }
        bringSubviewToFront(banner)
    }

    private func transientMessageBanner(in state: TransientMessageHostState) -> TransientMessageBannerView {
        if let banner = state.banner {
            return banner
        }
        let banner = TransientMessageBannerView()
        state.banner = banner
        addSubview(banner)
        return banner
    }

    private func nextVisibilityGeneration(in state: TransientMessageHostState) -> Int {
        state.visibilityGeneration += 1
        return state.visibilityGeneration
    }

    private func cancelTransientMessageAutoHide() {
        transientMessageHostState.autoHideWorkItem?.cancel()
        transientMessageHostState.autoHideWorkItem = nil
    }
}
