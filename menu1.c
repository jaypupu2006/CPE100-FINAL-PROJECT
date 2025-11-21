#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define NAME_MAXLEN 128
#define NICK_MAXLEN 64
#define DATE_MAXLEN 16

/* ตรวจว่า member.txt เปิดได้ไหม ถ้าไม่ได้ให้สร้างไฟล์ใหม่ */
void input_file(){
    FILE *member = fopen("member.txt","r");
    if(member == NULL){
        member = fopen("member.txt","w");
        if(member == NULL){
            printf("ไม่สามารถสร้างไฟล์ member.txt ได้\n");
            return;
        }
    }
    fclose(member);
}

/* สร้างวันที่ปัจจุบันเป็น YYYY-MM-DD */
static void get_today(char buf[DATE_MAXLEN]){
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, DATE_MAXLEN, "%Y-%m-%d", tm_info);
}

/* ลงทะเบียนสมาชิกใหม่ */
void register_member(){
    FILE *fp;
    int last_id = 0;

    fp = fopen("member.txt", "r");
    if(fp != NULL){
        char line[512];
        while(fgets(line, sizeof(line), fp)){
            int id;
            if(sscanf(line, "%d|", &id) == 1){
                if(id > last_id) last_id = id;
            }
        }
        fclose(fp);
    }

    int new_id = last_id + 1;
    char fullname[NAME_MAXLEN];
    char nickname[NICK_MAXLEN];
    int gender;
    char created_at[DATE_MAXLEN];

    printf("\n=== ลงทะเบียนสมาชิกใหม่ ===\n");
    printf("ป้อนชื่อ-นามสกุล (เว้นวรรคคั่น): ");
    getchar(); // เคลียร์ \n ค้างจาก scanf ก่อนหน้า
    fgets(fullname, sizeof(fullname), stdin);
    fullname[strcspn(fullname, "\n")] = '\0';

    if(strlen(fullname) == 0){
        printf("ยกเลิกการลงทะเบียน (ไม่ได้กรอกชื่อ)\n");
        return;
    }

    printf("ป้อนชื่อเล่น: ");
    fgets(nickname, sizeof(nickname), stdin);
    nickname[strcspn(nickname, "\n")] = '\0';

    printf("เพศ (1=ชาย, 2=หญิง, 0=ยกเลิก): ");
    if(scanf("%d", &gender) != 1){
        while(getchar()!='\n' && !feof(stdin)){}
        printf("ข้อมูลไม่ถูกต้อง ยกเลิกการลงทะเบียน\n");
        return;
    }
    if(gender == 0){
        printf("ยกเลิกการลงทะเบียน\n");
        return;
    }

    get_today(created_at);

    fp = fopen("member.txt","a");
    if(fp == NULL){
        printf("ไม่สามารถเปิด member.txt เพื่อเขียนได้\n");
        return;
    }

    fprintf(fp, "%d|%s|%s|%d|%s\n", new_id, fullname, nickname, gender, created_at);
    fclose(fp);

    printf("ลงทะเบียนสำเร็จ! รหัสสมาชิกของคุณคือ: %d\n", new_id);
}

