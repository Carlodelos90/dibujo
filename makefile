# Compiler and flags
CXX = g++
CXXFLAGS ?= -std=c++17 -I/opt/homebrew/opt/sfml@2/include
LDFLAGS ?= -L/opt/homebrew/opt/sfml@2/lib -lsfml-graphics -lsfml-window -lsfml-system

# Target executable and source file
TARGET = dibujo_v0.11
SRC = dibujo.cpp LogoManager.cpp

# Default target to build the executable
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Clean target to remove the compiled executable
clean:
	rm -f $(TARGET)
