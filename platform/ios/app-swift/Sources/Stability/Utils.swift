import MapLibre

struct RandomNumberGeneratorWithSeed: RandomNumberGenerator {
    private var state1: UInt64
    private var state2: UInt64

    init(seed: UInt64) {
        state1 = seed
        state2 = seed + 1
    }

    mutating func next() -> UInt64 {
        let s1 = state1
        let s2 = state2

        state1 = s2
        state2 = s1 ^ (s1 << 23)
        state2 ^= s2 ^ (s2 >> 17)
        state2 ^= s1 ^ (s1 >> 26)

        return state1 &+ state2
    }
}

extension Int {
    @MainActor func `repeat`(f: () async -> Void) async {
        for _ in 0 ..< self {
            await f()
        }
    }

    func `repeat`(f: () -> Void) {
        for _ in 0 ..< self {
            f()
        }
    }
}

extension MLNMapView {
    @MainActor func animate(camera: MLNMapCamera, withDuration duration: TimeInterval) async {
        await withCheckedContinuation { continuation in
            self.setCamera(camera, withDuration: duration, animationTimingFunction: nil) {
                continuation.resume()
            }
        }
    }
}
