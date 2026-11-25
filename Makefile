CC ?= gcc
CXX ?= g++
CFLAGS ?= -W -Wall -Wextra
CXXFLAGS ?= -W -Wall -Wextra -std=c++11
LDFLAGS ?= -lm
SANITIZERS ?= address,undefined

ifneq ($(OS),Windows_NT)
    LDFLAGS += -pthread
endif

UNAME_S := $(shell uname -s 2>/dev/null)

ifdef DEBUG
    CFLAGS += -DWAH_DEBUG -g -O0
else
    CFLAGS += -DWAH_ASSERT=assert -O2
endif

ifneq ($(strip $(SANITIZERS)),)
    SANITIZER_FLAGS := -fsanitize=$(SANITIZERS) -fno-omit-frame-pointer
ifneq ($(findstring undefined,$(SANITIZERS)),)
    SANITIZER_FLAGS += -DWAH_SANITIZE_UNDEFINED -fno-sanitize-recover=undefined
endif
endif

# JIT=1 builds wah_impl.o with wahjit.h, so that API-only tests (including spec tests) run natively where possible.
ifdef JIT
    TEST_CFLAGS_JIT := -DWAH_TEST_JIT
endif

# Lowered code is always checked in tests
TEST_CFLAGS := $(CFLAGS) -DWAH_CHECK_LOWERED $(SANITIZER_FLAGS) $(TEST_CFLAGS_JIT)
TEST_CXXFLAGS := $(CXXFLAGS) $(SANITIZER_FLAGS)
TEST_LDFLAGS := $(LDFLAGS) $(SANITIZER_FLAGS)

TEST_RUN_PREFIX ?=
ifneq ($(strip $(SANITIZERS)),)
ifdef WSL_DISTRO_NAME
ifeq ($(strip $(TEST_RUN_PREFIX)),)
    TEST_RUN_PREFIX := setarch $(shell uname -m) -R
endif
endif
endif

ALL_TEST_SRCS := $(sort $(wildcard tests/test_*.c))
ALL_TEST_NAMES := $(patsubst tests/test_%.c, %, $(ALL_TEST_SRCS))

# Classify tests by whether they define WAH_IMPLEMENTATION.
# Standalone tests embed the wah implementation; API-only tests link against wah_impl.o.
IMPL_SRCS := $(shell grep -l 'define WAH_IMPLEMENTATION' $(ALL_TEST_SRCS) 2>/dev/null)
API_SRCS := $(filter-out $(IMPL_SRCS), $(ALL_TEST_SRCS))

# --- Precompiled objects ---

# Records whether JIT was given, so that switching it rebuilds wah_impl.o and relinks tests
tests/.build_mode: FORCE
	@echo "$(if $(JIT),jit,interp)" | cmp -s - $@ || echo "$(if $(JIT),jit,interp)" > $@

.PHONY: FORCE
FORCE:

tests/wah_impl.o: tests/wah_impl.c tests/wah_impl.h wah.h wahjit.h tests/.build_mode
	@echo "## Compiling wah_impl.o..."
	@$(CC) $(TEST_CFLAGS) -c $< -o $@

tests/common.o: tests/common.c tests/common.h wah.h
	@echo "## Compiling common.o..."
	@$(CC) $(TEST_CFLAGS) -c $< -o $@

# --- Test binaries ---

# Standalone tests (have WAH_IMPLEMENTATION): link with common.o
$(patsubst %.c, %, $(IMPL_SRCS)): tests/test_%: tests/test_%.c tests/common.o wah.h wahjit.h tests/common.h
	@echo "## Compiling test_$*.c..."
	@$(CC) $(TEST_CFLAGS) -c $< -o tests/test_$*.o
	@$(CC) tests/test_$*.o tests/common.o -o $@ $(TEST_LDFLAGS)
	@rm -f tests/test_$*.o

tests/test_spectest: tests/spectest.c tests/wast.c
$(patsubst %.c, %, $(filter tests/test_feature_%, $(IMPL_SRCS))): tests/features.c

# API-only tests: link with wah_impl.o + common.o
$(patsubst %.c, %, $(API_SRCS)): tests/test_%: tests/test_%.c tests/wah_impl.o tests/common.o wah.h tests/common.h tests/wah_impl.h
	@echo "## Compiling test_$*.c..."
	@$(CC) $(TEST_CFLAGS) -c $< -o tests/test_$*.o
	@$(CC) tests/test_$*.o tests/wah_impl.o tests/common.o -o $@ $(TEST_LDFLAGS)
	@rm -f tests/test_$*.o

