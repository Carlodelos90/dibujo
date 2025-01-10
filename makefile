# Compiler and flags
CXX = g++
CXXFLAGS ?= -std=c++11 -I/opt/homebrew/opt/sfml@2/include
LDFLAGS ?= -L/opt/homebrew/opt/sfml@2/lib -lsfml-graphics -lsfml-window -lsfml-system

# Target executable and source file
TARGET = dibujov0.1
SRC = dibujo.cpp

# Default target to build the executable
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Clean target to remove the compiled executable
clean:
	rm -f $(TARGET)
