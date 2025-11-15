# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -I../../imgui/ -I../../imgui/backends/ -pthread
LDFLAGS = -lGL -lglfw -pthread

# Profiling flags (added when building with 'make profile')
PROFILE_FLAGS = -pg -g -O2

# Target executable
TARGET = gui_app

# Your source files (modular!)
APP_SOURCES = main.cpp \
              display.cpp \
              rendering.cpp \
              udp_receiver.cpp

# ImGui source files
IMGUI_SOURCES = ../../imgui/imgui.cpp \
                ../../imgui/imgui_draw.cpp \
                ../../imgui/imgui_widgets.cpp \
                ../../imgui/imgui_tables.cpp \
                ../../imgui/backends/imgui_impl_glfw.cpp \
                ../../imgui/backends/imgui_impl_opengl3.cpp

# All sources
SOURCES = $(APP_SOURCES) $(IMGUI_SOURCES)

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Profile output directory
PROFILE_DIR = profile_data

# Default target
all: $(TARGET)

# Link object files
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f ../../imgui/*.o
	rm -f ../../imgui/backends/*.o

# Run the program
run: $(TARGET)
	./$(TARGET)

# Complete rebuild (fixes ImGui version issues)
rebuild: clean all

# ============================================================================
# PROFILING TARGETS
# ============================================================================

# Build with profiling enabled
profile-build: clean
	@echo "=========================================="
	@echo "Building with profiling enabled (-pg)"
	@echo "=========================================="
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(PROFILE_FLAGS)" LDFLAGS="$(LDFLAGS) $(PROFILE_FLAGS)"

# Run profiling session
profile-run: profile-build
	@echo ""
	@echo "=========================================="
	@echo "Starting profiling session..."
	@echo "=========================================="
	@echo "USE THE GUI FOR 20-30 SECONDS, THEN CLOSE IT"
	@echo "Try different scenarios and control modes"
	@echo ""
	@mkdir -p $(PROFILE_DIR)
	@cd $(PROFILE_DIR) && ../$(TARGET)
	@if [ -f $(PROFILE_DIR)/gmon.out ]; then \
		echo ""; \
		echo "Profile data generated successfully!"; \
	else \
		echo ""; \
		echo "ERROR: No profile data generated"; \
		exit 1; \
	fi

# Analyze profiling data
profile-analyze:
	@echo ""
	@echo "=========================================="
	@echo "Analyzing profile data..."
	@echo "=========================================="
	@if [ ! -f $(PROFILE_DIR)/gmon.out ]; then \
		echo "ERROR: No profile data found. Run 'make profile-run' first"; \
		exit 1; \
	fi
	@echo ""
	@echo "TOP FUNCTIONS BY CPU TIME:"
	@echo "---------------------------------------------"
	@gprof $(TARGET) $(PROFILE_DIR)/gmon.out -b -p | head -30
	@echo ""
	@echo "Generating detailed reports..."
	@gprof $(TARGET) $(PROFILE_DIR)/gmon.out > profile_report.txt
	@gprof $(TARGET) $(PROFILE_DIR)/gmon.out -q > profile_callgraph.txt
	@echo ""
	@echo "Reports saved:"
	@echo "  - profile_report.txt      (flat profile)"
	@echo "  - profile_callgraph.txt   (call graph)"
	@echo ""
	@echo "Look for high % time in:"
	@echo "  - drawAttitudeGauge"
	@echo "  - drawRateIndicator"
	@echo "  - sin/cos functions"
	@echo "  - ImGui::Render"

# Complete profiling workflow
profile: profile-run profile-analyze
	@echo ""
	@echo "=========================================="
	@echo "Profiling complete!"
	@echo "=========================================="
	@echo "Read profile_report.txt for detailed results"

# Clean profiling data
profile-clean:
	@echo "Cleaning profile data..."
	@rm -rf $(PROFILE_DIR)
	@rm -f profile_report.txt profile_callgraph.txt gmon.out
	@echo "Profile data cleaned"

# Show profiling help
profile-help:
	@echo "Profiling Targets:"
	@echo "  make profile         - Run complete profiling (build + run + analyze)"
	@echo "  make profile-build   - Build with profiling enabled"
	@echo "  make profile-run     - Build and run profiling session"
	@echo "  make profile-analyze - Analyze existing profile data"
	@echo "  make profile-clean   - Clean profiling data"
	@echo ""
	@echo "Quick start:"
	@echo "  1. make profile"
	@echo "  2. Use the GUI for 20-30 seconds"
	@echo "  3. Close the window"
	@echo "  4. Read profile_report.txt"

# Phony targets
.PHONY: all clean run rebuild profile-build profile-run profile-analyze profile profile-clean profile-help
