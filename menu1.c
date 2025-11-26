#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include <ctype.h>
//#include <windows.h>
#include "Library/delay.h"
#define NAME_MAXLEN 128
#define NICK_MAXLEN 64
#define GENDER_MAXLEN 32
#define DATE_MAXLEN 11 // เก็บ วัน/เดือน/ปี


void delete_buffle()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF); // ล้าง buffer ทั้งหมด จนถึง \n หรือ EOF
}

/* ตรวจว่า member.txt เปิดได้ไหม ถ้าไม่ได้ให้สร้างไฟล์ใหม่ */
int input_file()
{
    FILE *member = fopen("input/member.txt", "r"); // เปิดไฟล์ member.txt
    if (member == NULL)
    {                                            // ถ้า pointer member ไม่ได้ชี้ไปที่ไหนเลย จะเข้าเงื่อนไข
        member = fopen("input/member.txt", "w"); // "w" จะสร้างไฟล์ member.txt ให้เองถ้าไม่มีไฟล์ดังกล่าว
        if (member == NULL)
        { // ถ้ายัง NULL อีก จะแสดงไม่สามารถสร้างไฟล์ดังกล่าวได้ อาจจะเพราะ path ไม่ถูกต้อง
            printf("ไม่สามารถสร้างไฟล์ member.txt ได้ เปิดหาโฟล์เดอร์ \"input\" ไม่เจอ\n");
            delay(3);
            return 0;
        }
    }
    fclose(member);
    return 1;
}

/* สร้างวันที่ปัจจุบันเป็น YYYY-MM-DD */
static void get_today(char buf[DATE_MAXLEN])
{                                                    // static เพราะ จะใช้งานแค่ในไฟล์นี้เท่านั้น
    time_t t = time(NULL);                           // ค่าเวลาที่ที่เป็นวินาที ตั้งแต่ 00:00 ปี 1970
    struct tm *tm_info = localtime(&t);              // แปลงจากวินาทีเป็น ปี-เดือน-วัน-ชั่วโมง-นาที-วินาที ปกติ -- pointer tm_info ชี้ไปยัง struct tm ที่อยู่ใน library time.h
    strftime(buf, DATE_MAXLEN, "%d-%m-%Y", tm_info); // แปลงค่าที่ tm_info ชี้ ให้เป็น string วัน-เดือน-ปี เก็บใน buf
}

void trim(char *str)
{ // ลบ space ท้ายตัวหนังสือจนถึง '\0'
    int len = strlen(str);
    if (len == 0)
        return;

    int i = len - 1; // เริ่มจากตัวสุดท้ายก่อน '\0'

    // เลื่อนถอยหลังจนกว่าจะเจอตัวที่ไม่ใช่ space
    while (i >= 0 && str[i] == ' ')
    { // หาตำแหน่งที่ไม่ใช่ ' ' (i)
        i--;
    }

    str[i + 1] = '\0'; // ปิด string ที่ตำแหน่ง i + 1
}

