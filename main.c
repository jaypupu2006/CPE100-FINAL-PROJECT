#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>


#include "Library/menu1.h"
#include "Library/menu2.h"   // <-- ensure this is included
#include "Library/menu3.h"
#include "Library/delay.h"

/*
 * main: โปรแกรมหลักของระบบ
 * - แสดงเมนูหลักให้ผู้ใช้เลือกทำงาน (สมาชิก, เปิดก๊วน, สรุปข้อมูล)
 * - เรียก `input_file()` เพื่อตรวจ/สร้างไฟล์ที่จำเป็น
 * - เรียกเมนูย่อยตามตัวเลือกและวนลูปจนกว่าจะกรอก 0 เพื่อออก
 */

int main (){

    SetConsoleOutputCP(CP_UTF8); // ตั้งค่าหน้าจอให้ใช้ UTF-8
    SetConsoleCP(CP_UTF8);       // ตั้งค่าการรับ input ให้เป็น UTF-8 ด้วย เพื่อการรับ ouput ภาษาไทย

    int menu;
    while(1){

        system("cls"); // จาก stdlib.h -- ให้โปรแกรมทำเหมือนการพิมพ์คำว่า cls ใน Command Prompt => เคลียร์จอ

        printf("Badminton Group Payment System\n");
        printf("กรอกหมายเลขเพื่อทำระบบต่อไปนี้\n");
        printf("1. ระบบสมาชิก\n");
        printf("2. เปิดก๊วน\n");
        printf("3. สรุปข้อมูล\n");
        printf("0. เพื่อหยุดโปรแกรม\n");
        printf("กรอกหมายเลขตามเมนูที่ต้องการ : ");

        if(scanf("%d", &menu) != 1) { // ถ้าผู้ใช้ไม่ได้กรอกตัวเลข
            int c;
            while((c = getchar()) != '\n' && c != EOF) {}  //ล้าง buffer ทั้งหมด จนถึง \n หรือ EOF
            printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
            delay(2);
            continue;
        }

        if(menu == 1) {
            menu1_choose();
        }else if(menu == 2) {
            menu2_choose();
        }else if(menu == 3) {
            menu3_choose();
        }else if(menu == 0) {
            break;
        }else {
            printf("\nไม่มีเมนูเลข %d โปรดลองอีกครั้ง\n", menu);
            delay(2);
        }
    }
    printf("ปิดโปรแกรม บ๊ายบายยย :)\n");
    return 0;
}