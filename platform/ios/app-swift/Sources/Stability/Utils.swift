import MapLibre

import MachO

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

func getMemoryUsage() -> UInt64 {
    var info = mach_task_basic_info()
    var count = mach_msg_type_number_t(MemoryLayout<mach_task_basic_info>.size) / 4

    let result: kern_return_t = withUnsafeMutablePointer(to: &info) {
        $0.withMemoryRebound(to: integer_t.self, capacity: Int(count)) {
            task_info(mach_task_self_, task_flavor_t(MACH_TASK_BASIC_INFO), $0, &count)
        }
    }

    guard result == KERN_SUCCESS else { return 0 }
    return UInt64(info.resident_size)
}

func printMemoryUsage() {
    print("Total memory: \(Float(ProcessInfo.processInfo.physicalMemory) / 1024 / 1024) MB")
    print("Used memory: \(getMemoryUsage() / 1024 / 1024) MB")
}
