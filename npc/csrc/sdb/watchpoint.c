/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "sdb.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].Hitnum = 0;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].prev = (i == 0         ? NULL : &wp_pool[i - 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(){
    WP *watchp = NULL;
    WP *old_WP = free_;
    int min = 100;
    if (free_ != NULL){

        while(old_WP!=NULL){
            if(old_WP->NO<min){
                watchp = old_WP;
                min = old_WP->NO;
            }
            old_WP = old_WP->next;
        }
        if((watchp->next!=NULL)&&(watchp->prev!=NULL)){
            watchp->prev->next = watchp->next;
            watchp->next->prev = watchp->prev;
        }
        else if(watchp->next!=NULL){
            free_ = watchp->next;
            watchp->next->prev = NULL;
        }
        else if(watchp->prev!=NULL){
            watchp->prev->next = NULL;
        }
        else{
            free_ = NULL;
        }
        watchp->prev = NULL;
        watchp->next = NULL;

        old_WP = head;
        if(head==NULL){
            head = watchp;
            return watchp;
        }
        while(old_WP!=NULL){
            if(old_WP->NO < watchp->NO){
                if(old_WP->next==NULL){
                    old_WP->next = watchp;
                    watchp->prev = old_WP;
                    return watchp;
                }
                else{
                    old_WP = old_WP->next;
                }
            }
            else{
                watchp->prev = old_WP->prev;
                watchp->next = old_WP;
                if(old_WP->prev!=NULL){
                    old_WP->prev->next = watchp;
                }
                old_WP->prev = watchp;
                return watchp;
            }
        }
    }
    else{
        assert(0);
    }
    return NULL;
}

void free_wp(int NO){
    WP *wp=head;
    while (wp!=NULL){
        if(wp->NO==NO){
            break;
        }
        wp = wp->next;
    }
    if(wp==NULL){
        Log("you free a nonexistent watchpoint: %d", NO);
        return;
    }
    wp->Hitnum = 0;
    if (wp->next != NULL){
        wp->next->prev = wp->prev;
    }
    if(wp->prev!=NULL){
        wp->prev->next = wp->next;
    }
    if(head==wp){
        if(wp->next!=NULL){
            head = wp->next;
        }
        else{
            head = NULL;
        }
    }
    wp->prev = NULL;
    wp->next = free_;
    if(free_!=NULL){
        free_->prev = wp;
    }
    free_ = wp;
    printf("succue to free the %d watchpoint\n", NO);
    return;
}

void watchpoint_display(){
    if(head==NULL){
        printf("Now don't have watchpoint\n");
        return;
    }
    printf("No  Hitnum What\n");
    WP *now = head;
    while(now!=NULL){
        printf("%-3d %-6d ", now->NO, now->Hitnum);
        for (int i = 0; i < now->wp_nr_token;i++){
            printf("%s ", now->wp_tokens[i].str);
        }
        printf("\n");
        now = now->next;
    }
    return;
}


bool check_watchpoint(){
    bool check_bool = true;
    WP *now = head;

    while(now!=NULL){
        bool check_succue = true;
        word_t now_value = expr(NULL, &check_succue, true, &now->wp_nr_token, now->wp_tokens);
        if(check_succue==false)
            assert(0);
        else if (now_value != now->old_value){
            printf("watchpoint %-3d: ", now->NO);
            for (int i = 0; i < now->wp_nr_token;i++){
                printf("%s", now->wp_tokens[i].str);
            }
            printf("\n");
            printf("\n");
            printf("\n");
            now->Hitnum++;
            printf("Old value = %#x\n", now->old_value);
            printf("New value = %#x\n", now_value);
            now->old_value = now_value;
            check_bool = false;
        }
        now = now->next;
    }

    return check_bool;
}
