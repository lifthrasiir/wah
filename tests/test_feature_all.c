#define WAH_COMPILED_FEATURES WAH_FEATURE_ALL
#define WAH_IMPLEMENTATION
#include "../wah.h"
#include "features.c"
int main(void) { return run_feature_tests(); }
