#include <iostream>
#include "../identity.h"

using namespace hnu::Middleware::transport;

int main() {
    Identity id; // generate
    std::cout << "ID as string: " << id.ToString() << std::endl;
    std::cout << "ID length: " << id.Length() << std::endl;
    std::cout << "Hash value: " << id.HashValue() << std::endl;
    return 0;
}
