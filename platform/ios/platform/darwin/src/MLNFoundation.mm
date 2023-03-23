#import "MLNFoundation_Private.h"

/// Initializes the run loop shim that lives on the main thread.
void MLNInitializeRunLoop() {
    static mbgl::util::RunLoop runLoop;
}
