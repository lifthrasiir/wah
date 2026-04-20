// Specification test runner for tests/spectest/*.bin.wast.
// Include this file after defining WAH_IMPLEMENTATION and including wah.h and wast.c.
// Provides run_parser_sanity() and run_file() for use by a test driver.

#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

typedef struct {
    void *ptr;
    uint32_t id;
} host_ref_t;

typedef struct {
    char *name;
    wah_module_t module;
    int valid;
} spectest_module_def_t;

typedef struct {
    char *name;
    spectest_module_def_t *def;
    wah_exec_context_t exec;
    int live;
} spectest_instance_t;

typedef struct {
    char *name;
    spectest_module_def_t *def;
} registered_module_t;

typedef struct {
    uint32_t total_checks;
    uint32_t passed_checks;
    uint32_t failed_checks;
    uint32_t files_run;
    uint32_t files_failed;
    uint32_t files_skipped;
} tally_t;

typedef struct {
    const char *path;
    tally_t tally;

    spectest_module_def_t *defs;
    size_t def_count;
    size_t def_capacity;

    spectest_instance_t *instances;
    size_t instance_count;
    size_t instance_capacity;

    registered_module_t *registered;
    size_t registered_count;
    size_t registered_capacity;

    spectest_module_def_t *current_def;
    spectest_instance_t *current_instance;
    int current_line;

    wah_module_t spectest_host;
    int host_ready;

    host_ref_t *host_refs;
    size_t host_ref_count;
    size_t host_ref_capacity;

    int skip_file;
} spectest_env_t;


static void no_op_host_func(wah_call_context_t *ctx, void *userdata) {
    (void)ctx;
    (void)userdata;
}

static void fail_check(spectest_env_t *env, const char *fmt, ...) {
    va_list args;
    env->tally.total_checks++;
    env->tally.failed_checks++;
    if (env->current_line > 0) {
        fprintf(stderr, "[FAIL] %s:%d: ", env->path ? env->path : "<sanity>", env->current_line);
    } else {
        fprintf(stderr, "[FAIL] %s: ", env->path ? env->path : "<sanity>");
    }
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
}

static void pass_check(spectest_env_t *env) {
    env->tally.total_checks++;
    env->tally.passed_checks++;
}

static int ensure_capacity(void **items, size_t *capacity, size_t elem_size, size_t count) {
    void *new_items;
    size_t new_capacity;
    if (count <= *capacity) {
        return 1;
    }
    new_capacity = (*capacity == 0) ? 8 : (*capacity * 2);
    while (new_capacity < count) {
        new_capacity *= 2;
    }
    new_items = realloc(*items, new_capacity * elem_size);
    if (!new_items) {
        return 0;
    }
    *items = new_items;
    *capacity = new_capacity;
    return 1;
}

static void *host_ref_for_id(spectest_env_t *env, uint32_t id) {
    size_t i;
    uint32_t *storage;
    for (i = 0; i < env->host_ref_count; ++i) {
        if (env->host_refs[i].id == id) {
            return env->host_refs[i].ptr;
        }
    }
    if (!ensure_capacity((void **)&env->host_refs, &env->host_ref_capacity,
                         sizeof(*env->host_refs), env->host_ref_count + 1)) {
        return NULL;
    }
    storage = (uint32_t *)malloc(sizeof(*storage));
    if (!storage) {
        return NULL;
    }
    *storage = id;
    env->host_refs[env->host_ref_count].id = id;
    env->host_refs[env->host_ref_count].ptr = storage;
    env->host_ref_count++;
    return storage;
}

static spectest_module_def_t *find_def_by_name(spectest_env_t *env, const char *name) {
    size_t i;
    for (i = 0; i < env->def_count; ++i) {
        if (env->defs[i].name && strcmp(env->defs[i].name, name) == 0) {
            return &env->defs[i];
        }
    }
    return NULL;
}

static spectest_instance_t *find_instance_by_name(spectest_env_t *env, const char *name) {
    size_t i;
    for (i = 0; i < env->instance_count; ++i) {
        if (env->instances[i].live && env->instances[i].name &&
            strcmp(env->instances[i].name, name) == 0) {
            return &env->instances[i];
        }
    }
    return NULL;
}

static spectest_instance_t *resolve_instance(spectest_env_t *env, const wast_node_t *name_node) {
    char *name;
    spectest_instance_t *instance;
    if (!name_node) {
        return env->current_instance;
    }
    name = wast_atom_dup_cstr(name_node);
    if (!name) {
        return NULL;
    }
    instance = find_instance_by_name(env, name);
    free(name);
    return instance;
}

static spectest_module_def_t *resolve_def(spectest_env_t *env, const wast_node_t *name_node) {
    char *name;
    spectest_module_def_t *def;
    if (!name_node) {
        return env->current_def;
    }
    name = wast_atom_dup_cstr(name_node);
    if (!name) {
        return NULL;
    }
    def = find_def_by_name(env, name);
    free(name);
    return def;
}

static void *spectest_resolve_ref(void *userdata, uint32_t id) {
    return host_ref_for_id((spectest_env_t *)userdata, id);
}

