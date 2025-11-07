#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

void input_file(){
    FILE *member;
    member = fopen("member.txt","r");
    if(member == NULL){
        printf("fail open");
        return;
    }
    fclose(member);
}

void register_member(){
    FILE *fp;
    char fullname[100], nickname[50];
    int gender;
    int last_id = 0;
    char line[256];
    
    fp = fopen("member.txt", "r");
    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp)) {
            if (isdigit((unsigned char)line[0])) {
                sscanf(line, "%d|", &last_id);
            }
        }
        fclose(fp);
    }

    printf("\n=== ลงทะเบียนสมาชิกก๊วน ===\n");

    /* เคลียร์ newline ที่ค้างมาจาก scanf เมนู */
    { int c; while ((c = getchar()) != '\n' && c != EOF) {} }

    printf("กรอกชื่อ-นามสกุล: ");
    fgets(fullname, sizeof(fullname), stdin);
    fullname[strcspn(fullname, "\n")] = 0;

    printf("กรอกชื่อเล่น: ");
    fgets(nickname, sizeof(nickname), stdin);
    nickname[strcspn(nickname, "\n")] = 0;

    printf("เพศ (1. ชาย, 2. หญิง): ");
    scanf("%d", &gender);
    
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char created_at[20];
    sprintf(created_at, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    
    fp = fopen("member.txt", "a");
    if (fp == NULL) {
        printf("เกิดข้อผิดพลาดในการเปิดไฟล์เพื่อบันทึกข้อมูล\n");
        return;
    }

    fprintf(fp, "%d|%s|%s|%d|%s\n", last_id + 1, fullname, nickname, gender, created_at);
    fclose(fp);

    printf("\n ลงทะเบียนเสร็จสิ้น! รหัสสมาชิกของคุณคือ %d\n", last_id + 1);
}

void search_member() {
    FILE *fp;
    char line[256];
    int choice;

    printf("\n=== ค้นหารายชื่อในระบบ ===\n");
    printf("1. ค้นหาด้วยเลขสมาชิก\n");
    printf("2. ค้นหาด้วยชื่อเล่น\n");
    printf("3. ค้นหาด้วยชื่อ-นามสกุล\n");
    printf("0. ย้อนกลับ\n");
    printf("เลือกวิธีค้นหา: ");
    scanf("%d", &choice);

    if (choice == 0) return;

    int target_id = 0;
    char keyword[128];

    if (choice == 1) {
        printf("กรอกเลขสมาชิก: ");
        scanf("%d", &target_id);
    } else {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
        printf("กรอกคำค้นหา: ");
        fgets(keyword, sizeof(keyword), stdin);
        keyword[strcspn(keyword, "\n")] = 0;
    }

    fp = fopen("member.txt", "r");
    if (fp == NULL) {
        printf("ไม่สามารถเปิดไฟล์ได้\n");
        return;
    }

    printf("\n=== ผลการค้นหา ===\n");
    int id, gender;
    char fullname[100], nickname[50], created_at[20];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (!(line[0] >= '0' && line[0] <= '9')) {
            continue;
        }
        if (sscanf(line, "%d|%99[^|]|%49[^|]|%d|%19s", &id, fullname, nickname, &gender, created_at) == 5) {
            int match = 0;
            if (choice == 1 && id == target_id) {
                match = 1;
            } else if (choice == 2 && strstr(nickname, keyword)) {
                match = 1;
            } else if (choice == 3 && strstr(fullname, keyword)) {
                match = 1;
            }

            if (match) {
                printf("รหัส: %d | %s (%s) | เพศ: %s | วันที่สมัคร: %s\n",
                       id, fullname, nickname, (gender == 1 ? "ชาย" : "หญิง"), created_at);
                found = 1;
            }
        }
    }

    fclose(fp);
    if (!found) {
        printf("ไม่พบข้อมูลที่ค้นหา\n");
    }
}

void show_all_members() {
    FILE *fp = fopen("member.txt", "r");
    if (fp == NULL) {
        printf("ไม่สามารถเปิดไฟล์ได้\n");
        return;
    }

    char line[256];
    int id, gender, count = 0;
    char fullname[100], nickname[50], created_at[20];

    printf("\n=== รายชื่อสมาชิกทั้งหมด ===\n");
    printf("%-5s | %-24s | %-12s | %-6s | %-10s\n", "ID", "ชื่อ-นามสกุล", "ชื่อเล่น", "เพศ", "สมัครเมื่อ");
    printf("--------------------------------------------------------------------------\n");

    while (fgets(line, sizeof(line), fp)) {
        if (!(line[0] >= '0' && line[0] <= '9')) {
            continue;
        }
        if (sscanf(line, "%d|%99[^|]|%49[^|]|%d|%19s", &id, fullname, nickname, &gender, created_at) == 5) {
            printf("%-5d | %-24s | %-12s | %-6s | %-10s\n",
                   id, fullname, nickname, (gender == 1 ? "ชาย" : "หญิง"), created_at);
            count++;
        }
    }

    fclose(fp);

    if (count == 0) {
        printf("ยังไม่มีข้อมูลสมาชิก\n");
    } else {
        printf("--------------------------------------------------------------------------\n");
        printf("รวมสมาชิกทั้งหมด: %d คน\n", count);
    }
}

void menu1_choose (){
    while(1){
    printf("1.ลงทะเบียนสมาชิกก๊วน\n");
    printf("2.ค้นหารายชื่อในระบบ\n");
    printf("3.แสดงรายชื่อสมาชิกทั้งหมด\n");
    printf("0.กลับสู่หน้าหลัก\n");
    int menu1;
    printf("ระบุเมนูที่ต้องการทำงาน :");
    scanf("%d",&menu1);
        if(menu1 == 1){
            register_member();
            break;
        }else if(menu1 == 2){
            search_member();
            break;
        }else if(menu1 == 3){
            show_all_members();
            break;
        }else if(menu1 == 0){
            break;
        }else{
            printf("ไม่ตรงกับในเมนูโปรดลองอีกครั้ง");
        }
    }
}