/* ลงทะเบียนสมาชิกใหม่ */
void register_member()
{
    system("cls");

    FILE *fp;
    int last_id = 0;

    fp = fopen("input/member.txt", "r"); // เปิดอ่านไฟล์ member.txt
    if (fp != NULL)
    {                   // ถ้าอ่านได้
        char line[500]; // ประกาศเผื่อ

        // while loop หาค่า id สูงสุดในไฟล์ member.txt
        while (fgets(line, sizeof(line), fp))
        { // อ่านไฟล์ member.txt ทีละบรรทัด ถ้าอ่านได้จะเข้า
            int id;
            if (sscanf(line, "%d|", &id))
            { // ดึงค่า id จากบรรทัดนั้นมา
                if (id > last_id)
                    last_id = id; // ถ้าไอดีที่ได้ มากกว่า last_id
            }
        }
        fclose(fp); // ปิดการอ่านไฟล์
    }
    else
    {
        printf("ไม่สามารถเปิด member.txt\n");
        input_file(); // ไปเช็คอีกครั้งว่าทำไมไม่สามารถอ่านได้
        delay(3);
        return; // ย้อนกลับไป menu1_choose
    }

    int new_id = last_id + 1; // id ใหม่ คือ last_id + 1
    char fullname[NAME_MAXLEN];
    char nickname[NICK_MAXLEN];
    int gen;
    char gender[GENDER_MAXLEN];
    char created_at[DATE_MAXLEN];

    printf("\n=== ลงทะเบียนสมาชิกใหม่ ===\n");
    printf("ป้อนชื่อ-นามสกุล #เว้นวรรคคั่น (0 = ยกเลิก) : ");

    getchar(); // เคลียร์ \n ค้างจาก scanf จาก menu1_choose

    fgets(fullname, sizeof(fullname), stdin);

    fullname[strcspn(fullname, "\n")] = '\0'; // แทนที่ '\n' เป็น '\0'
    trim(fullname);                           // ลบ space ท้ายตัวหนังสือจนถึง '\0'

    if (strcmp(fullname, "0") == 0)
    { // กรอก 0 = ย้อนกลับ
        fclose(fp);
        return;
    }

    if (strlen(fullname) == 0)
    {
        printf("ยกเลิกการลงทะเบียน (ไม่ได้กรอกชื่อ)\n");
        delay(3);
        return;
    }

    printf("ป้อนชื่อเล่น (0 = ยกเลิก) : ");
    fgets(nickname, sizeof(nickname), stdin);
    nickname[strcspn(nickname, "\n")] = '\0'; // แทนที่ '\n' เป็น '\0'
    trim(nickname);                           // ลบ space ท้ายตัวหนังสือจนถึง '\0'

    if (strcmp(nickname, "0") == 0)
    { // กรอก 0 = ย้อนกลับ
        fclose(fp);
        return;
    }

    if (strlen(nickname) == 0)
    {
        printf("ยกเลิกการลงทะเบียน (ไม่ได้กรอกชื่อเล่น)\n");
        delay(3);
        return;
    }

    printf("เพศ (1 = ชาย, 2 = หญิง, 0 = ยกเลิก): ");

    if (scanf("%d", &gen) != 1)
    {
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
        {
        } // ล้าง buffer ทั้งหมด จนถึง \n หรือ EOF
        printf("ข้อมูลไม่ถูกต้อง ยกเลิกการลงทะเบียน\n");
        delay(3);
        return;
    }

    if (gen == 0)
    {
        printf("ยกเลิกการลงทะเบียน\n");
        delay(3);
        return;
    }
    else if (gen == 1)
        strcpy(gender, "ชาย");
    else if (gen == 2)
        strcpy(gender, "หญิง");
    else
    {
        printf("\nไม่มีเมนูเลข %d โปรดลองอีกครั้ง\n", gen);
        delay(2);
        return;
    }

    get_today(created_at); // ดึงข้อมูล วัน/เดือน/ปี ปัจจุบัน

    fp = fopen("input/member.txt", "a"); // เปิดไฟล์เพื่อเขียนต่อท้าย
    if (fp == NULL)
    { // เช็คทุกครั้งว่าเปิดได้ไหม
        printf("ไม่สามารถเปิด member.txt\n");
        input_file(); // ไปเช็คอีกครั้งว่าทำไมไม่สามารถอ่านได้
        delay(3);
        return;
    }

    fprintf(fp, "%d|%s|%s|%s|%s\n", new_id, fullname, nickname, gender, created_at);

    printf("\nลงทะเบียนสำเร็จ! รหัสสมาชิกของคุณคือ: %d\n", new_id);

    printf("\nพิมพ์ 0 เพื่อย้อนกลับ : "); // จริง ๆ พิมพ์อะไรก็ย้อนกลับได้ เพราะสุดท้ายแล้ว จะพิมพ์อะไรก็จะออกจาก Function อยู่ดี
    int _tmp;
    scanf("%d", &_tmp);
    fclose(fp);
}