static int is_func_ref(const spectest_instance_t *instance, void *ref) {
    uintptr_t start;
    uintptr_t end;
    uintptr_t ptr;
    if (!instance || !instance->live || !ref || !instance->exec.function_table ||
        instance->exec.function_table_count == 0) {
        return 0;
    }
    start = (uintptr_t)&instance->exec.function_table[0];
    end = (uintptr_t)(&instance->exec.function_table[instance->exec.function_table_count]);
    ptr = (uintptr_t)ref;
    return ptr >= start && ptr < end;
}

static int match_num_pat_i32(int32_t actual, const wast_num_pat_t *pat) {
    return pat->kind == WAST_NUM_LITERAL && actual == pat->literal.i32;
}

static int match_num_pat_i64(int64_t actual, const wast_num_pat_t *pat) {
    return pat->kind == WAST_NUM_LITERAL && actual == pat->literal.i64;
}

static int match_num_pat_f32(uint32_t bits, const wast_num_pat_t *pat) {
    if (pat->kind == WAST_NUM_LITERAL) {
        return bits == pat->literal.f32_bits;
    }
    if ((bits & 0x7f800000u) != 0x7f800000u || (bits & 0x007fffffu) == 0) {
        return 0;
    }
    if (pat->kind == WAST_NUM_NAN_CANONICAL) {
        return bits == 0x7fc00000u || bits == 0xffc00000u;
    }
    return (bits & 0x00400000u) != 0;
}

static int match_num_pat_f64(uint64_t bits, const wast_num_pat_t *pat) {
    if (pat->kind == WAST_NUM_LITERAL) {
        return bits == pat->literal.f64_bits;
    }
    if ((bits & UINT64_C(0x7ff0000000000000)) != UINT64_C(0x7ff0000000000000) ||
        (bits & UINT64_C(0x000fffffffffffff)) == 0) {
        return 0;
    }
    if (pat->kind == WAST_NUM_NAN_CANONICAL) {
        return bits == UINT64_C(0x7ff8000000000000) || bits == UINT64_C(0xfff8000000000000);
    }
    return (bits & UINT64_C(0x0008000000000000)) != 0;
}

static int match_single_pattern(const spectest_env_t *env,
                                const spectest_instance_t *instance,
                                const wast_const_value_t *actual,
                                const wast_result_pat_t *pat) {
    (void)env;
    if (pat->kind == WAST_PAT_EITHER) {
        size_t i;
        for (i = 0; i < pat->alternative_count; ++i) {
            if (match_single_pattern(env, instance, actual, &pat->alternatives[i])) {
                return 1;
            }
        }
        return 0;
    }
    switch (pat->kind) {
        case WAST_PAT_I32:
            return actual->type == WAH_TYPE_I32 && match_num_pat_i32(actual->value.i32, &pat->scalar);
        case WAST_PAT_I64:
            return actual->type == WAH_TYPE_I64 && match_num_pat_i64(actual->value.i64, &pat->scalar);
        case WAST_PAT_F32: {
            uint32_t bits;
            if (actual->type != WAH_TYPE_F32) return 0;
            memcpy(&bits, &actual->value.f32, sizeof(bits));
            return match_num_pat_f32(bits, &pat->scalar);
        }
        case WAST_PAT_F64: {
            uint64_t bits;
            if (actual->type != WAH_TYPE_F64) return 0;
            memcpy(&bits, &actual->value.f64, sizeof(bits));
            return match_num_pat_f64(bits, &pat->scalar);
        }
        case WAST_PAT_V128: {
            uint8_t bytes[16];
            size_t i;
            if (actual->type != WAH_TYPE_V128) return 0;
            memcpy(bytes, &actual->value.v128, sizeof(bytes));
            if (strcmp(pat->shape, "i8x16") == 0) {
                for (i = 0; i < 16; ++i) {
                    if (!match_num_pat_i32((uint8_t)bytes[i], &pat->lanes[i])) return 0;
                }
                return 1;
            }
            if (strcmp(pat->shape, "i16x8") == 0) {
                for (i = 0; i < 8; ++i) {
                    uint16_t lane = (uint16_t)bytes[i * 2] | ((uint16_t)bytes[i * 2 + 1] << 8);
                    if (!match_num_pat_i32((int32_t)lane, &pat->lanes[i])) return 0;
                }
                return 1;
            }
            if (strcmp(pat->shape, "i32x4") == 0) {
                for (i = 0; i < 4; ++i) {
                    int32_t lane = (int32_t)wast_read_u32_le(bytes + i * 4);
                    if (!match_num_pat_i32(lane, &pat->lanes[i])) return 0;
                }
                return 1;
            }
            if (strcmp(pat->shape, "i64x2") == 0) {
                for (i = 0; i < 2; ++i) {
                    int64_t lane = (int64_t)wast_read_u64_le(bytes + i * 8);
                    if (!match_num_pat_i64(lane, &pat->lanes[i])) return 0;
                }
                return 1;
            }
            if (strcmp(pat->shape, "f32x4") == 0) {
                for (i = 0; i < 4; ++i) {
                    if (!match_num_pat_f32(wast_read_u32_le(bytes + i * 4), &pat->lanes[i])) return 0;
                }
                return 1;
            }
            if (strcmp(pat->shape, "f64x2") == 0) {
                for (i = 0; i < 2; ++i) {
                    if (!match_num_pat_f64(wast_read_u64_le(bytes + i * 8), &pat->lanes[i])) return 0;
                }
                return 1;
            }
            return 0;
        }
        case WAST_PAT_REF_ANY:
            return actual->value.ref != NULL;
        case WAST_PAT_REF_NULL:
            return actual->value.ref == NULL;
        case WAST_PAT_REF_FUNC:
            return actual->value.ref != NULL && is_func_ref(instance, actual->value.ref);
        case WAST_PAT_REF_EXTERN:
            return actual->value.ref != NULL;
        case WAST_PAT_REF_HOST:
            if (actual->value.ref == NULL) return 0;
            if (pat->host_ref_id == 0 && !pat->ref_kind_name) return 1;
            return actual->value.ref == host_ref_for_id((spectest_env_t *)env, pat->host_ref_id);
        case WAST_PAT_REF_OTHER:
            if (!strcmp(pat->ref_kind_name, "ref.array") ||
                !strcmp(pat->ref_kind_name, "ref.struct") ||
                !strcmp(pat->ref_kind_name, "ref.i31") ||
                !strcmp(pat->ref_kind_name, "ref.eq") ||
                !strcmp(pat->ref_kind_name, "ref.i")) {
                return actual->value.ref != NULL;
            }
            return actual->value.ref != NULL;
        default:
            return 0;
    }
}

