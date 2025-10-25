#include <iostream>

#define API __attribute__((visibility("default")))

extern "C" {
 API void Run() {
    std::cout << "hello from c++" << std::endl;
 }
}
