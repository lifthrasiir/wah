// The implementation of WAH, built by build.rs.
#define WAH_IMPLEMENTATION
#ifdef WAH_SYS_JIT
#include "wahjit.h"
#else
#include "wah.h"
#endif

// WAH_COMPILED_FEATURES as compiled, against which the Rust constant is tested.
const uint64_t wah_sys_compiled_features = WAH_COMPILED_FEATURES;