static int ensure_instance_instantiated(spectest_instance_t *instance) {
    if (!instance || !instance->live) {
        return 0;
    }
    if (!instance->exec.is_instantiated) {
        return wah_instantiate(&instance->exec) == WAH_OK;
    }
    return 1;
}

static int action_export(spectest_instance_t *instance,
                         const char *field_name,
                         wah_entry_t *entry,
                         char *error_buf,
                         size_t error_buf_size) {
    wah_error_t err;
    if (!instance) {
        snprintf(error_buf, error_buf_size, "no current instance");
        return 0;
    }
    err = wah_module_export_by_name(instance->def ? &instance->def->module : instance->exec.module, field_name, entry);
    if (err != WAH_OK) {
        snprintf(error_buf, error_buf_size, "export \"%s\" not found (%s)", field_name, wah_strerror(err));
        return 0;
    }
    return 1;
}

static int execute_action(const wast_node_t *action_node,
                          spectest_env_t *env,
                          wast_const_result_t *result,
                          wah_error_t *out_err,
                          spectest_instance_t **out_instance,
                          char *error_buf,
                          size_t error_buf_size) {
    spectest_instance_t *instance = NULL;
    char *field_name = NULL;
    wah_entry_t entry = {0};
    size_t arg_index = 1;
    size_t param_count = 0;
    uint32_t actual_results = 0;
    wah_value_t params[32];
    wast_const_value_t parsed_values[32];
    if (out_err) *out_err = WAH_OK;
    result->count = 0;
    if (!action_node || action_node->is_atom || action_node->child_count < 2) {
        snprintf(error_buf, error_buf_size, "invalid action");
        return 0;
    }
    if (wast_atom_eq(action_node->children[0], "invoke")) {
        if (action_node->child_count >= 3 && action_node->children[1]->is_atom &&
            !action_node->children[1]->atom.is_string) {
            instance = resolve_instance(env, action_node->children[1]);
            arg_index = 2;
        } else {
            instance = env->current_instance;
        }
        if (!instance) {
            snprintf(error_buf, error_buf_size, "invoke without active instance");
            return 0;
        }
        if (arg_index >= action_node->child_count || !action_node->children[arg_index]->is_atom ||
            !action_node->children[arg_index]->atom.is_string) {
            snprintf(error_buf, error_buf_size, "invoke missing export name");
            return 0;
        }
        field_name = wast_atom_dup_cstr(action_node->children[arg_index]);
        if (!field_name) {
            snprintf(error_buf, error_buf_size, "out of memory");
            return 0;
        }
        if (!action_export(instance, field_name, &entry, error_buf, error_buf_size)) {
            free(field_name);
            return 0;
        }
        if (!WAH_TYPE_IS_FUNCTION(entry.type)) {
            snprintf(error_buf, error_buf_size, "\"%s\" is not a function export", field_name);
            free(field_name);
            return 0;
        }
        for (++arg_index; arg_index < action_node->child_count; ++arg_index) {
            if (param_count >= sizeof(params) / sizeof(params[0])) {
                snprintf(error_buf, error_buf_size, "too many invoke arguments");
                free(field_name);
                return 0;
            }
            if (!wast_parse_const(action_node->children[arg_index], &parsed_values[param_count], spectest_resolve_ref, env)) {
                snprintf(error_buf, error_buf_size, "unsupported invoke argument");
                free(field_name);
                return 0;
            }
            params[param_count] = parsed_values[param_count].value;
            param_count++;
        }
        {
            wah_value_t raw_results[WAST_MAX_RESULTS];
            wah_error_t err = wah_call_multi(&instance->exec, entry.id, params,
                                             (uint32_t)param_count,
                                             raw_results, WAST_MAX_RESULTS, &actual_results);
            if (out_err) *out_err = err;
            if (err == WAH_OK) {
                uint32_t i, nargs = 0, nrets = 0;
                const wah_type_t *arg_types = NULL;
                const wah_type_t *ret_types = NULL;
                (void)wah_entry_func(&entry, &nargs, &arg_types, &nrets, &ret_types);
                for (i = 0; i < actual_results; ++i) {
                    result->values[i].value = raw_results[i];
                    result->values[i].type = ret_types ? ret_types[i] : WAH_TYPE_I32;
                }
                result->count = actual_results;
            }
        }
        free(field_name);
        if (out_instance) *out_instance = instance;
        return 1;
    }
    if (wast_atom_eq(action_node->children[0], "get")) {
        uint32_t global_idx;
        if (action_node->child_count == 3 && action_node->children[1]->is_atom &&
            !action_node->children[1]->atom.is_string) {
            instance = resolve_instance(env, action_node->children[1]);
            arg_index = 2;
        } else {
            instance = env->current_instance;
            arg_index = 1;
        }
        if (!instance) {
            snprintf(error_buf, error_buf_size, "get without active instance");
            return 0;
        }
        if (arg_index >= action_node->child_count || !action_node->children[arg_index]->is_atom ||
            !action_node->children[arg_index]->atom.is_string) {
            snprintf(error_buf, error_buf_size, "get missing export name");
            return 0;
        }
        field_name = wast_atom_dup_cstr(action_node->children[arg_index]);
        if (!field_name) {
            return 0;
        }
        if (!action_export(instance, field_name, &entry, error_buf, error_buf_size)) {
            free(field_name);
            return 0;
        }
        if (WAH_GET_ENTRY_KIND(entry.id) != WAH_ENTRY_KIND_GLOBAL) {
            snprintf(error_buf, error_buf_size, "\"%s\" is not a global export", field_name);
            free(field_name);
            return 0;
        }
        if (!ensure_instance_instantiated(instance)) {
            if (out_err) *out_err = WAH_ERROR_TRAP;
            free(field_name);
            return 1;
        }
        global_idx = WAH_GET_ENTRY_INDEX(entry.id);
        if (global_idx >= instance->exec.global_count) {
            snprintf(error_buf, error_buf_size, "global index out of range");
            free(field_name);
            return 0;
        }
        result->values[0].type = entry.type;
        result->values[0].value = instance->exec.globals[global_idx];
        result->count = 1;
        if (out_instance) *out_instance = instance;
        free(field_name);
        return 1;
    }
    snprintf(error_buf, error_buf_size, "unsupported action");
    return 0;
}

