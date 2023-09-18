#include "sdb.h"

static bool Batch_mode = false;

void sdb_set_batch_mode(){
    Batch_mode = true;
    printf("Enter the Batch mode\n");
    return;
}
