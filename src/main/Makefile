# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -I../../imgui/ -I../../imgui/backends/ -pthread
LDFLAGS = -lGL -lglfw -pthread

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

# Phony targets
.PHONY: all clean run rebuild