static int add_table_export(wah_module_t *mod, const char *name, uint32_t min, uint32_t max) {
    char *name_copy;
    wah_table_type_t *new_tables = (wah_table_type_t *)realloc(
        mod->tables, (mod->table_count + 1) * sizeof(*mod->tables));
    if (!new_tables) return 0;
    mod->tables = new_tables;
    mod->tables[mod->table_count].elem_type = WAH_TYPE_FUNCREF;
    mod->tables[mod->table_count].min_elements = min;
    mod->tables[mod->table_count].max_elements = max;

    if (!ensure_capacity((void **)&mod->exports, (size_t *)&mod->capacity_exports,
                         sizeof(*mod->exports), mod->export_count + 1)) {
        return 0;
    }
    name_copy = strdup(name);
    if (!name_copy) {
        return 0;
    }
    mod->exports[mod->export_count].name = name_copy;
    mod->exports[mod->export_count].name_len = strlen(name_copy);
    mod->exports[mod->export_count].kind = 1;
    mod->exports[mod->export_count].index = mod->table_count;
    mod->export_count++;
    mod->table_count++;
    return 1;
}

static int setup_spectest_host(spectest_env_t *env) {
    if (env->host_ready) {
        return 1;
    }
    if (wah_new_module(&env->spectest_host) != WAH_OK) return 0;
    if (wah_module_export_global_i32(&env->spectest_host, "global_i32", 0, 666) != WAH_OK) return 0;
    if (wah_module_export_global_i64(&env->spectest_host, "global_i64", 0, 666) != WAH_OK) return 0;
    if (wah_module_export_global_f32(&env->spectest_host, "global_f32", 0, 666.6f) != WAH_OK) return 0;
    if (wah_module_export_global_f64(&env->spectest_host, "global_f64", 0, 666.6) != WAH_OK) return 0;
    if (wah_module_export_memory(&env->spectest_host, "memory", 1, 2) != WAH_OK) return 0;
    if (!add_table_export(&env->spectest_host, "table", 10, 20)) return 0;
    if (wah_module_export_func(&env->spectest_host, "print", "->", no_op_host_func, NULL, NULL) != WAH_OK) return 0;
    if (wah_module_export_func(&env->spectest_host, "print_i32", "i32->", no_op_host_func, NULL, NULL) != WAH_OK) return 0;
    if (wah_module_export_func(&env->spectest_host, "print_i64", "i64->", no_op_host_func, NULL, NULL) != WAH_OK) return 0;
    if (wah_module_export_func(&env->spectest_host, "print_f32", "f32->", no_op_host_func, NULL, NULL) != WAH_OK) return 0;
    if (wah_module_export_func(&env->spectest_host, "print_f64", "f64->", no_op_host_func, NULL, NULL) != WAH_OK) return 0;
    if (wah_module_export_func(&env->spectest_host, "print_i32_f32", "i32,f32->", no_op_host_func, NULL, NULL) != WAH_OK) return 0;
    if (wah_module_export_func(&env->spectest_host, "print_f64_f64", "f64,f64->", no_op_host_func, NULL, NULL) != WAH_OK) return 0;
    env->host_ready = 1;
    return 1;
}

