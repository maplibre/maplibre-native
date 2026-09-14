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

/// A one-shot awaitable gate. `wait()` suspends until `open()` is called, in either
/// order, and repeated `open()` calls are ignored.
///
/// Map callbacks are not guaranteed to run: a style load can fail, and an in-flight
/// camera animation stops advancing as soon as the map view leaves the window (its
/// display link is destroyed), so its completion handler is never invoked. A bare
/// `CheckedContinuation` in that situation is never resumed, which strands the
/// awaiting task and — because the task holds the map view — leaks the whole map,
/// renderer and tile cache. The gate lets teardown release the waiter explicitly.
class Gate {
    private var continuation: CheckedContinuation<Void, Never>?
    private var isOpen = false

    func wait() async {
        if isOpen { return }

        await withCheckedContinuation { continuation in
            if isOpen {
                continuation.resume()
            } else {
                self.continuation = continuation
            }
        }
    }

    func open() {
        if isOpen { return }

        isOpen = true
        continuation?.resume()
        continuation = nil
    }
}

extension MLNMapView {
    @MainActor func animate(camera: MLNMapCamera, withDuration duration: TimeInterval, gate: Gate) async {
        setCamera(camera, withDuration: duration, animationTimingFunction: nil) {
            gate.open()
        }

        await gate.wait()
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