/* ค้นหาสมาชิก */
void search_member()
{
    FILE *fp = fopen("input/member.txt", "r");
    if (fp == NULL)
    {
        printf("ไม่สามารถเปิด member.txt\n");
        input_file(); // ไปเช็คอีกครั้งว่าทำไมไม่สามารถอ่านได้
        delay(3);
        return;
    }

    int mode;
    char key[NAME_MAXLEN];
    int search_id = 0;

    

    while (1)
    {
        system("cls");
        printf("=== ค้นหาสมาชิก ===\n");
        printf("1. ค้นหาด้วยรหัสสมาชิก\n");
        printf("2. ค้นหาด้วยชื่อเล่น\n");
        printf("3. ค้นหาด้วยชื่อ-นามสกุล\n");
        printf("0. ย้อนกลับ\n");
        printf("เลือก : ");

        if (scanf("%d", &mode) != 1)
        {
            delete_buffle();
            printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
            delay(3);
            continue;
        }

        if (mode == 0)
        { // ย้อนกลับ
            fclose(fp);
            return;
        }

        if (mode == 1)
        {
            printf("ป้อนรหัสสมาชิก (0 = ยกเลิก) : ");
            if (scanf("%d", &search_id) != 1)
            {
                delete_buffle();
                printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
                delay(3);
                continue;
            }
            if (search_id == 0)
            {
                continue;
            }
        }
        else if (mode == 2 || mode == 3)
        {
            printf("ป้อนคำที่ต้องการค้นหา (0 = ยกเลิก) : ");
            delete_buffle();
            fgets(key, sizeof(key), stdin);
            key[strcspn(key, "\n")] = '\0';
            trim(key); // ลบ space ท้ายตัวหนังสือจนถึง '\0'

            if (strcmp(key, "0") == 0)
            { // ย้อยกลับ
                continue;
            }
        }
        else
        {
            printf("\nไม่มีเมนูเลข %d โปรดลองอีกครั้ง\n", mode);
            delay(3);
            continue;
        }
        break;
    }

    printf("\n=== ผลการค้นหา ===\n");
    printf("\n |  ID  | ชื่อ-นามสกุล | ชื่อเล่น | เพศ |   วันที่สมัคร   |\n");
    printf("----------------------------------------------------------\n");

    char line[500];
    int found = 0;
    while (fgets(line, sizeof(line), fp))
    { // อ่าน member.txt ทีละบรรทัด เก็บใน line จนกว่าจะไม่สามารถอ่านได้
        int id;
        char fullname[NAME_MAXLEN];
        char nickname[NICK_MAXLEN];
        char gender[GENDER_MAXLEN];
        char created_at[DATE_MAXLEN];

        /*
        sscanf แยก string และเลขไปเก็บในแต่ละตัวแปร
         %50[^|] อ่าน char ไป 50 ตัว หรือจนกว่าจะเจอ |
        */
        if (sscanf(line, "%d|%127[^|]|%63[^|]|%31[^|]|%10[^\n]", &id, fullname, nickname, gender, created_at) == 5)
        {

            int match = 0; // base case
            if (mode == 1)
            {
                if (id == search_id)
                    match = 1; // id ที่อ่าน เหมือนกับ id ที่ผู้ใช้กรอก
            }
            else if (mode == 2)
            {
                if (strstr(nickname, key) != NULL)
                    match = 1; // strstr จาก string.h หา Sub String
            }
            else if (mode == 3)
            {
                if (strstr(fullname, key) != NULL)
                    match = 1; // key เป็น Sub ของ fullname หรือไม่
            }

            if (match)
            {
                printf(" |%4d | %s | %s | %-4s | %-10s |\n", id, fullname, nickname, gender, created_at); // ถ้าเจอหลายข้อมูลก็จะ show หมด
                found++;
            }
        }
    }
    if (found > 0)
    {
        printf("\nพบข้อมูลทั้งหมด %d รายการ\n", found);
    }
    else
    {
        printf("\nไม่พบข้อมูลที่ค้นหา\n");
    }
    printf("\nพิมพ์ 0 เพื่อย้อนกลับ : "); // จริง ๆ พิมพ์อะไรก็ย้อนกลับได้ เพราะสุดท้ายแล้ว จะพิมพ์อะไรก็จะออกจาก Function อยู่ดี
    int _tmp;
    scanf("%d", &_tmp);
    fclose(fp);
}

