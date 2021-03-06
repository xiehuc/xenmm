/*=============================================================================
#     FileName: mono.c
#         Desc: mono memory benchmark
#               simplify the original mono version from zhangdi
#       Author: xiehuc
#        Email: xiehuc@gmail.com
=============================================================================*/
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//adjust this value to set max mb size of memory
//malloced
void show_help()
{
   printf("useage: mono low high\n"
        "example: mono 100M 500M\n");
}
int main(int argc,char** argv)
{
    ul low,high,target;
    if(argc!=3){show_help();return 0;}
    char* end;
    low = strtoul(argv[1],&end,10);
    low *= unit_expand(*end);
    high = strtoul(argv[2],&end,10);
    high *= unit_expand(*end);
    if(low>=high){
        fprintf(stderr,"low value must less than high value\n");
        return -1;
    }
    if(high>=MAX_MB*1024){
        fprintf(stderr,"high value must less than MAX_MB(%u)\n",MAX_MB);
        return -1;
    }

    printf("\n");

    visit_pages(low);
    sleep(RESP_TIME);

    for(target = low;target<high;target+=ONE_PAGE*5){
        printf(".");
        fflush(stdout);
        visit_pages(ONE_PAGE*5);
        sleep(1);
    }
    for(target = high;target>=low;target-=ONE_PAGE*5){
        printf(".");
        fflush(stdout);
        free_pages(ONE_PAGE*5);
        sleep(1);
    }

    free_all_pages();
    sleep(1);
    printf("\n");
    return 0;
}
