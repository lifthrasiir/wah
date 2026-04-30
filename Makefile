CC ?= gcc
CFLAGS ?= -W -Wall -Wextra
LDFLAGS ?= -lm
SANITIZERS ?= address,undefined

ifneq ($(OS),Windows_NT)
    LDFLAGS += -pthread
endif

ifdef DEBUG
    CFLAGS += -DWAH_DEBUG -g -O0
else
    CFLAGS += -DWAH_ASSERT=assert -O2
endif

ifneq ($(strip $(SANITIZERS)),)
    SANITIZER_FLAGS := -fsanitize=$(SANITIZERS) -fno-omit-frame-pointer
ifneq ($(findstring undefined,$(SANITIZERS)),)
    SANITIZER_FLAGS += -DWAH_SANITIZE_UNDEFINED
endif
endif

TEST_CFLAGS := $(CFLAGS) $(SANITIZER_FLAGS)
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

tests/wah_impl.o: tests/wah_impl.c tests/wah_impl.h wah.h
	@echo "## Compiling wah_impl.o..."
	@$(CC) $(TEST_CFLAGS) -c $< -o $@

tests/common.o: tests/common.c tests/common.h wah.h
	@echo "## Compiling common.o..."
	@$(CC) $(TEST_CFLAGS) -c $< -o $@

# --- Test binaries ---

# Standalone tests (have WAH_IMPLEMENTATION): link with common.o
$(patsubst %.c, %, $(IMPL_SRCS)): tests/test_%: tests/test_%.c tests/common.o wah.h tests/common.h
	@echo "## Compiling test_$*.c..."
	@$(CC) $(TEST_CFLAGS) -c $< -o tests/test_$*.o
	@$(CC) tests/test_$*.o tests/common.o -o $@ $(TEST_LDFLAGS)
	@rm -f tests/test_$*.o

tests/test_spectest: tests/spectest.c tests/wast.c

# API-only tests: link with wah_impl.o + common.o
$(patsubst %.c, %, $(API_SRCS)): tests/test_%: tests/test_%.c tests/wah_impl.o tests/common.o wah.h tests/common.h tests/wah_impl.h
	@echo "## Compiling test_$*.c..."
	@$(CC) $(TEST_CFLAGS) -c $< -o tests/test_$*.o
	@$(CC) tests/test_$*.o tests/wah_impl.o tests/common.o -o $@ $(TEST_LDFLAGS)
	@rm -f tests/test_$*.o

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
test: $(RUN_TARGETS)
	@echo "## All tests passed."

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

.PHONY: bench
bench: bench/bench_coremark
	@echo "## Running CoreMark benchmark..."
	@./bench/bench_coremark

bench/bench_coremark: bench/bench_coremark.c wah.h
	@echo "## Compiling bench_coremark..."
	@$(CC) -W -Wall -Wextra -DWAH_ASSERT=assert -O3 -c $< -o bench/bench_coremark.o
	@$(CC) bench/bench_coremark.o -o $@ $(LDFLAGS)
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

FUZZ_CFLAGS := $(filter-out -O%,$(CFLAGS)) $(FUZZ_OPT) $(SANITIZER_FLAGS)
FUZZ_LDFLAGS := $(LDFLAGS) $(SANITIZER_FLAGS)
LIBFUZZER_CFLAGS := $(filter-out -O%,$(CFLAGS)) $(FUZZ_OPT) -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer -DWAH_SANITIZE_UNDEFINED
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
	@rm -f tests/*.o
	@rm -f $(FUZZ_HARNESS_BIN)
	@rm -f $(LIBFUZZER_HARNESS_BIN)
	@rm -f bench/bench_coremark
	@rm -f *.gcda *.gcno tests/*.gcda tests/*.gcno fuzz/*.gcda fuzz/*.gcno coverage.info
	@rm -rf cov/ coverage_report/