/* ค้นหาสมาชิก */
void search_member(){
    FILE *fp = fopen("member.txt","r");
    if(fp == NULL){
        printf("ไม่พบไฟล์ member.txt\n");
        return;
    }

    int mode;
    printf("\n=== ค้นหาสมาชิก ===\n");
    printf("1. ค้นหาด้วยรหัสสมาชิก\n");
    printf("2. ค้นหาด้วยชื่อเล่น\n");
    printf("3. ค้นหาด้วยชื่อ-นามสกุล\n");
    printf("0. ย้อนกลับ\n");
    printf("เลือก: ");
    if(scanf("%d", &mode) != 1){
        while(getchar()!='\n' && !feof(stdin)){}
        fclose(fp);
        return;
    }
    if(mode == 0){
        fclose(fp);
        return;
    }

    char key[NAME_MAXLEN];
    int search_id = 0;

    if(mode == 1){
        printf("ป้อนรหัสสมาชิก (0 = ย้อนกลับ): ");
        if(scanf("%d", &search_id) != 1){
            while(getchar()!='\n' && !feof(stdin)){}
            fclose(fp);
            return;
        }
        if(search_id == 0){
            fclose(fp);
            return;
        }
    }else{
        printf("ป้อนคำที่ต้องการค้นหา (0 = ย้อนกลับ): ");
        getchar();
        fgets(key, sizeof(key), stdin);
        key[strcspn(key, "\n")] = '\0';
        if(strcmp(key,"0") == 0){
            fclose(fp);
            return;
        }
    }

    printf("\nผลการค้นหา:\n");
    printf("ID | ชื่อ-นามสกุล | ชื่อเล่น | เพศ | วันที่สมัคร\n");
    printf("----------------------------------------------------------\n");

    char line[512];
    int found = 0;
    while(fgets(line, sizeof(line), fp)){
        int id, gender;
        char fullname[NAME_MAXLEN];
        char nickname[NICK_MAXLEN];
        char created_at[DATE_MAXLEN];

        if(sscanf(line, "%d|%127[^|]|%63[^|]|%d|%15[^\n]",
                  &id, fullname, nickname, &gender, created_at) == 5){
            int match = 0;
            if(mode == 1){
                if(id == search_id) match = 1;
            }else if(mode == 2){
                if(strstr(nickname, key) != NULL) match = 1;
            }else if(mode == 3){
                if(strstr(fullname, key) != NULL) match = 1;
            }

            if(match){
                printf("%d | %s | %s | %d | %s\n", id, fullname, nickname, gender, created_at);
                found = 1;
            }
        }
    }

    if(!found){
        printf("ไม่พบข้อมูลที่ค้นหา\n");
    }

    fclose(fp);
}

/* แสดงสมาชิกทั้งหมด */
void show_all_members(){
    FILE *fp = fopen("member.txt","r");
    if(fp == NULL){
        printf("ไม่พบไฟล์ member.txt\n");
        return;
    }

    printf("\n=== รายชื่อสมาชิกทั้งหมด ===\n");
    printf("ID | ชื่อ-นามสกุล | ชื่อเล่น | เพศ | วันที่สมัคร\n");
    printf("----------------------------------------------------------\n");

    char line[512];
    int count = 0;
    while(fgets(line, sizeof(line), fp)){
        int id, gender;
        char fullname[NAME_MAXLEN];
        char nickname[NICK_MAXLEN];
        char created_at[DATE_MAXLEN];

        if(sscanf(line, "%d|%127[^|]|%63[^|]|%d|%15[^\n]",
                  &id, fullname, nickname, &gender, created_at) == 5){
            printf("%d | %s | %s | %d | %s\n", id, fullname, nickname, gender, created_at);
            count++;
        }
    }

    if(count == 0){
        printf("ยังไม่มีสมาชิกในระบบ\n");
    }else{
        printf("----------------------------------------------------------\n");
        printf("จำนวนสมาชิกทั้งหมด: %d คน\n", count);
    }

    fclose(fp);
}

/* เมนูหลักของระบบสมาชิก – มี 0 เพื่อย้อนกลับแน่นอน */
void menu1_choose(){
    int menu1;

    while(1){
        printf("\n=== ระบบสมาชิก ===\n");
        printf("1. ลงทะเบียนสมาชิกก๊วน\n");
        printf("2. ค้นหารายชื่อในระบบ\n");
        printf("3. แสดงรายชื่อสมาชิกทั้งหมด\n");
        printf("0. กลับสู่หน้าหลัก\n");
        printf("ระบุเมนูที่ต้องการทำงาน : ");

        if(scanf("%d", &menu1) != 1){
            while(getchar()!='\n' && !feof(stdin)){}
            continue;
        }

        if(menu1 == 0){
            return; // ย้อนกลับไป main.c
        }else if(menu1 == 1){
            register_member();
        }else if(menu1 == 2){
            search_member();
        }else if(menu1 == 3){
            show_all_members();
        }else{
            printf("ไม่ตรงกับในเมนู โปรดลองอีกครั้ง\n");
        }
    }
}
