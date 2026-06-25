CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

TARGET = main
SOURCE = main.cpp

.PHONY: all build run clean

all: build

build: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

run: build
	./$(TARGET)

clean:
	rm -f $(TARGET)
