//

//

class RefHolder {
    let ref: _baseRef
    private let release: ((_baseRef) -> Void)?

    init(_ ref: _baseRef) {
        self.ref = ref
        release = nil
    }

    init(ref: _baseRef, release: @escaping (_baseRef) -> Void) {
        self.ref = ref
        self.release = release
    }

    deinit {
        if let fun = release {
            fun(ref)
        }
    }
}

typealias _baseRef = Int64

struct PrimitiveHolder<T> {
    let ref: T

    init(_ ref: T) {
        self.ref = ref
    }
}