tests/test_cpp: tests/test_cpp.cpp tests/wah_impl.o wah.h tests/wah_impl.h
	@echo "## Compiling test_cpp.cpp..."
	@$(CXX) $(TEST_CXXFLAGS) -c $< -o tests/test_cpp.o
	@$(CXX) tests/test_cpp.o tests/wah_impl.o -o $@ $(TEST_LDFLAGS)
	@rm -f tests/test_cpp.o

# --- Run targets ---
# test depends on all run_X; each run_X depends on its binary.
# With -j2, make can compile one test while running another.

RUN_TARGETS := $(patsubst %, run_%, $(ALL_TEST_NAMES))

define MAKE_RUN_RULE
.PHONY: run_$(1)
run_$(1): tests/test_$(1)
	@echo "## Running test_$(1).c..."
	@$(TEST_RUN_PREFIX) ./tests/test_$(1)
	@echo ""
endef
$(foreach t,$(ALL_TEST_NAMES),$(eval $(call MAKE_RUN_RULE,$(t))))

.PHONY: all test
all: test
test: $(RUN_TARGETS) test-cpp
	@echo "## All tests passed."

.PHONY: test-cpp
test-cpp: tests/test_cpp
	@echo "## Running test_cpp.cpp..."
	@$(TEST_RUN_PREFIX) ./tests/test_cpp
	@echo ""

# --- Individual test targets (make test_basic, etc.) ---

define MAKE_SINGLE_TEST
.PHONY: test_$(1)
test_$(1): tests/test_$(1)
	@echo "## Running test_$(1).c..."
	@$(TEST_RUN_PREFIX) ./tests/test_$(1)
	@echo ""
endef
$(foreach t,$(ALL_TEST_NAMES),$(eval $(call MAKE_SINGLE_TEST,$(t))))

# --- Benchmark ---

BENCH_CFLAGS :=
BENCH_LDFLAGS :=

ifeq ($(UNAME_S),Darwin)
    BENCH_CFLAGS += -DWAH_BENCH_KPC_CLOCK
    BENCH_LDFLAGS += -F/System/Library/PrivateFrameworks -framework kperf
endif

.PHONY: bench
bench: bench/bench_coremark
	@echo "## Running CoreMark benchmark..."
	@./bench/bench_coremark

bench/bench_coremark: bench/bench_coremark.c wah.h
	@echo "## Compiling bench_coremark..."
	@$(CC) -W -Wall -Wextra -DWAH_ASSERT=assert $(BENCH_CFLAGS) -O3 -c $< -o bench/bench_coremark.o
	@$(CC) bench/bench_coremark.o -o $@ $(LDFLAGS) $(BENCH_LDFLAGS)
	@rm -f bench/bench_coremark.o

# --- Fuzzing ---

FUZZ_HARNESS_SRC := fuzz/fuzz_afl.c
FUZZ_HARNESS_BIN := fuzz/fuzz_afl
LIBFUZZER_HARNESS_SRC := fuzz/fuzz_libfuzzer.c
LIBFUZZER_HARNESS_BIN := fuzz/fuzz_libfuzzer
LIBFUZZER_CORPUS_DIR ?= fuzz/in_dir
LIBFUZZER_TIMEOUT ?= 60
IN_DIR ?= fuzz/in_dir
OUT_DIR ?= fuzz/out_dir
AFL_CC ?= afl-clang
LIBFUZZER_CC ?= clang
FUZZ_OPT ?= -O0

# Lowered code is checked as in tests, so that any validator or lowering bug the fuzzer reaches aborts
FUZZ_CFLAGS := $(filter-out -O%,$(CFLAGS)) $(FUZZ_OPT) -DWAH_CHECK_LOWERED $(SANITIZER_FLAGS)
FUZZ_LDFLAGS := $(LDFLAGS) $(SANITIZER_FLAGS)
LIBFUZZER_CFLAGS := $(filter-out -O%,$(CFLAGS)) $(FUZZ_OPT) -DWAH_CHECK_LOWERED -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer -DWAH_SANITIZE_UNDEFINED
LIBFUZZER_LDFLAGS := $(LDFLAGS) -fsanitize=fuzzer,address,undefined

