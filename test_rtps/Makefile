FASTDDS_ROOT := /home/ysy/Fast-DDS/Fast-DDS
FASTCDR_ROOT := /home/ysy/Fast-DDS/Fast-CDR

CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
INCLUDES := -I$(FASTDDS_ROOT)/include -I/home/ysy/Fast-DDS
LDFLAGS := -L$(FASTDDS_ROOT)/build/src/cpp -L$(FASTCDR_ROOT)/build/src/cpp
LDLIBS := -lfastdds -lfastcdr
RPATH := -Wl,-rpath=$(FASTDDS_ROOT)/build/src/cpp -Wl,-rpath=$(FASTCDR_ROOT)/build/src/cpp

TARGET := test_rtps
SOURCES := main_RTPSTest.cpp TestReaderPersistent.cpp TestWriterPersistent.cpp

.PHONY: all clean
.PHONY: run-reader run-writer

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(SOURCES) $(LDFLAGS) $(LDLIBS) $(RPATH)

run-reader: $(TARGET)
	./$(TARGET) reader

run-writer: $(TARGET)
	./$(TARGET) writer

clean:
	rm -f $(TARGET)