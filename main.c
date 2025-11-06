#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "menu1.h"

int main (){

    int menu;
    while(1){
        printf("Badminton Group Payment System\n");
        printf("กรอกหมายเลขเพื่อทำระบบต่อไปนี้\n");
        printf("1.ระบบสมาชิก\n");
        printf("2.เปิดก๊วน\n");
        printf("3.สรุปข้อมูล\n");
        printf("0 เพื่อหยุดโปรแกรม\n");
        printf("กรอกหมายเลขตามเมนูที่ต้องการ :");
        scanf("%d" , &menu);

        if(menu == 1){
            input_file();
            



        }else if(menu == 2){

        }else if(menu == 3){

        }else if(menu == 0){
            break;
        }else{
            printf("กรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง");
        }
    }
    printf("End of program Goodbye :))");
}