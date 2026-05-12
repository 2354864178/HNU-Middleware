#include <iostream>
using namespace std;

#include "logger.h"
using namespace hnu::Middleware::logger;

int main() {
    Logger::instance()->open("test.log");
    Logger::instance()->set_level(Logger::INFO);

    debug("This is a debug message");
    info("This is an info message");
    warn("This is a warn message");
    error("This is an error message");
    fatal("This is a fatal message");
    return 0;
}
