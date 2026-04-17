#include <cstring>
#include <iostream>

#include <fastdds/rtps/RTPSDomain.hpp>

#include "TestReaderPersistent.h"
#include "TestWriterPersistent.h"

using namespace eprosima::fastdds::rtps;

int main(int argc, char** argv) {
  std::cout << "Starting RTPS test (no serialization)" << std::endl;

  int type = 0;

  // 解析命令行参数，确定是运行 writer 还是 reader
  if (argc > 1) { 
    if (std::strcmp(argv[1], "writer") == 0) {
      type = 1;
    } else if (std::strcmp(argv[1], "reader") == 0) {
      type = 2;
    } else {
      std::cout << "NEEDS writer OR reader as first argument" << std::endl;
      return 1;
    }
  } else {
    std::cout << "NEEDS writer OR reader ARGUMENT" << std::endl;
    std::cout << "./test_rtps writer" << std::endl;
    std::cout << "./test_rtps reader" << std::endl;
    return 1;
  }

  switch (type) {
    case 1: {
      TestWriterPersistent writer;
      if (writer.init() && writer.reg()) {
        writer.run(10); // 发送10条样本数据后退出
      }
      break;
    }
    case 2: {
      TestReaderPersistent reader;
      if (reader.init() && reader.reg()) {
        reader.run();   // 持续等待接收数据，默认10秒后退出
      }
      break;
    }
    default:
      return 1;
  }

  RTPSDomain::stopAll();
  std::cout << "EVERYTHING STOPPED FINE" << std::endl;
  return 0;
}
