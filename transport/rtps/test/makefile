FASTDDS_ROOT := /home/ysy/Fast-DDS/Fast-DDS
FASTCDR_ROOT := /home/ysy/Fast-DDS/Fast-CDR

CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -I$(FASTDDS_ROOT)/src/cpp -I$(FASTDDS_ROOT)/include -I../../..
LDFLAGS := -L$(FASTDDS_ROOT)/build/src/cpp -L$(FASTCDR_ROOT)/build/src/cpp
LDLIBS := -lfastdds -lfastcdr -luuid
RPATH := -Wl,-rpath=$(FASTDDS_ROOT)/build/src/cpp -Wl,-rpath=$(FASTCDR_ROOT)/build/src/cpp

TESTS := attributes_filler_test rea_listener_test participant_test

.PHONY: all run run-all clean

all: $(TESTS)

attributes_filler_test: attributes_filler_test.cpp ../attributes_filler.cpp ../../qos/qos_profile_conf.cpp ../../../serialize/data_stream.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS) $(RPATH)


rea_listener_test: rea_listener_test.cpp ../rea_listener.cpp ../../message/message_info.cpp ../../common/identity.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS) $(RPATH)

participant_test: participant_test.cpp ../participant.cpp ../../../logger/logger.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS) $(RPATH)

run: attributes_filler_test
	./attributes_filler_test

run-all: all
	./attributes_filler_test
	./rea_listener_test
	./participant_test

clean:
	rm -f $(TESTS)