static spectest_module_def_t *add_module_def(spectest_env_t *env, const char *name) {
    spectest_module_def_t *def;
    spectest_module_def_t *old_base = env->defs;
    if (!ensure_capacity((void **)&env->defs, &env->def_capacity, sizeof(*env->defs), env->def_count + 1)) {
        return NULL;
    }
    if (env->defs != old_base && old_base != NULL) {
        ptrdiff_t delta = (char *)env->defs - (char *)old_base;
        for (size_t i = 0; i < env->instance_count; i++) {
            if (env->instances[i].def)
                env->instances[i].def = (spectest_module_def_t *)((char *)env->instances[i].def + delta);
        }
        for (size_t i = 0; i < env->registered_count; i++) {
            if (env->registered[i].def)
                env->registered[i].def = (spectest_module_def_t *)((char *)env->registered[i].def + delta);
        }
        if (env->current_def)
            env->current_def = (spectest_module_def_t *)((char *)env->current_def + delta);
    }
    def = &env->defs[env->def_count++];
    memset(def, 0, sizeof(*def));
    if (name) {
        def->name = strdup(name);
        if (!def->name) {
            return NULL;
        }
    }
    return def;
}

static spectest_instance_t *add_instance(spectest_env_t *env, const char *name, spectest_module_def_t *def) {
    spectest_instance_t *instance;
    spectest_instance_t *old_base = env->instances;
    if (!ensure_capacity((void **)&env->instances, &env->instance_capacity,
                         sizeof(*env->instances), env->instance_count + 1)) {
        return NULL;
    }
    if (env->instances != old_base && old_base != NULL) {
        ptrdiff_t delta = (char *)env->instances - (char *)old_base;
        if (env->current_instance)
            env->current_instance = (spectest_instance_t *)((char *)env->current_instance + delta);
    }
    instance = &env->instances[env->instance_count++];
    memset(instance, 0, sizeof(*instance));
    instance->def = def;
    instance->live = 1;
    if (name) {
        instance->name = strdup(name);
        if (!instance->name) {
            return NULL;
        }
    }
    return instance;
}

static int register_instance_import_name(spectest_env_t *env, spectest_instance_t *instance, const char *name) {
    registered_module_t *item;
    if (!ensure_capacity((void **)&env->registered, &env->registered_capacity,
                         sizeof(*env->registered), env->registered_count + 1)) {
        return 0;
    }
    item = &env->registered[env->registered_count++];
    memset(item, 0, sizeof(*item));
    item->name = strdup(name);
    if (!item->name) {
        return 0;
    }
    item->def = instance->def;
    return 1;
}

static wah_error_t instantiate_def_into_instance(spectest_env_t *env, spectest_module_def_t *def, spectest_instance_t *instance) {
    size_t i;
    wah_error_t err;
    if (!setup_spectest_host(env)) {
        return WAH_ERROR_OUT_OF_MEMORY;
    }
    err = wah_exec_context_create(&instance->exec, &def->module);
    if (err != WAH_OK) {
        return err;
    }
    err = wah_link_module(&instance->exec, "spectest", &env->spectest_host);
    if (err != WAH_OK) {
        return err;
    }
    for (i = 0; i < env->registered_count; ++i) {
        if (!env->registered[i].def || env->registered[i].def == def) {
            continue;
        }
        err = wah_link_module(&instance->exec, env->registered[i].name, &env->registered[i].def->module);
        if (err != WAH_OK) {
            return err;
        }
    }
    return wah_instantiate(&instance->exec);
}

