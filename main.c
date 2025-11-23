#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>


#include "menu1.h"
#include "menu2.h"
#include "menu3.h"
#include "delay.h"

int main (){
    SetConsoleOutputCP(65001);

    int menu;
    while(1){
        system("cls");
        printf("Badminton Group Payment System\n");
        printf("กรอกหมายเลขเพื่อทำระบบต่อไปนี้\n");
        printf("1.ระบบสมาชิก\n");
        printf("2.เปิดก๊วน\n");
        printf("3.สรุปข้อมูล\n");
        printf("0 เพื่อหยุดโปรแกรม\n");
        printf("กรอกหมายเลขตามเมนูที่ต้องการ : ");

        if(scanf("%d", &menu) != 1){
            int c;
            while((c = getchar()) != '\n' && c != EOF) {}
            printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
            delay(2);
            continue;
        }

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
            printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
            delay(2);
        }
    }
    printf("End of program Goodbye :))\n");
    return 0;
}