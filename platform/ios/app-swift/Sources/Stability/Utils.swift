import Foundation
import MapLibre

import MachO

private let stabilityRandomSeedEnvironmentKey = "MLN_STABILITY_RANDOM_SEED"
private let stabilityRandomSeedDefault: UInt64 = 42

func stabilityRandomSeed() -> UInt64 {
    guard let raw = ProcessInfo.processInfo.environment[stabilityRandomSeedEnvironmentKey],
          !raw.isEmpty,
          let seed = UInt64(raw)
    else {
        return stabilityRandomSeedDefault
    }
    return seed
}

struct RandomNumberGeneratorWithSeed: RandomNumberGenerator {
    private var s0: UInt64
    private var s1: UInt64

    init(seed: UInt64) {
        // SplitMix64 expands a single seed into two well-mixed, non-zero states.
        func splitmix64(_ z: inout UInt64) -> UInt64 {
            z &+= 0x9E37_79B9_7F4A_7C15
            var result = z
            result = (result ^ (result >> 30)) &* 0xBF58_476D_1CE4_E5B9
            result = (result ^ (result >> 27)) &* 0x94D0_49BB_1331_11EB
            return result ^ (result >> 31)
        }
        var z = seed
        s0 = splitmix64(&z)
        s1 = splitmix64(&z)
    }

    mutating func next() -> UInt64 {
        // xoroshiro128++
        let result = rotl(s0 &+ s1, 17) &+ s0
        let t = s1 ^ s0
        s0 = rotl(s0, 49) ^ t ^ (t << 21)
        s1 = rotl(t, 28)
        return result
    }

    private func rotl(_ x: UInt64, _ k: UInt64) -> UInt64 {
        (x << k) | (x >> (64 &- k))
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
/// order, and repeated `open()` calls are ignored. Cancelling the waiting task also
/// opens the gate so the waiter is not stranded.
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
    private let lock = NSLock()

    func wait() async {
        if isAlreadyOpen { return }

        await withTaskCancellationHandler {
            await withCheckedContinuation { continuation in
                self.suspend(continuation)
            }
        } onCancel: {
            self.open()
        }
    }

    private var isAlreadyOpen: Bool {
        lock.lock()
        defer { lock.unlock() }
        return isOpen
    }

    private func suspend(_ continuation: CheckedContinuation<Void, Never>) {
        lock.lock()
        if isOpen {
            lock.unlock()
            continuation.resume()
        } else {
            self.continuation = continuation
            lock.unlock()
        }
    }

    func open() {
        lock.lock()
        if isOpen {
            lock.unlock()
            return
        }
        isOpen = true
        let pending = continuation
        continuation = nil
        lock.unlock()
        pending?.resume()
    }
}

extension MLNMapView {
    @MainActor func animate(camera: MLNMapCamera, withDuration duration: TimeInterval, gate: Gate) async {
        beginCameraAnimation(camera, duration: duration, gate: gate)
        await gate.wait()
    }

    /// Synchronous so the completion-handler `setCamera` is not called directly
    /// from an async function.
    @MainActor private func beginCameraAnimation(_ camera: MLNMapCamera, duration: TimeInterval, gate: Gate) {
        setCamera(camera, withDuration: duration, animationTimingFunction: nil) {
            gate.open()
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
