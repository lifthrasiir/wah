#define WAH_IMPLEMENTATION
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "../wah.h"

#define WAH_FUZZ_MAX_INPUT_SIZE (10 * 1024 * 1024)
#define WAH_FUZZ_CALL_FUEL 100000
#define WAH_FUZZ_MAX_MEMORY_BYTES (64 * 1024 * 1024)

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0 || size > WAH_FUZZ_MAX_INPUT_SIZE) {
        return 0;
    }

    wah_module_t module = {0};
    wah_exec_context_t exec_ctx = {0};
    wah_parse_options_t parse_options = {
        .features = WAH_DEFAULT_FEATURES,
        .enable_fuel_metering = true,
    };

    wah_error_t err = wah_parse_module_ex(data, size, &parse_options, &module);
    if (err != WAH_OK) {
        return 0;
    }

    wah_rlimits_t limits = {
        .max_memory_bytes = WAH_FUZZ_MAX_MEMORY_BYTES,
    };
    err = wah_exec_context_create_with_limits(&exec_ctx, &module, &limits);
    if (err != WAH_OK) {
        wah_free_module(&module);
        return 0;
    }

    err = wah_instantiate(&exec_ctx);
    if (err != WAH_OK) {
        goto cleanup;
    }

    for (uint32_t i = 0; i < wah_module_export_count(&module); i++) {
        wah_export_desc_t entry;
        err = wah_module_export(&module, i, &entry);
        if (err != WAH_OK) {
            goto cleanup;
        }

        if (entry.kind != WAH_KIND_FUNCTION || entry.u.func.param_count != 0) {
            continue;
        }

        wah_value_t results[16];
        uint32_t actual_results = 0;
        uint32_t max_results = entry.u.func.result_count <= 16 ? entry.u.func.result_count : 16;

        wah_set_fuel(&exec_ctx, WAH_FUZZ_CALL_FUEL);
        err = wah_call_multi(&exec_ctx, entry.index, NULL, 0, results, max_results, &actual_results);
        if (err == WAH_STATUS_FUEL_EXHAUSTED) {
            err = WAH_OK;
            continue;
        }
        if (err != WAH_OK) {
            goto cleanup;
        }
    }

cleanup:
    wah_exec_context_destroy(&exec_ctx);
    wah_free_module(&module);
    return 0;
}
