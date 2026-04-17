CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra

TARGET := stream_test
SOURCES := main_StreamTest.cpp data_stream.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $(SOURCES)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
