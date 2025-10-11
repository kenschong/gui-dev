# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -I ../../imgui/ -I ../../imgui/backends/
LDFLAGS = -lGL -lglfw

# Target executable
TARGET = gui_app

# Source files
SOURCES = main.cpp \
          ../../imgui/imgui.cpp \
          ../../imgui/imgui_draw.cpp \
          ../../imgui/imgui_widgets.cpp \
          ../../imgui/imgui_tables.cpp \
          ../../imgui/backends/imgui_impl_glfw.cpp \
          ../../imgui/backends/imgui_impl_opengl3.cpp

# Object files (replace .cpp with .o)
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Rebuild everything from scratch
rebuild: clean all

# Phony targets (not actual files)
.PHONY: all clean run rebuild