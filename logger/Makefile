CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra

TARGET := log_test
SOURCES := log_test.cpp logger.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $(SOURCES)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
