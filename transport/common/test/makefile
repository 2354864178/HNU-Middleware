CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -I../../..

TARGET := identity_test
SOURCES := test_endpoint.cpp ../identity.cpp ../endpoint.cpp ../../../serialize/data_stream.cpp ../../../common/global_data.cpp ../../../common/file.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $(SOURCES) -luuid

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)