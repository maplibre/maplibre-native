#import "MLNFoundation_Private.h"

/// Initializes the run loop shim that lives on the main thread.
void MLNInitializeRunLoop() { static mln::util::RunLoop runLoop; }