.PHONY: fuzz-afl
fuzz-afl: $(FUZZ_HARNESS_BIN)
	afl-fuzz -i $(IN_DIR) -o $(OUT_DIR) -- ./$(FUZZ_HARNESS_BIN)

$(FUZZ_HARNESS_BIN): $(FUZZ_HARNESS_SRC)
	AFL_DONT_OPTIMIZE=1 $(AFL_CC) $(FUZZ_CFLAGS) $< -o $@ $(FUZZ_LDFLAGS)

.PHONY: fuzz-libfuzzer
fuzz-libfuzzer: $(LIBFUZZER_HARNESS_BIN)
	./$(LIBFUZZER_HARNESS_BIN) -timeout=$(LIBFUZZER_TIMEOUT) $(LIBFUZZER_CORPUS_DIR)

$(LIBFUZZER_HARNESS_BIN): $(LIBFUZZER_HARNESS_SRC) wah.h
	$(LIBFUZZER_CC) $(LIBFUZZER_CFLAGS) $< -o $@ $(LIBFUZZER_LDFLAGS)

# --- CBMC proofs ---
# Each entry is FILE.HARNESS, checking proofs/FILE.c from harness_HARNESS with loops unwound up to CBMC_UNWIND_FILE
# and extra flags CBMC_FLAGS_FILE. Conversions to signed types and unsigned wraparound are intended, so they aren't
# checked by default. Logs leave out passing properties, which are tens of thousands over the whole implementation.

CBMC ?= cbmc
CBMC_FLAGS := --bounds-check --pointer-check --signed-overflow-check --undefined-shift-check --unwinding-assertions
CBMC_FLOAT_BOUNDARY_PROOFS := $(foreach t, trunc trunc_sat, $(t)_f32_to_i32 $(t)_f32_to_i64 $(t)_f64_to_i64)
CBMC_FLOAT_PROOFS := $(foreach n, 32 64, $(patsubst %, float.%_f$(n), min max nearest) \
	$(foreach t, i32 u32 i64 u64, float.trunc_f$(n)_to_$(t) float.trunc_sat_f$(n)_to_$(t)))
CBMC_PROOFS := leb128.uleb128 leb128.uleb128_64 leb128.sleb128_32 leb128.sleb128_64 utf8.valid \
	bounds.effective_addr bounds.range_in_bounds bounds.count decode.opcode decode.memarg decode.val_type \
	decode.storage_type $(CBMC_FLOAT_PROOFS) $(patsubst %, float_boundary.%, $(CBMC_FLOAT_BOUNDARY_PROOFS)) \
	$(foreach e, 0 1 3 16 huge, alloc.malloc_$(e) alloc.realloc_$(e)) pin.ops refs.i31 refs.gc_link \
	fuel.bulk_limit fuel.bulk_limit_elems fuel.budget layout.place layout.field_class \
	types.ref_eq types.hash_ref types.types_eq types.is_subtype types.accepts_repr types.matches_supertype
CBMC_UNWIND_leb128 := 12
CBMC_UNWIND_utf8 := 9
CBMC_UNWIND_bounds := 9
CBMC_UNWIND_decode := 22
CBMC_UNWIND_float := 2
CBMC_UNWIND_float_boundary := 2
CBMC_FLAGS_float := --conversion-check
CBMC_UNWIND_alloc := 9
CBMC_FLAGS_alloc := --memory-leak-check
CBMC_UNWIND_pin := 8
CBMC_UNWIND_refs := 2
CBMC_UNWIND_fuel := 2
CBMC_UNWIND_layout := 6
CBMC_UNWIND_types := 6

CBMC_TARGETS := $(patsubst %, cbmc_%, $(CBMC_PROOFS))

.PHONY: cbmc $(CBMC_TARGETS)
cbmc: $(CBMC_TARGETS)
	@echo "## All proofs passed."

