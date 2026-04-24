# Compiler and flags
CC ?= gcc
CFLAGS ?= -W -Wall -Wextra
LDFLAGS ?= -lm # Link math library, potentially needed for wah.h

# Debugging options
# If DEBUG environment variable is set to 1, add debugging flags and disable optimizations.
ifdef DEBUG
    CFLAGS += -DWAH_DEBUG -g -O0
else
    CFLAGS += -DWAH_ASSERT=assert -O2
endif

# List of test source files
ALL_TEST_SRCS := $(wildcard tests/test_*.c)
TEST_SRCS := $(ALL_TEST_SRCS)
# List of compiled test executables
TEST_BINS := $(patsubst %.c, %, $(TEST_SRCS))

# Rule for fuzzing with afl-fuzz
FUZZ_HARNESS_SRC := fuzz/fuzz_afl.c
FUZZ_HARNESS_BIN := fuzz/fuzz_afl

# Default fuzzing directories
IN_DIR ?= fuzz/in_dir
OUT_DIR ?= fuzz/out_dir

# Compiler for fuzzing harness (afl-clang for instrumentation)
AFL_CC ?= afl-clang # Or afl-clang-fast if available and configured

# CFLAGS for coverage generation
# These flags enable gcov instrumentation.
GCOV_CFLAGS := -fprofile-arcs -ftest-coverage

# Default target: if 'make' is run without arguments, 'test' target is executed.
.PHONY: all
all: test

# Target to compile and run all tests
# First, build all test executables, then run each one sequentially.
.PHONY: test
test: $(TEST_BINS)
	@echo "## Running all tests..."
	@for test_bin in $(TEST_BINS); do \
		echo "## Running $$test_bin..."; \
		./$$test_bin; \
		if [ $$? -ne 0 ]; then \
			echo "## $$test_bin failed."; \
			exit 1; \
		fi; \
		echo ""; \
	done
	@echo "## All tests passed."

# Rule to compile a single test source file into an executable
% : %.c
	@echo "## Compiling $<..."
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# Template for individual test rules
# $(1) is the test source file (e.g., tests/test_simd.c)
define RUN_SINGLE_TEST_TEMPLATE
TEST_NAME_$(1) := $(patsubst tests/test_%.c, %, $(1))
TEST_TARGET_$(1) := $(patsubst tests/test_%.c, test_%, $(1))

.PHONY: $(patsubst tests/test_%.c, test_%, $(1))
$(patsubst tests/test_%.c, test_%, $(1)):
	@echo "## Running $(patsubst tests/test_%.c, %, $(1))..."
	$(CC) $(CFLAGS) $(1) -o tests/$(patsubst tests/test_%.c, %, $(1)) $(LDFLAGS)
	./tests/$(patsubst tests/test_%.c, %, $(1))
	if [ $$$$? -ne 0 ]; then \
		echo "## $(patsubst tests/test_%.c, %, $(1)) failed."; \
		exit 1; \
	fi
	echo ""
	rm -f tests/$(patsubst tests/test_%.c, %, $(1))
endef

# Generate individual test targets using the template
$(foreach test_src, $(ALL_TEST_SRCS), $(eval $(call RUN_SINGLE_TEST_TEMPLATE,$(test_src))))

# New target for coverage report generation for all tests
coverage: clean
	@echo "## Compiling all tests for coverage..."
	@for test_src in $(TEST_SRCS); do \
		echo "## Compiling $$test_src with coverage flags..."; \
		test_bin=$$(basename $$test_src .c); \
		$(CC) $(CFLAGS) $(GCOV_CFLAGS) $$test_src -o tests/$$test_bin $(LDFLAGS); \
		if [ $$? -ne 0 ]; then \
			echo "## Compilation of $$test_src failed."; \
			exit 1; \
		fi; \
		done
	@echo "## Running all tests for coverage..."
	@for test_bin in $(TEST_BINS); do \
		echo "## Running $$test_bin for coverage..."; \
		./$$test_bin; \
		if [ $$? -ne 0 ]; then \
			echo "## $$test_bin failed during coverage run."; \
			exit 1; \
		fi; \
		echo ""; \
	done
	@echo "## All tests passed for coverage. Generating coverage report..."
	# Initialize lcov and capture coverage data
	lcov --capture --directory . --output-file coverage.info
	# Filter out system headers and test files from the report
	lcov --remove coverage.info '/usr/*' '*/tests/test_*.c' --output-file coverage.info
	# Generate HTML report
	genhtml coverage.info --output-directory coverage_report
	@echo "## Coverage report generated in coverage_report/index.html"

# Rule for fuzzing with afl-fuzz
.PHONY: fuzz-afl
fuzz-afl: $(FUZZ_HARNESS_BIN)
	@echo "## Starting afl-fuzz with harness $(FUZZ_HARNESS_BIN)..."
	@echo "## Input directory: $(IN_DIR)"
	@echo "## Output directory: $(OUT_DIR)"
	@echo "## Make sure you have an initial seed corpus in $(IN_DIR)!"
	@echo "## To stop fuzzing, press Ctrl+C."
	afl-fuzz -i $(IN_DIR) -o $(OUT_DIR) -- ./$(FUZZ_HARNESS_BIN)

# Rule to compile the fuzzing harness
$(FUZZ_HARNESS_BIN): $(FUZZ_HARNESS_SRC)
	@echo "## Compiling fuzzing harness $< with AFL instrumentation and gcov flags..."
	$(AFL_CC) $(CFLAGS) $(GCOV_CFLAGS) $< -o $@ $(LDFLAGS)

# Target to clean up compiled executables
.PHONY: clean
clean:
	@echo "## Cleaning up..."
	@rm -f $(TEST_BINS)
	@rm -f $(FUZZ_HARNESS_BIN) # Remove the afl-fuzz harness executable
	@rm -f *.gcda *.gcno tests/*.gcda tests/*.gcno fuzz/*.gcda fuzz/*.gcno coverage.info
	@rm -rf cov/ coverage_report/ # Remove coverage report directories
	@rm -f *.exe tests/*.exe fuzz/*.exe
