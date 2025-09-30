# Windows Makefile for RegiStart
CXX = g++
CXXFLAGS = -O2 -Wall -Wextra
LDFLAGS = -ladvapi32 -mconsole
TARGET = RegiStart.exe
SOURCES = main.cpp
OBJS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	del /Q $(OBJS) $(TARGET) 2>nul || exit 0

# Clean and rebuild
rebuild: clean all

# Install (copy to current directory - placeholder)
install: $(TARGET)
	@echo Build complete: $(TARGET)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: rebuild

# Release build (default)
release: CXXFLAGS += -O3 -s
release: rebuild

# Show help
help:
	@echo Available targets:
	@echo   all       - Build the executable (default)
	@echo   clean     - Remove all build files
	@echo   rebuild   - Clean and rebuild
	@echo   debug     - Build with debug symbols
	@echo   release   - Build optimized for release
	@echo   install   - Build and show completion message
	@echo   help      - Show this help message

.PHONY: all clean rebuild debug release install help

