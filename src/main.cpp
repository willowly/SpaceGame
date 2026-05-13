#include <chrono>
#include <stdexcept>
#include <thread>
#define TRACY_ENABLE 1
#include "tracy/Tracy.hpp"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "networking/net-test-app.hpp"

//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

using std::string;


int main() {
    NetTestApp app;

    app.run();
}