static int eval_module_command(const wast_node_t *node, spectest_env_t *env, spectest_module_def_t **out_def) {
    uint8_t *bytes = NULL;
    size_t bytes_len = 0;
    char *name = NULL;
    int is_definition = 0;
    size_t i;
    spectest_module_def_t *def = NULL;
    *out_def = NULL;
    if (!node || node->is_atom || node->child_count < 2 || !wast_atom_eq(node->children[0], "module")) {
        return 0;
    }
    for (i = 1; i < node->child_count; ++i) {
        if (!node->children[i]->is_atom) continue;
        if (wast_atom_eq(node->children[i], "definition")) {
            is_definition = 1;
        } else if (!node->children[i]->atom.is_string && node->children[i]->atom.data[0] == '$') {
            free(name);
            name = wast_atom_dup_cstr(node->children[i]);
        }
    }
    if (!wast_gather_binary_module(node, &bytes, &bytes_len)) {
        free(name);
        return 0;
    }
    def = add_module_def(env, name);
    free(name);
    if (!def) {
        free(bytes);
        return 0;
    }
    {
        wah_error_t parse_err = wah_parse_module(bytes, bytes_len, &def->module);
        if (parse_err == WAH_OK) {
            def->valid = 1;
            env->current_def = def;
            *out_def = def;
        } else if (parse_err == WAH_ERROR_UNIMPLEMENTED) {
            fprintf(stderr, "[SKIP] %s:%u: %s\n", env->path, node->line, wah_strerror(parse_err));
            env->skip_file = 1;
        } else {
            fprintf(stderr, "[DEBUG] parse failed in %s:%u: %s\n", env->path, node->line, wah_strerror(parse_err));
        }
    }
    free(bytes);
    (void)is_definition;
    return 1;
}

static int expect_malformed(wah_error_t err) {
    return err == WAH_ERROR_INVALID_MAGIC_NUMBER ||
           err == WAH_ERROR_INVALID_VERSION ||
           err == WAH_ERROR_UNEXPECTED_EOF ||
           err == WAH_ERROR_TOO_LARGE;
}

static int expect_unlinkable(wah_error_t err) {
    return err == WAH_ERROR_IMPORT_NOT_FOUND || err == WAH_ERROR_VALIDATION_FAILED;
}

static int expect_trap_like(wah_error_t err) {
    return err == WAH_ERROR_TRAP || err == WAH_ERROR_MEMORY_OUT_OF_BOUNDS;
}

static int execute_command(const wast_node_t *node, spectest_env_t *env);

static int handle_assert_return(const wast_node_t *node, spectest_env_t *env) {
    wast_const_result_t actual;
    spectest_instance_t *instance = NULL;
    wast_result_pat_t patterns[WAST_MAX_RESULTS];
    size_t i;
    char error_buf[256] = {0};
    wah_error_t err = WAH_OK;
    int ok = 1;
    memset(patterns, 0, sizeof(patterns));
    if (!execute_action(node->children[1], env, &actual, &err, &instance, error_buf, sizeof(error_buf))) {
        fail_check(env, "%s", error_buf);
        return 0;
    }
    if (err != WAH_OK) {
        fail_check(env, "assert_return action failed with %s", wah_strerror(err));
        return 0;
    }
    if (actual.count != node->child_count - 2) {
        fail_check(env, "result count mismatch: expected %zu, got %u", node->child_count - 2, actual.count);
        return 0;
    }
    for (i = 2; i < node->child_count && i - 2 < WAST_MAX_RESULTS; ++i) {
        if (!wast_parse_result_pattern(node->children[i], &patterns[i - 2])) {
            fail_check(env, "unsupported result pattern");
            ok = 0;
            break;
        }
    }
    if (ok) {
        for (i = 0; i < actual.count; ++i) {
            if (!match_single_pattern(env, instance, &actual.values[i], &patterns[i])) {
                if (actual.values[i].type == WAH_TYPE_I32) {
                    fail_check(env, "result %zu did not match expected pattern (got i32:%d)", i, actual.values[i].value.i32);
                } else if (actual.values[i].type == WAH_TYPE_I64) {
                    fail_check(env, "result %zu did not match expected pattern (got i64:%lld)", i, (long long)actual.values[i].value.i64);
                } else if (actual.values[i].type == WAH_TYPE_F32) {
                    fail_check(env, "result %zu did not match expected pattern (got f32:%g)", i, (double)actual.values[i].value.f32);
                } else if (actual.values[i].type == WAH_TYPE_F64) {
                    fail_check(env, "result %zu did not match expected pattern (got f64:%g)", i, actual.values[i].value.f64);
                } else {
                    fail_check(env, "result %zu did not match expected pattern (type=%d)", i, actual.values[i].type);
                }
                ok = 0;
                break;
            }
        }
    }
    for (i = 0; i < node->child_count - 2 && i < WAST_MAX_RESULTS; ++i) {
        wast_free_result_pattern(&patterns[i]);
    }
    if (ok) pass_check(env);
    return ok;
}

static int handle_assert_trap_like(const wast_node_t *node, spectest_env_t *env, int exhaustion) {
    wast_const_result_t actual;
    spectest_instance_t *instance = NULL;
    char error_buf[256] = {0};
    wah_error_t err = WAH_OK;
    (void)actual;
    (void)instance;
    if (!execute_action(node->children[1], env, &actual, &err, &instance, error_buf, sizeof(error_buf))) {
        fail_check(env, "%s", error_buf);
        return 0;
    }
    if ((!exhaustion && expect_trap_like(err)) ||
        (exhaustion && (err == WAH_ERROR_CALL_STACK_OVERFLOW || err == WAH_ERROR_OUT_OF_MEMORY || err == WAH_ERROR_TRAP))) {
        pass_check(env);
        return 1;
    }
    fail_check(env, "%s expected, got %s",
               exhaustion ? "exhaustion" : "trap", wah_strerror(err));
    return 0;
}

