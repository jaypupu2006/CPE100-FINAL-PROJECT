#include <stdio.h>
#include <string.h>
#include <windows.h>

static void trim(char *str) { // ลบ space ท้ายตัวหนังสือจนถึง '\0'
    int len = strlen(str);
    if (len == 0) return;

    int i = len - 1; // เริ่มจากตัวสุดท้ายก่อน '\0'

    // เลื่อนถอยหลังจนกว่าจะเจอตัวที่ไม่ใช่ space
    while (i >= 0 && str[i] == ' ') { // หาตำแหน่งที่ไม่ใช่ ' ' (i)
        i--;
    }

    str[i + 1] = '\0'; // ปิด string ที่ตำแหน่ง i + 1
}

int main() {

    SetConsoleOutputCP(CP_UTF8); // ตั้งค่าหน้าจอให้ใช้ UTF-8
    SetConsoleCP(CP_UTF8);       // ตั้งค่าการรับ input ให้เป็น UTF-8 ด้วย เพื่อการรับ ouput ภาษาไทย

    int dummy;
    scanf("%d", &dummy);

    char key[50];
    printf("ป้อนคำที่ต้องการค้นหา (0 = ยกเลิก): ");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    fgets(key, sizeof(key), stdin);
    key[strcspn(key, "\n")] = '\0';
    trim(key); // ลบ space ท้ายตัวหนังสือจนถึง '\0'

    printf("คำที่ค้นหา คือ %s", key);
}