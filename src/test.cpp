#include <stdio.h>

#define API __attribute__((visibility("default")))

extern "C" {
 API void Run() {
    printf("HELLOOO");
 }
}
