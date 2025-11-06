#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void input_file(){
    FILE *member;
    char mem[100];
    printf("ระบุชื่อไฟล์ member ที่ต้องการใช้งาน : ");
    scanf("%s",mem);
    member = fopen(mem,"r");
    if(member == NULL){
        printf("fail open");
        return;
    }
}