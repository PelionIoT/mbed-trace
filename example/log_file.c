#include <stdio.h>

#define TRACE_GROUP "file"
#include "mbed-trace/mbed_trace.h"

int main(int argc, char* argv[]) {
    mbed_trace_init();
    uint8_t cfg = TRACE_ACTIVE_LEVEL_ALL | TRACE_CARRIAGE_RETURN;
    mbed_trace_config_set(cfg);
    FILE *fh = fopen("./debug.log", "a");
    mbed_trace_set_pipe(fh);
    tr_debug("debugging to file");
    tr_info("info message to file");
    tr_warn("warning essage to file");
    fclose(fh);
    return 0;
}