/* แสดงสมาชิกทั้งหมด */
void show_all_members()
{
    FILE *fp = fopen("input/member.txt", "r");
    if (fp == NULL)
    {
        printf("ไม่พบไฟล์ member.txt\n");
        input_file(); // ไปเช็คอีกครั้งว่าทำไมไม่สามารถอ่านได้
        delay(3);
        return;
    }

    printf("\n=== รายชื่อสมาชิกทั้งหมด ===\n");
    printf("\n |  ID  |    ชื่อ-นามสกุล    |  ชื่อเล่น  | เพศ |   วันที่สมัคร   |\n");
    printf("----------------------------------------------------------\n");

    char line[500];
    int count = 0;
    while (fgets(line, sizeof(line), fp))
    { // อ่านข้อมูลจากไฟล์ member.txt ทีละ บรรทัด จนกว่าจะไม่สามารถอ่านได้
        int id;
        char fullname[NAME_MAXLEN];
        char nickname[NICK_MAXLEN];
        char gender[GENDER_MAXLEN];
        char created_at[DATE_MAXLEN];

        if (sscanf(line, "%d|%127[^|]|%63[^|]|%31[^|]|%10[^\n]", &id, fullname, nickname, gender, created_at) == 5)
        { // แยกข้อมูล
            /* printf("----------------------------------------------------------\n");
             printf("ID         : ");
             printf("%d\n", id);
             printf("ชื่อ-นามสกุล : ");
             printf("%s\n", fullname);
             printf("ชื่อเล่น      : ");
             printf("%s\n", nickname);
             printf("เพศ        : ");
             printf("%s\n", gender);
             printf("วันที่สมัคร    : ");
             printf("%s\n", created_at);*/
            printf(" |%4d  | %-25s | %-8s | %-4s | %-10s |\n", id, fullname, nickname, gender, created_at);
            count++;
        }
    }

    if (count == 0)
    {
        printf("\nยังไม่มีสมาชิกในระบบ\n");
    }
    else
    {
        printf("----------------------------------------------------------\n");
        printf("\nจำนวนสมาชิกทั้งหมด : %d คน\n", count);
    }
    printf("\nพิมพ์ 0 เพื่อย้อนกลับ : "); // พิมพ์อะไรก็ได้
    int _tmp;
    scanf("%d", &_tmp);
    fclose(fp);
}

/* เมนูหลักของระบบสมาชิก – มี 0 เพื่อย้อนกลับ */
void menu1_choose()
{

    /*  ฟังก์ชันจาก menu1.h -- ตรวจว่า member.txt เปิดได้ไหม ถ้าไม่ได้ให้สร้างไฟล์ใหม่
        ถ้าสร้างไฟล์ใหม่ไม่ได้ อาจจะเพราะ Folder input ไม่มี
    */
    if (!input_file())
        return; // ถ้าสร้างไฟล์ไม่ได้จะจบการทำงาน

    int menu1;

    while (1)
    {

        system("cls"); // เคลียร์หน้าจอ cmd

        printf("=== ระบบสมาชิก ===\n");
        printf("1. ลงทะเบียนสมาชิกก๊วน\n");
        printf("2. ค้นหารายชื่อในระบบ\n");
        printf("3. แสดงรายชื่อสมาชิกทั้งหมด\n");
        printf("0. กลับสู่หน้าหลัก\n");
        printf("ระบุเมนูที่ต้องการทำงาน : ");

        if (scanf("%d", &menu1) != 1)
        { // ถ้าผู้ใช้ไม่ได้กรอกตัวเลข
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
            {
            } // ล้าง buffer ทั้งหมด จนถึง \n หรือ EOF
            printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
            delay(2);
            continue;
        }

        if (menu1 == 0)
        {
            return; // ย้อนกลับไป main.c
        }
        else if (menu1 == 1)
        {
            register_member();
        }
        else if (menu1 == 2)
        {
            search_member();
        }
        else if (menu1 == 3)
        {
            show_all_members();
        }
        else
        {
            printf("\nไม่มีเมนูเลข %d โปรดลองอีกครั้ง\n", menu1);
            delay(3);
        }
    }
}
