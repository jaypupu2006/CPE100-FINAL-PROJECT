#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "menu2.h"
#include "delay.h"

/*
 * menu2_choose: เมนูหลักสำหรับการเปิดก๊วนและจัดการข้อมูลรายวัน
 * - รับวันที่จากผู้ใช้เพื่อเลือกไฟล์รายวัน (เช่น DD-MM-YYYY.txt)
 * - ตรวจสอบ/สร้างไฟล์รายวัน, โหลดค่าราคา (จาก config.txt) และแสดงเมนูย่อย
 * - เมนูย่อยมีการลงชื่อเบิกลูก, สรุปยอดรายบุคคล, ค้นหายอดค้าง, สรุปรายวัน, และปรับราคา
 */
void menu2_choose(void) {
    char date[DATE_MAXLEN];
    char daily_path[64];
    Prices prices;
    int sub;

    printf("\n=== เมนูเปิดก๊วน / จัดการรายวัน ===\n");
    printf("กรอกวันที่ (DD-MM-YYYY) (0 = ย้อนกลับ): ");
    if (scanf("%15s", date) != 1) return;
    if (strcmp(date, "0") == 0) return;

    snprintf(daily_path, sizeof(daily_path), "%s.txt", date);

    if (!open_or_create_daily(daily_path)) {
        printf("ไม่สามารถเปิดหรือสร้างไฟล์รายวันได้\n");
        delay(3);
        return;
    }

    if (!load_prices("config.txt", &prices)) {
        printf("ไม่พบ config.txt จะใช้ค่าเริ่มต้น\n");
        prices.shuttle_price = 5;
        prices.court_fee_per_person = 50;
        save_prices("config.txt", &prices);
    }

    while (1) {
        system("cls");
        printf("\n=== เมนูจัดการวันที่ %s ===\n", date);
        printf("1. ลงชื่อเบิกลูก\n");
        printf("2. สรุปยอดที่ต้องชำระ (รายบุคคล)\n");
        printf("3. ค้นหายอดค้างชำระ\n");
        printf("4. สรุปข้อมูลรายวัน (แสดงบนหน้าจอ)\n");
        printf("5. เปลี่ยนราคาลูกแบด\n");
        printf("6. เปลี่ยนค่าสนามต่อคน\n");
        printf("0. ย้อนกลับ\n");
        printf("เลือกเมนู: ");

        if (scanf("%d", &sub) != 1) {
            while (getchar()!='\n' && !feof(stdin)) {}
            printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
            delay(2);
            continue;
        }

        if (sub == 0) {
            return; // ย้อนกลับไป main.c
        }

        if (sub == 1) {
            /* -------- เมนู 1: ลงชื่อเบิกลูก -------- */
            int count_player;
            printf("จำนวนผู้เล่นที่จะลงชื่อ (0 = ยกเลิก): ");
            if (scanf("%d", &count_player) != 1) {
                while (getchar()!='\n' && !feof(stdin)) {}
                printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
                delay(2);
                continue;
            }
            if (count_player <= 0) continue;

            for (int i = 0; i < count_player; i++) {
                int mode;
                char key[NAME_MAXLEN];
                Member *arr = NULL;
                size_t arr_count = 0;

                printf("\n[ผู้เล่นคนที่ %d]\n", i+1);
                printf("วิธีค้นหา 1=id 2=ชื่อเล่น 3=ชื่อ-นามสกุล 0=ยกเลิก: ");
                if (scanf("%d", &mode) != 1) {
                    while (getchar()!='\n' && !feof(stdin)) {}
                    continue;
                }
                if (mode == 0) continue;

                SearchBy by;
                if (mode == 1) by = BY_ID;
                else if (mode == 2) by = BY_NICKNAME;
                else if (mode == 3) by = BY_FULLNAME;
                else {
                    printf("เมนูค้นหาไม่ถูกต้อง\n");
                    continue;
                }

                printf("กรอกคำค้น (0 = ยกเลิก): ");
                getchar(); // เคลียร์ \n
                fgets(key, sizeof(key), stdin);
                key[strcspn(key, "\n")] = '\0';
                if (strcmp(key,"0") == 0) {
                    continue;
                }

                if (!search_members("member.txt", by, key, &arr, &arr_count) || arr_count == 0) {
                    printf("ไม่พบสมาชิกที่ตรงกับคำค้น\n");
                    free(arr);
                    continue;
                }

                size_t pick_index = 0;
                if (arr_count == 1) {
                    pick_index = 0;
                    printf("พบสมาชิก 1 คน: %s (%s)\n", arr[0].fullname, arr[0].nickname);
                } else {
                    printf("พบสมาชิก %zu คน:\n", arr_count);
                    for (size_t j=0; j<arr_count; j++) {
                        printf("%zu) %d | %s | %s\n", j+1, arr[j].id, arr[j].fullname, arr[j].nickname);
                    }
                    int pick;
                    printf("เลือกหมายเลข 1-%zu (0 = ยกเลิก): ", arr_count);
                    if (scanf("%d", &pick) != 1) {
                        while (getchar()!='\n' && !feof(stdin)) {}
                        free(arr);
                        continue;
                    }
                    if (pick == 0) {
                        free(arr);
                        continue;
                    }
                    if (pick < 1 || (size_t)pick > arr_count) {
                        printf("หมายเลขไม่ถูกต้อง\n");
                        free(arr);
                        continue;
                    }
                    pick_index = (size_t)(pick - 1);
                }

                Member sel = arr[pick_index];
                free(arr);

                int add_qty;
                printf("จำนวนลูกที่เบิกเพิ่ม (0 = ยกเลิก): ");
                if (scanf("%d", &add_qty) != 1) {
                    while (getchar()!='\n' && !feof(stdin)) {}
                    continue;
                }
                if (add_qty <= 0) {
                    printf("ยกเลิกการลงชื่อสำหรับคนนี้\n");
                    continue;
                }

                if (!upsert_daily_entry(daily_path, &prices, &sel, add_qty, PAY_NONE, 0, 0)) {
                    printf("บันทึกข้อมูลรายวันล้มเหลว\n");
                } else {
                    printf("บันทึกข้อมูลเรียบร้อย: %s (%s) +%d ลูก\n",
                           sel.fullname, sel.nickname, add_qty);
                }
            }
        } else if (sub == 2) {
            /* -------- เมนู 2: สรุปยอดที่ต้องชำระ (รายบุคคล) -------- */
            int mode;
            char key[NAME_MAXLEN];

            printf("\n=== สรุปยอดที่ต้องชำระ (รายบุคคล) ===\n");
            printf("วิธีค้นหา 1=id 2=ชื่อเล่น 3=ชื่อ-นามสกุล 0=ยกเลิก: ");
            if (scanf("%d", &mode) != 1) {
                while (getchar()!='\n' && !feof(stdin)) {}
                continue;
            }
            if (mode == 0) continue;

            SearchBy by;
            if (mode == 1) by = BY_ID;
            else if (mode == 2) by = BY_NICKNAME;
            else if (mode == 3) by = BY_FULLNAME;
            else {
                printf("เมนูค้นหาไม่ถูกต้อง\n");
                continue;
            }

            printf("กรอกคำค้น (0 = ยกเลิก): ");
            getchar();
            fgets(key, sizeof(key), stdin);
            key[strcspn(key, "\n")] = '\0';
            if (strcmp(key,"0")==0) continue;

            DailyEntry *darr = NULL;
            size_t dcount = 0;
            if (!search_daily(daily_path, by, key, &darr, &dcount) || dcount == 0) {
                printf("ไม่พบข้อมูลในไฟล์รายวันวันนี้\n");
                delay(3);
                free(darr);
                continue;
            }

            size_t pick_index = 0;
            if (dcount == 1) {
                pick_index = 0;
                printf("พบรายการ 1 รายการของสมาชิกนี้\n");
            } else {
                printf("พบรายการ %zu รายการ:\n", dcount);
                for (size_t j=0; j<dcount; j++) {
                    printf("%zu) member_id=%d nickname=%s shuttle=%d amount_today=%d\n",
                           j+1,
                           darr[j].member_id,
                           darr[j].nickname,
                           darr[j].shuttle_qty,
                           darr[j].amount_today);
                }
                int pick;
                printf("เลือกหมายเลข 1-%zu (0 = ยกเลิก): ", dcount);
                if (scanf("%d", &pick) != 1) {
                    while (getchar()!='\n' && !feof(stdin)) {}
                    free(darr);
                    continue;
                }
                if (pick == 0) {
                    free(darr);
                    continue;
                }
                if (pick < 1 || (size_t)pick > dcount) {
                    printf("หมายเลขไม่ถูกต้อง\n");
                    free(darr);
                    continue;
                }
                pick_index = (size_t)(pick - 1);
            }

            DailyEntry de = darr[pick_index];
            free(darr);

            printf("\nข้อมูลวันนี้ของสมาชิก:\n");
            printf("ID: %d | ชื่อเล่น: %s | ลูก: %d | ยอดวันนี้: %d\n",
                   de.member_id, de.nickname, de.shuttle_qty, de.amount_today);

            int act;
            printf("เลือกการชำระ 1=เงินสด 2=โอน 3=ค้าง 0=ยกเลิก: ");
            if (scanf("%d", &act) != 1) {
                while (getchar()!='\n' && !feof(stdin)) {}
                continue;
            }
            if (act == 0) continue;

            Member m;
            m.id = de.member_id;
            strncpy(m.fullname, de.fullname, NAME_MAXLEN);
            strncpy(m.nickname, de.nickname, NICK_MAXLEN);
            m.gender = de.gender;
            strncpy(m.created_at, "", DATE_MAXLEN); // ไม่ได้ใช้

            if (act == 1) {
                if (!upsert_daily_entry(daily_path, &prices, &m, 0, PAY_CASH, de.amount_today, 0))
                    printf("อัปเดตการชำระเงินสดล้มเหลว\n");
                else
                    printf("บันทึกการชำระเงินสดเรียบร้อย\n");
            } else if (act == 2) {
                if (!upsert_daily_entry(daily_path, &prices, &m, 0, PAY_TRANSFER, de.amount_today, 0))
                    printf("อัปเดตการชำระแบบโอนล้มเหลว\n");
                else
                    printf("บันทึกการชำระแบบโอนเรียบร้อย\n");
            } else if (act == 3) {
                if (!append_os("OSpayment.txt", &m, date, de.amount_today, "OS")) {
                    printf("บันทึกยอดค้างล้มเหลว\n");
                } else {
                    if (!upsert_daily_entry(daily_path, &prices, &m, 0, PAY_OS, 0, 0))
                        printf("อัปเดตรายวันสำหรับค้างจ่ายล้มเหลว\n");
                    else
                        printf("เปลี่ยนสถานะเป็นค้างจ่ายสำเร็จ\n");
                }
            } else {
                printf("เมนูผิดพลาด\n");
            }

        } else if (sub == 3) {
            /* -------- เมนู 3: ค้นหายอดค้างชำระ -------- */
            int mode;
            char key[NAME_MAXLEN];

            printf("\n=== ค้นหายอดค้างชำระ ===\n");
            printf("วิธีค้นหา 1=id 2=ชื่อเล่น 3=ชื่อ-นามสกุล 0=ยกเลิก: ");
            if (scanf("%d", &mode) != 1) {
                while (getchar()!='\n' && !feof(stdin)) {}
                continue;
            }
            if (mode == 0) continue;

            SearchBy by;
            if (mode == 1) by = BY_ID;
            else if (mode == 2) by = BY_NICKNAME;
            else if (mode == 3) by = BY_FULLNAME;
            else {
                printf("เมนูค้นหาไม่ถูกต้อง\n");
                continue;
            }

            printf("กรอกคำค้น (0 = ยกเลิก): ");
            getchar();
            fgets(key, sizeof(key), stdin);
            key[strcspn(key, "\n")] = '\0';
            if (strcmp(key,"0")==0) continue;

            OSEntry *os_arr = NULL;
            size_t os_cnt = 0;
            if (!search_os("OSpayment.txt", by, key, &os_arr, &os_cnt) || os_cnt == 0) {
                printf("ไม่พบยอดค้างของสมาชิกนี้\n");
                delay(3);
                free(os_arr);
                continue;
            }

            printf("พบยอดค้าง %zu รายการ:\n", os_cnt);
            for (size_t j=0; j<os_cnt; j++) {
                printf("%zu) ID=%d %s (%s) วันที่ =%s ค้างจ่าย=%d\n",
                       j+1,
                       os_arr[j].member_id,
                       os_arr[j].fullname,
                       os_arr[j].nickname,
                       os_arr[j].date,
                       os_arr[j].os_amount);
            }

            int pick;
            printf("เลือกหมายเลข 1-%zu (0 = ยกเลิก): ", os_cnt);
            if (scanf("%d", &pick) != 1) {
                while (getchar()!='\n' && !feof(stdin)) {}
                free(os_arr);
                continue;
            }
            if (pick == 0) {
                free(os_arr);
                continue;
            }
            if (pick < 1 || (size_t)pick > os_cnt) {
                printf("หมายเลขไม่ถูกต้อง\n");
                free(os_arr);
                continue;
            }

            OSEntry chosen = os_arr[pick-1];
            free(os_arr);

            int act;
            printf("ชำระหนี้ค้างนี้หรือไม่? 1=เงินสด 2=โอน 0=ยกเลิก: ");
            if (scanf("%d", &act) != 1) {
                while (getchar()!='\n' && !feof(stdin)) {}
                continue;
            }
            if (act == 0) continue;

            if (!remove_os_entry("OSpayment.txt", &chosen)) {
                printf("ลบยอดค้างไม่สำเร็จ\n");
            } else {
                printf("ชำระหนี้ค้างเรียบร้อย และลบออกจากค้างจ่ายแล้ว\n");
            }

        } else if (sub == 4) {
            /* -------- เมนู 4: สรุปข้อมูลรายวันบนหน้าจอ -------- */
            summarize_daily(daily_path, 1);

        } else if (sub == 5) {
            /* -------- เมนู 5: เปลี่ยนราคาลูก -------- */
            printf("ราคาลูกแบดปัจจุบัน: %d\n", prices.shuttle_price);
            printf("กรอกราคาลูกใหม่ (0 = ยกเลิก): ");
            int p;
            if (scanf("%d", &p) != 1) {
                while (getchar()!='\n' && !feof(stdin)) {}
                continue;
            }
            if (p == 0) continue;
            prices.shuttle_price = p;
            save_prices("config.txt", &prices);
            printf("บันทึกราคาลูกใหม่แล้ว\n");

        } else if (sub == 6) {
            /* -------- เมนู 6: เปลี่ยนค่าสนาม -------- */
            printf("ค่าสนามต่อคนปัจจุบัน: %d\n", prices.court_fee_per_person);
            printf("กรอกค่าสนามต่อคนใหม่ (0 = ยกเลิก): ");
            int p;
            if (scanf("%d", &p) != 1) {
                while (getchar()!='\n' && !feof(stdin)) {}
                continue;
            }
            if (p == 0) continue;
            prices.court_fee_per_person = p;
            save_prices("config.txt", &prices);
            printf("บันทึกค่าสนามใหม่แล้ว\n");

        } else {
            printf("เมนูไม่ถูกต้อง โปรดลองอีกครั้ง\n");
        }
        delay(3);
    }
}
