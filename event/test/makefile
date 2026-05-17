
(CXX := g++)
(CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pthread)

# Set to 1 to build against real GlobalData implementation
USE_GLOBALDATA ?= 0

TARGET := event_test
SOURCES := event_test.cpp ../perf_event_cache.cpp

ifeq ($(USE_GLOBALDATA),1)
    EXTRA := ../../common/global_data.cpp ../../common/file.cpp
    CXXFLAGS += -DUSE_GLOBALDATA=1
else
    EXTRA :=
endif

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCES) $(EXTRA)
	$(CXX) $(CXXFLAGS) -o $@ $(SOURCES) $(EXTRA)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

