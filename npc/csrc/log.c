#include "common.h"
#include "utils.h"
#include "debug.h"

FILE *log_fp = NULL;

void init_log(const char *log_file){
    assert(log_file);
    // printf("%s\n", "w");
    log_fp = fopen(log_file, "w");
    assert(log_fp);
    Log("Open log file " ANSI_FMT("%s", ANSI_FG_YELLOW) " succces\n",log_file);
}

bool log_enable(){
    return true;
}