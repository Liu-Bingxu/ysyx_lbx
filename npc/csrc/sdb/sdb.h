#ifndef __SDB_H__
#define __SDB_H__

#include "common.h"
#include "debug.h"
#include "pmem.h"
#include "utils.h"

typedef struct token {
  int type;
  char str[32];
} Token;

typedef struct watchpoint {
    int NO;
    struct watchpoint *next;
    struct watchpoint *prev;

  /* TODO: Add more members if necessary */

    int Hitnum;
    word_t old_value;
    int wp_nr_token;
    Token wp_tokens[100];

} WP;

typedef union{
    word_t val;
    struct{
        char x1;
        char x2;
        char x3;
        char x4;
    };
} printf_char;

void
init_sdb();

bool check_watchpoint();

long my_atoi(const char *args);

void sdb_set_batch_mode();
word_t expr(char *e, bool *success, bool mode, int *nr_tekones, Token *tekenes);

WP *new_wp();
void free_wp(int NO);
void watchpoint_display();

#endif
