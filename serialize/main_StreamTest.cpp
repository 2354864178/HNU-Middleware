#include <iostream>

using namespace std;
#include "data_stream.h"
using namespace hnu::Middleware::serialize;

int main() {
    DataStream ds;
    string a = "aaa";
    int c = 100;
    ds << a << c;
    ds.show();
    std::cout << std::endl;
    string b;
    int d;
    ds >> b >> d;
    std::cout << "b:" << b << endl;
    std::cout << "d:" << d << endl;

    return 0;
}