#define WAH_COMPILED_FEATURES WAH_FEATURE_WASM_V2
#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "features.c"
int main(void) { return run_feature_tests(); }
