CXX ?= c++
AR ?= ar
CPPFLAGS += -Idevelopment/include
CXXFLAGS ?= -O2 -std=c++98 -Wall -Wextra -pedantic
# Set to 1 to compile trace APIs. Separate default directories prevent stale ABIs.
COMPRESSION_TRACE ?= 0
ifeq ($(filter $(COMPRESSION_TRACE),0 1),)
$(error COMPRESSION_TRACE must be 0 or 1)
endif
ifeq ($(COMPRESSION_TRACE),1)
TRACE_CPPFLAGS = -DCOMPRESSION_TRACE
BUILD_DIR ?= build-make-trace
else
BUILD_DIR ?= build-make
endif
TRACE_CONFIG = $(BUILD_DIR)/compression-trace.config
SOURCES = development/src/GXByteBuffer.cpp development/src/GXBitArray.cpp development/src/GXV44Options.cpp development/src/GXV44Encoder.cpp development/src/GXV44Decoder.cpp
OBJECTS = $(patsubst development/src/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
.PHONY: all test clean FORCE
all: $(BUILD_DIR)/libgurux_compression.a
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
# Keep explicit BUILD_DIR overrides safe when the trace option changes.
FORCE:
$(TRACE_CONFIG): FORCE | $(BUILD_DIR)
	@if test ! -f "$@" || test "$$(cat "$@")" != "$(COMPRESSION_TRACE)"; then printf '%s\n' '$(COMPRESSION_TRACE)' > "$@"; fi
$(BUILD_DIR)/%.o: development/src/%.cpp $(TRACE_CONFIG) | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(TRACE_CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@
$(BUILD_DIR)/libgurux_compression.a: $(OBJECTS)
	$(AR) rcs $@ $^
$(BUILD_DIR)/compression_tests: tests/tests.cpp tests/trace_configuration.cpp development/src/GXBitArray.h $(BUILD_DIR)/libgurux_compression.a
	$(CXX) $(CPPFLAGS) $(TRACE_CPPFLAGS) $(CXXFLAGS) -Idevelopment/src -DFIXTURE_DIR='"$(CURDIR)/tests/fixtures"' tests/tests.cpp tests/trace_configuration.cpp $(BUILD_DIR)/libgurux_compression.a -o $@
$(BUILD_DIR)/compression_interop: tests/interop.cpp $(BUILD_DIR)/libgurux_compression.a
	$(CXX) $(CPPFLAGS) $(TRACE_CPPFLAGS) $(CXXFLAGS) $< $(BUILD_DIR)/libgurux_compression.a -o $@
test: $(BUILD_DIR)/compression_tests
	$(BUILD_DIR)/compression_tests
clean:
	$(RM) $(TRACE_CONFIG) $(OBJECTS) $(OBJECTS:.o=.d) $(BUILD_DIR)/libgurux_compression.a $(BUILD_DIR)/compression_tests $(BUILD_DIR)/compression_interop
-include $(OBJECTS:.o=.d)
