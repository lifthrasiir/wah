#define WAH_COMPILED_FEATURES (WAH_FEATURE_ALL & ~WAH_FEATURE_GC & ~WAH_FEATURE_TYPED_FUNCREF)
#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "features.c"
int main(void) { return run_feature_tests(); }
