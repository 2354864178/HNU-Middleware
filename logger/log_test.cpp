#include <iostream>
using namespace std;

#include "logger.h"
using namespace hnu::Middleware::logger;

int main() {
    Logger::instance()->open("test.log");
    // Logger::instance()->log(Logger::DEBUG, __FILE__, __LINE__, "This is a debug message");
    debug("This is a debug message");
    info("This is an info message");
    warn("This is a warn message");
    error("This is an error message");
    fatal("This is a fatal message");
    return 0;
}