$(CBMC_TARGETS): cbmc_%: FORCE
	@echo "## Proving harness_$(word 2,$(subst ., ,$*)) in proofs/$(word 1,$(subst ., ,$*)).c..."
	@$(CBMC) proofs/$(word 1,$(subst ., ,$*)).c --function harness_$(word 2,$(subst ., ,$*)) \
		--unwind $(CBMC_UNWIND_$(word 1,$(subst ., ,$*))) $(CBMC_FLAGS) $(CBMC_FLAGS_$(word 1,$(subst ., ,$*))) \
		> proofs/$*.log.tmp 2>&1; status=$$?; \
		grep -v ': SUCCESS$$' proofs/$*.log.tmp > proofs/$*.log; rm -f proofs/$*.log.tmp; \
		[ $$status -eq 0 ] || { grep -E 'FAILURE|ERROR|error' proofs/$*.log; exit 1; }

# Proofs that interpreter handlers move the stack and the instruction pointer as the abstract machine of the Lean model
# of the lowered code checker does (proofs/step.c), one CBMC run for each opcode that proofs/step_ops.c lists.
CBMC_STEP_DIR := proofs/.step

.PHONY: cbmc-step
# Each run is killed beyond CBMC_STEP_SECS seconds or CBMC_STEP_MB megabytes (by proofs/guard.pl), as runs that go
# wrong can take all memory and hang the machine (runs take under 800 MB). CBMC_STEP_JOBS runs at once, by default
# as many as cores and two thirds of the memory allow.
CBMC_STEP_SECS ?= 300
CBMC_STEP_MB ?= 1500
CBMC_STEP_RAM_MB := $(shell (sysctl -n hw.memsize 2>/dev/null || awk '/MemTotal/ {print $$2 * 1024}' /proc/meminfo) | \
	awk '{print int($$1 / 1048576)}')
CBMC_STEP_JOBS ?= $(shell n=$$(getconf _NPROCESSORS_ONLN); m=$$(( $(CBMC_STEP_RAM_MB) * 2 / 3 / $(CBMC_STEP_MB) )); \
	[ $$m -lt 1 ] && m=1; [ $$m -lt $$n ] && n=$$m; echo $$n)
cbmc-step:
	@echo "## Proving interpreter handlers against the abstract machine..."
	@mkdir -p $(CBMC_STEP_DIR)
	@$(CC) -O1 -w -o $(CBMC_STEP_DIR)/ops proofs/step_ops.c -lm
	@$(CBMC_STEP_DIR)/ops | xargs -P $(CBMC_STEP_JOBS) -L 1 sh -c \
		'h=$$0; op=$$1; name=$$(echo "$$*" | tr " =" "__"); shift; \
		proofs/guard.pl $(CBMC_STEP_SECS) $(CBMC_STEP_MB) $(CBMC) proofs/step.c -DSTEP_OP=WAH_OP_$$op "$$@" \
			--function harness_$$h --unwind 17 $(CBMC_FLAGS) > $(CBMC_STEP_DIR)/$$name.log 2>&1 || \
			{ echo "harness_$$h for $$op $$*:"; grep -E "FAILURE|^guard:" $(CBMC_STEP_DIR)/$$name.log; exit 1; }'
	@echo "## All $$($(CBMC_STEP_DIR)/ops | wc -l | tr -d ' ') runs of handlers move as the abstract machine does."

# --- Lean proofs ---
# Builds proofs/lean, rejects unproven or unsound constructs, and checks that every theorem only uses the standard
# axioms. Proofs must not rely on code generation (native_decide etc.) either.

LAKE ?= lake
LEAN_FORBIDDEN := \b(sorry|admit|native_decide|bv_decide)\b|^\s*((private|protected|noncomputable)\s+)*(axiom|unsafe|opaque)\b|@\[[^]]*\b(extern|implemented_by)\b

.PHONY: lean
lean:
	@echo "## Building Lean proofs..."
	@cd proofs/lean && $(LAKE) build
	@! grep -rnE '$(LEAN_FORBIDDEN)' proofs/lean/WahProofs proofs/lean/WahProofs.lean || \
		{ echo "Forbidden constructs in Lean proofs"; exit 1; }
	@cd proofs/lean && $(LAKE) env lean Audit.lean
	@echo "## All Lean proofs passed."

