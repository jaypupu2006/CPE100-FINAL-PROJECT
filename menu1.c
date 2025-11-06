#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void input_file(){
    FILE *member;
    char mem[100];
    member = fopen("memberfiles.txt","r");
    if(member == NULL){
        printf("fail open");
        return;
    }
}