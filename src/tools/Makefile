# src/tools/Makefile
# Build the rlvk-shdc code generator. C99, no deps, host compiler.

CC      ?= cc
CFLAGS  ?= -O2 -std=c99 -Wall -Wextra -Wpedantic
TOOL    = rlvk-shdc

.PHONY: all clean test update-golden

all: $(TOOL)

$(TOOL): rlvk-shdc.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TOOL)
	rm -rf tests/out

test: $(TOOL)
	bash tests/run_tests.sh

update-golden: $(TOOL)
	bash tests/update_golden.sh