# Proofs against the Lean model of Wasm 3.0 that SpecTec generates (WahSpecTec.lean), pinned to a commit of
# sgraf812/spectec-wp and fetched into .lake, with proofs/lean/spectec/model.patch: the model that the generator
# fixed by proofs/lean/spectec/generator.patch generates. The model takes minutes to elaborate the first time.
SPECTEC_WP_REV := c8f6efb767384785473c10d3cc618d15461b1d6d
SPECTEC_WP_SHA256 := 3eeb9832941cabe951b23bf8bd420b7e3aacc3c2a68d8ee6385295432ca23acb
SPECTEC_MODEL_SHA256 := 83b496e41ba9b552a2be81482b94e0cf55523253fe857b1b14ba1e7407ee2b41
SPECTEC_MODEL := proofs/lean/.lake/spectec/SpecTec/Wasm.lean

.PHONY: lean-spectec
lean-spectec:
	@echo "## Building Lean proofs against the SpecTec model..."
	@echo "$(SPECTEC_MODEL_SHA256)  $(SPECTEC_MODEL)" | shasum -a 256 -c --status 2>/dev/null || { \
		mkdir -p $(dir $(SPECTEC_MODEL)) && \
		curl -sfL -o $(SPECTEC_MODEL) \
			https://raw.githubusercontent.com/sgraf812/spectec-wp/$(SPECTEC_WP_REV)/SpecTecWP/Wasm.lean && \
		echo "$(SPECTEC_WP_SHA256)  $(SPECTEC_MODEL)" | shasum -a 256 -c --status && \
		patch -s $(SPECTEC_MODEL) proofs/lean/spectec/model.patch && \
		echo "$(SPECTEC_MODEL_SHA256)  $(SPECTEC_MODEL)" | shasum -a 256 -c --status; } || \
		{ echo "Failed to fetch the SpecTec model"; exit 1; }
	@cd proofs/lean && $(LAKE) build WahSpecTec
	@! grep -rnE '$(LEAN_FORBIDDEN)' proofs/lean/WahSpecTec proofs/lean/WahSpecTec.lean || \
		{ echo "Forbidden constructs in Lean proofs"; exit 1; }
	@cd proofs/lean && $(LAKE) env lean AuditSpecTec.lean
	@echo "## All Lean proofs against the SpecTec model passed."

# Compares canonical maps and subtyping of the spec tests and the fuzz corpus with the Lean models, as a check of the
# loops that the CBMC proofs don't cover or only cover for a few types. The second run makes all rec groups collide,
# so that every pair of them is compared structurally.
LEAN_DIFF_DIR := proofs/lean/.lake/diff
LEAN_DIFF_CORPUS := fuzz/corpus-24269.tar.gz

