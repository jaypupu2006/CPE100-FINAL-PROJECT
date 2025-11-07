#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "menu1.h"
#include "menu2.h"
#include "menu3.h"

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

        input_file();

        if(menu == 1){
            menu1_choose();
        }else if(menu == 2){
            menu2_choose();
        }else if(menu == 3){
            menu3_choose();
        }else if(menu == 0){
            break;
        }else{
            printf("กรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง");
        }
    }
    printf("End of program Goodbye :))");
}