static int handle_assert_exception(const wast_node_t *node, spectest_env_t *env) {
    return handle_assert_trap_like(node, env, 0);
}

static int handle_module_instance(const wast_node_t *node, spectest_env_t *env, int expect_failure_kind) {
    spectest_module_def_t *def = NULL;
    spectest_instance_t tmp = {0};
    spectest_instance_t *instance = &tmp;
    char *instance_name = NULL;
    size_t i;
    wah_error_t err;
    for (i = 1; i < node->child_count; ++i) {
        if (!node->children[i]->is_atom || node->children[i]->atom.is_string) continue;
        if (wast_atom_eq(node->children[i], "instance")) continue;
        if (node->children[i]->atom.data[0] == '$' && !instance_name) {
            instance_name = wast_atom_dup_cstr(node->children[i]);
        } else if (node->children[i]->atom.data[0] == '$') {
            def = resolve_def(env, node->children[i]);
        }
    }
    if (!def) {
        def = env->current_def;
    }
    if (!def || !def->valid) {
        fail_check(env, "module instance without valid definition");
        free(instance_name);
        return 0;
    }
    if (expect_failure_kind == 0) {
        instance = add_instance(env, instance_name, def);
        if (!instance) {
            fail_check(env, "out of memory");
            free(instance_name);
            return 0;
        }
    } else {
        memset(instance, 0, sizeof(*instance));
        instance->def = def;
    }
    free(instance_name);
    err = instantiate_def_into_instance(env, def, instance);
    if (expect_failure_kind == 1) {
        if (expect_unlinkable(err)) {
            pass_check(env);
            return 1;
        }
        fail_check(env, "expected unlinkable, got %s", wah_strerror(err));
        return 0;
    }
    if (expect_failure_kind == 2) {
        if (expect_trap_like(err)) {
            pass_check(env);
            return 1;
        }
        fail_check(env, "expected instantiation trap, got %s", wah_strerror(err));
        return 0;
    }
    if (err != WAH_OK) {
        fail_check(env, "module instantiation failed: %s", wah_strerror(err));
        return 0;
    }
    env->current_instance = instance;
    return 1;
}

static int execute_command(const wast_node_t *node, spectest_env_t *env) {
    char *name = NULL;
    spectest_instance_t *instance;
    spectest_module_def_t *def = NULL;
    uint8_t *bytes = NULL;
    size_t bytes_len = 0;
    wah_error_t err;
    env->current_line = node ? node->line : 0;
    if (!node || node->is_atom || node->child_count == 0 || !node->children[0]->is_atom) {
        fail_check(env, "invalid top-level command");
        return 0;
    }
    if (wast_atom_eq(node->children[0], "module") && node->child_count >= 2 &&
        node->children[1]->is_atom && wast_atom_eq(node->children[1], "instance")) {
        return handle_module_instance(node, env, 0);
    }
    if (wast_atom_eq(node->children[0], "module")) {
        if (!eval_module_command(node, env, &def)) {
            fail_check(env, "failed to decode module command");
            return 0;
        }
        if (node->child_count >= 2 && !wast_atom_eq(node->children[1], "definition")) {
            return handle_module_instance(node, env, 0);
        }
        return 1;
    }
    if (wast_atom_eq(node->children[0], "register") && node->child_count >= 2 &&
        node->children[1]->is_atom && node->children[1]->atom.is_string) {
        instance = (node->child_count >= 3) ? resolve_instance(env, node->children[2]) : env->current_instance;
        if (!instance || !instance->def) {
            fail_check(env, "register without instance");
            return 0;
        }
        name = wast_atom_dup_cstr(node->children[1]);
        if (!name || !register_instance_import_name(env, instance, name)) {
            fail_check(env, "failed to register module");
            free(name);
            return 0;
        }
        free(name);
        return 1;
    }
    if (wast_atom_eq(node->children[0], "invoke") || wast_atom_eq(node->children[0], "get")) {
        wast_const_result_t actual;
        spectest_instance_t *owner = NULL;
        char error_buf[256] = {0};
        if (!execute_action(node, env, &actual, &err, &owner, error_buf, sizeof(error_buf))) {
            fail_check(env, "%s", error_buf);
            return 0;
        }
        if (err == WAH_OK) {
            pass_check(env);
            return 1;
        }
        fail_check(env, "action failed: %s", wah_strerror(err));
        return 0;
    }
    if (wast_atom_eq(node->children[0], "assert_return")) return handle_assert_return(node, env);
    if (wast_atom_eq(node->children[0], "assert_trap")) {
        if (node->child_count >= 2 && !node->children[1]->is_atom &&
            wast_atom_eq(node->children[1]->children[0], "module")) {
            return handle_module_instance(node->children[1], env, 2);
        }
        return handle_assert_trap_like(node, env, 0);
    }
    if (wast_atom_eq(node->children[0], "assert_exhaustion")) return handle_assert_trap_like(node, env, 1);
    if (wast_atom_eq(node->children[0], "assert_exception")) return handle_assert_exception(node, env);
    if (wast_atom_eq(node->children[0], "assert_unlinkable") && node->child_count >= 2) {
        return handle_module_instance(node->children[1], env, 1);
    }
    if ((wast_atom_eq(node->children[0], "assert_invalid") ||
         wast_atom_eq(node->children[0], "assert_malformed")) &&
        node->child_count >= 2) {
        if (!wast_gather_binary_module(node->children[1], &bytes, &bytes_len)) {
            fail_check(env, "could not gather binary module");
            return 0;
        }
        {
            wah_module_t mod = {0};
            err = wah_parse_module(bytes, bytes_len, &mod);
            wah_free_module(&mod);
            free(bytes);
            if (err == WAH_ERROR_UNIMPLEMENTED) {
                env->skip_file = 1;
                return 1;
            }
            if (wast_atom_eq(node->children[0], "assert_invalid")) {
                if (err == WAH_ERROR_VALIDATION_FAILED) {
                    pass_check(env);
                    return 1;
                }
                fail_check(env, "expected invalid module, got %s", wah_strerror(err));
                return 0;
            }
            if (expect_malformed(err)) {
                pass_check(env);
                return 1;
            }
            fail_check(env, "expected malformed module, got %s", wah_strerror(err));
            return 0;
        }
    }
    fail_check(env, "unsupported command");
    return 0;
}