.PHONY: lean-diff
lean-diff:
	@echo "## Comparing canonical maps with the Lean model..."
	@cd proofs/lean && $(LAKE) build wah_types_oracle
	@rm -rf $(LEAN_DIFF_DIR) && mkdir -p $(LEAN_DIFF_DIR)/corpus && tar -xzf $(LEAN_DIFF_CORPUS) -C $(LEAN_DIFF_DIR)/corpus
	@$(CC) -O1 -Wall -Wno-unused-function -o $(LEAN_DIFF_DIR)/dump_types proofs/lean/dump_types.c -lm
	@$(CC) -O1 -Wall -Wno-unused-function -DDUMP_COLLIDE -o $(LEAN_DIFF_DIR)/dump_collide proofs/lean/dump_types.c -lm
	@for d in dump_types dump_collide; do \
		{ $(LEAN_DIFF_DIR)/$$d tests/spectest/*.wast && \
		  find $(LEAN_DIFF_DIR)/corpus -type f -print0 | xargs -0 $(LEAN_DIFF_DIR)/$$d; } | \
		proofs/lean/.lake/build/bin/wah_types_oracle || exit 1; \
	done
	@echo "## Canonical maps and subtyping agree with the Lean models."
	@echo "## Comparing verdicts of the lowered code checker with the Lean model..."
	@cd proofs/lean && $(LAKE) build wah_lowered_oracle
	@$(CC) -O1 -Wall -Wno-unused-function -o $(LEAN_DIFF_DIR)/dump_lowered proofs/lean/dump_lowered.c -lm
	@{ $(LEAN_DIFF_DIR)/dump_lowered tests/spectest/*.wast && \
	   $(LEAN_DIFF_DIR)/dump_lowered -m 64 proofs/lean/lowered_extra.wasm && \
	   find $(LEAN_DIFF_DIR)/corpus -type f -print0 | xargs -0 $(LEAN_DIFF_DIR)/dump_lowered; } | \
	   proofs/lean/.lake/build/bin/wah_lowered_oracle
	@echo "## The lowered code checker agrees with the Lean model."

# --- Comparison with the spec ---
# Runs the fuzz corpus and modules from `wasm-tools smith` through WAH and checks what it did against the reference
# interpreter of the spec (interpreter/wasm of https://github.com/WebAssembly/spec, built by `make` there), given as
# WASM_REF. Disagreements are kept in SPEC_DIFF_DIR as .wast scripts with the output of the reference interpreter.
# Smith modules have no imports, terminate (by a fuel global shared by all calls, which traps on every call once
# exhausted, so more is given than by default) and canonicalize NaNs (but see nan_variant.sh), and
# don't use relaxed SIMD, which is nondeterministic, or proposals beyond Wasm 3.0. With SPECTEC (the spectec directory
# of the spec repository, with `spectec` built there), validation and instantiation are also checked against the
# interpreter of SpecTec, which runs the spec itself (check_spectec.sh).

WASM_REF ?=
SPECTEC ?=
WASM_TOOLS ?= wasm-tools
SPEC_SMITH_COUNT ?= 5000
SPEC_DIFF_DIR := proofs/spec/.out
SPEC_SMITH_FLAGS := --max-imports 0 --min-funcs 3 --ensure-termination --fuel 10000 --canonicalize-nans true \
	--export-everything true --gc-enabled true --exceptions-enabled true --tail-call-enabled true \
	--memory64-enabled true --relaxed-simd-enabled false --threads-enabled false \
	--custom-page-sizes-enabled false --wide-arithmetic-enabled false

.PHONY: spec-diff
spec-diff:
	@[ -n "$(WASM_REF)" ] || { echo "Set WASM_REF to the reference interpreter of the spec"; exit 1; }
	@echo "## Comparing the fuzz corpus and $(SPEC_SMITH_COUNT) smith modules with the reference interpreter..."
	@rm -rf $(SPEC_DIFF_DIR) && mkdir -p $(SPEC_DIFF_DIR)/corpus $(SPEC_DIFF_DIR)/smith
	@[ -z "$(SPECTEC)" ] || mkdir -p $(SPEC_DIFF_DIR)/first
	@tar -xzf $(LEAN_DIFF_CORPUS) -C $(SPEC_DIFF_DIR)/corpus
	@for i in $$(seq 1 $(SPEC_SMITH_COUNT)); do \
		perl -e "srand($$i); print map chr(int rand 256), 1..16384" | \
		$(WASM_TOOLS) smith $(SPEC_SMITH_FLAGS) -o $(SPEC_DIFF_DIR)/smith/$$i.wasm || exit 1; \
	done
	@$(CC) -O1 -Wall -Wno-unused-function -o $(SPEC_DIFF_DIR)/wah_to_wast proofs/spec/wah_to_wast.c -lm
	@proofs/spec/nan_variant.sh wah.h $(SPEC_DIFF_DIR)/wah_nan.h
	@$(CC) -O1 -Wall -Wno-unused-function -DWAH_TO_WAST_HEADER='"$(abspath $(SPEC_DIFF_DIR))/wah_nan.h"' \
		-o $(SPEC_DIFF_DIR)/wah_to_wast_nan proofs/spec/wah_to_wast.c -lm
	@find $(SPEC_DIFF_DIR)/corpus $(SPEC_DIFF_DIR)/smith -type f -print0 | \
		xargs -0 -P $$(getconf _NPROCESSORS_ONLN) -n 1 proofs/spec/check_one.sh $(SPEC_DIFF_DIR)/wah_to_wast \
		$(SPEC_DIFF_DIR)/wah_to_wast_nan $(WASM_REF) $(SPEC_DIFF_DIR) > $(SPEC_DIFF_DIR)/results.txt 2> /dev/null
	@awk '{ n[$$1]++ } END { for (k in n) printf "%s %d, ", k, n[k]; print "" }' $(SPEC_DIFF_DIR)/results.txt
	@! grep '^fail' $(SPEC_DIFF_DIR)/results.txt || { echo "Disagreements are in $(SPEC_DIFF_DIR)"; exit 1; }
	@echo "## WAH agrees with the reference interpreter."
	@[ -z "$(SPECTEC)" ] || { \
		echo "## Comparing validation and instantiation with the SpecTec interpreter..." && \
		proofs/spec/check_spectec.sh $(abspath $(SPECTEC)) $(abspath $(SPEC_DIFF_DIR)) > $(SPEC_DIFF_DIR)/spectec.txt && \
		awk '{ n[$$1]++ } END { for (k in n) printf "%s %d, ", k, n[k]; print "" }' $(SPEC_DIFF_DIR)/spectec.txt && \
		{ ! grep '^fail' $(SPEC_DIFF_DIR)/spectec.txt || { echo "Disagreements are in $(SPEC_DIFF_DIR)/spectec"; exit 1; }; } && \
		echo "## WAH agrees with the SpecTec interpreter."; }

# --- Coverage ---

GCOV_CFLAGS := -fprofile-arcs -ftest-coverage
GCOV_LDFLAGS := -lgcov

tests/wah_impl_cov.o: tests/wah_impl.c tests/wah_impl.h wah.h
	@echo "## Compiling wah_impl_cov.o..."
	@$(CC) $(CFLAGS) $(GCOV_CFLAGS) -c $< -o $@

tests/common_cov.o: tests/common.c tests/common.h wah.h
	@echo "## Compiling common_cov.o..."
	@$(CC) $(CFLAGS) $(GCOV_CFLAGS) -c $< -o $@

# Coverage binaries for standalone tests (have WAH_IMPLEMENTATION): link with common_cov.o
$(patsubst %.c, %_cov, $(IMPL_SRCS)): tests/test_%_cov: tests/test_%.c tests/common_cov.o wah.h tests/common.h
	@echo "## Compiling test_$*_cov..."
	@$(CC) $(CFLAGS) $(GCOV_CFLAGS) -c $< -o tests/test_$*_cov.o
	@$(CC) tests/test_$*_cov.o tests/common_cov.o -o $@ $(LDFLAGS) $(GCOV_LDFLAGS)
	@rm -f tests/test_$*_cov.o

# Coverage binaries for API-only tests: link with wah_impl_cov.o + common_cov.o
$(patsubst %.c, %_cov, $(API_SRCS)): tests/test_%_cov: tests/test_%.c tests/wah_impl_cov.o tests/common_cov.o wah.h tests/common.h tests/wah_impl.h
	@echo "## Compiling test_$*_cov..."
	@$(CC) $(CFLAGS) $(GCOV_CFLAGS) -c $< -o tests/test_$*_cov.o
	@$(CC) tests/test_$*_cov.o tests/wah_impl_cov.o tests/common_cov.o -o $@ $(LDFLAGS) $(GCOV_LDFLAGS)
	@rm -f tests/test_$*_cov.o

COV_RUN_TARGETS := $(patsubst %, run_%_cov, $(ALL_TEST_NAMES))

define MAKE_COV_RUN_RULE
.PHONY: run_$(1)_cov
run_$(1)_cov: tests/test_$(1)_cov
	@echo "## Running test_$(1)_cov..."
	@./tests/test_$(1)_cov
	@echo ""
endef
$(foreach t,$(ALL_TEST_NAMES),$(eval $(call MAKE_COV_RUN_RULE,$(t))))

.PHONY: coverage
coverage: clean
	@$(MAKE) $(COV_RUN_TARGETS)
	@echo "## Generating coverage report..."
	lcov --capture --directory . --output-file coverage.info
	lcov --remove coverage.info '/usr/*' '*/tests/test_*.c' --output-file coverage.info
	genhtml coverage.info --output-directory coverage_report
	@echo "## Coverage report generated in coverage_report/index.html"

# --- Cleanup ---

.PHONY: clean
clean:
	@echo "## Cleaning up..."
	@rm -f $(patsubst %.c, %, $(ALL_TEST_SRCS)) $(patsubst %.c, %_cov, $(ALL_TEST_SRCS))
	@rm -f tests/test_cpp
	@rm -f tests/*.o tests/*.o.tmp tests/.build_mode
	@rm -f $(FUZZ_HARNESS_BIN)
	@rm -f $(LIBFUZZER_HARNESS_BIN)
	@rm -f bench/bench_coremark
	@rm -f proofs/*.log
	@rm -f *.gcda *.gcno tests/*.gcda tests/*.gcno fuzz/*.gcda fuzz/*.gcno coverage.info
	@rm -rf cov/ coverage_report/
