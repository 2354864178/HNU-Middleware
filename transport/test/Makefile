CXX := g++
CXXFLAGS := -std=c++17 -pthread -I.. -I../.. -I../shm/core -I../shm/segment -I../shm/notifier -I../common -I../receiver -I../transmitter -I../dispatcher
TRANSPORT_TEST_CXXFLAGS := $(CXXFLAGS) -DHNU_TRANSPORT_UNIT_TEST_LIGHT

DISPATCHER_SOURCES := dispatcher_unit_test.cpp \
	../dispatcher/dispatcher.cpp \
	../message/message_info.cpp \
	../common/identity.cpp \
	../../serialize/data_stream.cpp \
	../../logger/logger.cpp \
	../../common/global_data.cpp \
	../../common/file.cpp

TRANSPORT_SOURCES := transport_unit_test.cpp \
	../transport.cpp \
	transport_stubs.cpp \
	../dispatcher/dispatcher.cpp \
	../message/message_info.cpp \
	../common/identity.cpp \
	../../serialize/data_stream.cpp \
	../../logger/logger.cpp \
	../../common/global_data.cpp \
	../../common/file.cpp

SHM_TXRX_SOURCES := shm_txrx_test.cpp \
	../common/endpoint.cpp \
	../common/identity.cpp \
	../message/message_info.cpp \
	../dispatcher/dispatcher.cpp \
	../dispatcher/shm_dispatcher.cpp \
	../shm/core/block.cpp \
	../shm/core/readable_info.cpp \
	../shm/core/segment.cpp \
	../shm/core/shm_conf.cpp \
	../shm/core/state.cpp \
	../shm/notifier/condition_notifier.cpp \
	../shm/notifier/multicast_notifier.cpp \
	../shm/notifier/notifier_factory.cpp \
	../shm/segment/posix_segment.cpp \
	../shm/segment/segment_factory.cpp \
	../shm/segment/xsi_segment.cpp \
	../../event/perf_event_cache.cpp \
	../../serialize/data_stream.cpp \
	../../logger/logger.cpp \
	../../common/global_data.cpp \
	../../common/file.cpp

DISPATCHER_TARGET := dispatcher_unit_test
TRANSPORT_TARGET := transport_unit_test
SHM_TXRX_TARGET := shm_txrx_test

.PHONY: all clean run-shm-reader run-shm-writer

all: $(DISPATCHER_TARGET) $(TRANSPORT_TARGET) $(SHM_TXRX_TARGET)

$(DISPATCHER_TARGET): $(DISPATCHER_SOURCES)
	$(CXX) $(CXXFLAGS) -o $(DISPATCHER_TARGET) $(DISPATCHER_SOURCES) -luuid

$(TRANSPORT_TARGET): $(TRANSPORT_SOURCES)
	$(CXX) $(TRANSPORT_TEST_CXXFLAGS) -o $(TRANSPORT_TARGET) $(TRANSPORT_SOURCES) -luuid -lfastdds

$(SHM_TXRX_TARGET): $(SHM_TXRX_SOURCES)
	$(CXX) $(CXXFLAGS) -o $(SHM_TXRX_TARGET) $(SHM_TXRX_SOURCES) -luuid

run-shm-reader: $(SHM_TXRX_TARGET)
	./$(SHM_TXRX_TARGET) reader

run-shm-writer: $(SHM_TXRX_TARGET)
	./$(SHM_TXRX_TARGET) writer

clean:
	rm -f $(DISPATCHER_TARGET) $(TRANSPORT_TARGET) $(SHM_TXRX_TARGET)
