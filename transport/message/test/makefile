CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -I../../..

TARGET := message_info_test
SOURCES := message_info_test.cpp ../message_info.cpp ../../common/identity.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $(SOURCES) -luuid

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
