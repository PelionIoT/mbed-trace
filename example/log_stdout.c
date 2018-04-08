#include <stdio.h>

#define TRACE_GROUP "stdo"
#include "mbed-trace/mbed_trace.h"

int main(int argc, char* argv[]) {
    mbed_trace_init();
    tr_debug("test debug: %s: %d", "value", 1);
    tr_info("test info");
    tr_info("test info");
    tr_warn("test warn");
    return 0;
}
