# tests/rlvk/Makefile
# Headless Vulkan unit tests for the rlvk backend. Driven by SwiftShader in
# CI (set VK_ICD_FILENAMES to its loader JSON); also runs against the system
# Vulkan loader on dev machines with a real GPU.

CC      ?= cc
CFLAGS  ?= -O0 -g -std=c99 -Wall -Wextra -I../../src
LDLIBS  := -lvulkan -lm -ldl -lpthread

# On Apple, the Vulkan loader and frameworks need rpath wiring like the rest
# of raylib's Vulkan build. Pick those up from VULKAN_SDK if present.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    ifneq ($(VULKAN_SDK),)
        CFLAGS += -I$(VULKAN_SDK)/include
        LDFLAGS += -L$(VULKAN_SDK)/lib -Wl,-rpath,$(VULKAN_SDK)/lib
    endif
    LDLIBS := -lvulkan -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo
endif

.PHONY: all clean run

all: test_rlvk

test_rlvk: test_rlvk.c ../../src/rlvk.h ../../src/rlvk_shader.h fixtures/tiny.rlvk.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

# Regenerate the tiny fixture from source if rlvk-shdc is available.
fixtures/tiny.rlvk.h: fixtures/tiny.vert fixtures/tiny.frag
	@mkdir -p /tmp/_rlvk_tests
	glslc -o /tmp/_rlvk_tests/tiny.vs.spv fixtures/tiny.vert
	glslc -o /tmp/_rlvk_tests/tiny.fs.spv fixtures/tiny.frag
	../../src/tools/rlvk-shdc \
		-v /tmp/_rlvk_tests/tiny.vs.spv \
		-f /tmp/_rlvk_tests/tiny.fs.spv \
		-n tiny -o $@

run: test_rlvk
	./test_rlvk

clean:
	rm -f test_rlvk
