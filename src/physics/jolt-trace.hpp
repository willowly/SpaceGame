#include <Jolt/Jolt.h>
#include <iostream>
#include <cstdarg>

namespace Physics {

    static void TraceImpl(const char *inFMT, ...)
    {
        // Format the message
        va_list list;
        va_start(list, inFMT);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), inFMT, list);
        va_end(list);

        // Print to the TTY
        std::cout << buffer << std::endl;
    }

        // Callback for asserts, connect this to your own assert handler if you have one
    static bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, unsigned int inLine)
    {
        // Print to the TTY
        std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr? inMessage : "") << std::endl;

        // Breakpoint
        return true;
    };

}