static void free_env(spectest_env_t *env) {
    size_t i;
    for (i = 0; i < env->instance_count; ++i) {
        if (env->instances[i].live) {
            wah_exec_context_destroy(&env->instances[i].exec);
        }
        free(env->instances[i].name);
    }
    for (i = 0; i < env->def_count; ++i) {
        if (env->defs[i].valid) {
            wah_free_module(&env->defs[i].module);
        }
        free(env->defs[i].name);
    }
    for (i = 0; i < env->registered_count; ++i) {
        free(env->registered[i].name);
    }
    for (i = 0; i < env->host_ref_count; ++i) {
        free(env->host_refs[i].ptr);
    }
    if (env->host_ready) {
        wah_free_module(&env->spectest_host);
    }
    free(env->instances);
    free(env->defs);
    free(env->registered);
    free(env->host_refs);
}

static int run_parser_sanity(void) {
    const char *source =
        ";; leading comment\n"
        "(module definition $M binary \"\\00\\61\\73\\6d\" \"\\01\\00\\00\\00\")\n"
        "(module instance $I $M)\n"
        "(assert_return (invoke \"f\" (i32.const 0x2a)) (i32.const 0x2a))\n";
    wast_parser_t parser;
    wast_node_t *node;
    int count = 0;
    wast_parser_init(&parser, source, strlen(source));
    while ((node = wast_parse_next(&parser)) != NULL) {
        if (node->is_atom || node->child_count == 0) {
            fprintf(stderr, "parser sanity failed: top-level atom\n");
            wast_node_free(node);
            return 0;
        }
        count++;
        wast_node_free(node);
    }
    if (parser.error[0] != '\0' || count != 3) {
        fprintf(stderr, "parser sanity failed: %s\n", parser.error[0] ? parser.error : "unexpected count");
        return 0;
    }
    return 1;
}

static int run_file(const char *path, tally_t *grand_total) {
    spectest_env_t env;
    char *source;
    size_t source_len = 0;
    wast_parser_t parser;
    wast_node_t *node;
    memset(&env, 0, sizeof(env));
    env.path = path;
    source = wast_read_file(path, &source_len);
    if (!source) {
        fprintf(stderr, "[FAIL] %s: could not read file\n", path);
        grand_total->files_failed++;
        grand_total->files_run++;
        grand_total->failed_checks++;
        grand_total->total_checks++;
        return 0;
    }
    wast_parser_init(&parser, source, source_len);
    while ((node = wast_parse_next(&parser)) != NULL) {
        if (!env.skip_file) {
            execute_command(node, &env);
        }
        wast_node_free(node);
    }
    if (parser.error[0] != '\0' && !env.skip_file) {
        env.current_line = parser.line;
        fail_check(&env, "parse error: %s", parser.error);
    }
    if (env.skip_file) {
        printf("%s: SKIPPED (unimplemented feature)\n", path);
        grand_total->files_skipped++;
        free_env(&env);
        free(source);
        return 1;
    }
    printf("%s: %u passed, %u failed\n", path, env.tally.passed_checks, env.tally.failed_checks);
    grand_total->total_checks += env.tally.total_checks;
    grand_total->passed_checks += env.tally.passed_checks;
    grand_total->failed_checks += env.tally.failed_checks;
    grand_total->files_run++;
    if (env.tally.failed_checks != 0) {
        grand_total->files_failed++;
    }
    free_env(&env);
    free(source);
    return env.tally.failed_checks == 0;
}
