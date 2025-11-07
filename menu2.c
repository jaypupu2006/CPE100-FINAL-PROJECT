#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>     // ← เพิ่มบรรทัดนี้
#include "menu2.h"


void menu2_choose(void) {
    char date[16], daily_path[64];
    Prices prices;
    int sub;

    printf("กรอกวันที่ (DD-MM-YYYY): ");
    if (scanf("%15s", date) != 1) return;
    snprintf(daily_path, sizeof(daily_path), "%s.txt", date);

    if (!open_or_create_daily(daily_path)) {
        printf("เปิด/สร้างไฟล์รายวันไม่สำเร็จ\n");
        return;
    }

    if (!load_prices("config.txt", &prices)) {
        prices.shuttle_price = 5;
        prices.court_fee_per_person = 50;
        save_prices("config.txt", &prices);
    }

    for (;;) {
        printf("\nเมนูเปิดก๊วน (%s)\n", daily_path);
        printf("1. ลงชื่อเบิกลูก\n");
        printf("2. สรุปยอดที่ต้องชำระ (รายบุคคล)\n");
        printf("3. ค้นหายอดค้างชำระ (รายบุคคล)\n");
        printf("4. สรุปข้อมูลรายวัน\n");
        printf("5. เปลี่ยนราคาลูกขนไก่\n");
        printf("6. เปลี่ยนราคาสนามต่อคน\n");
        printf("0. ย้อนกลับ\n");
        printf("เลือก: ");
        if (scanf("%d", &sub) != 1) return;
        if (sub == 0) return;

        if (sub == 1) {
            int players;
            printf("กรอกจำนวนผู้เล่น: ");
            if (scanf("%d", &players) != 1 || players <= 0) continue;
            for (int p = 1; p <= players; p++) {
                int by, c, pick, idx = -1, add_qty;
                char key[128];
                Member *arr = NULL; size_t cnt = 0;

                while ((c = getchar()) != '\n' && c != EOF) {}
                printf("\nผู้เล่นคนที่ %d\n", p);
                printf("เลือกวิธีค้นหา 1=เลขสมาชิก 2=ชื่อเล่น 3=ชื่อ-นามสกุล: ");
                if (scanf("%d", &by) != 1 || by < 1 || by > 3) { free(arr); continue; }

                while ((c = getchar()) != '\n' && c != EOF) {}
                printf("กรอกคำค้นหา: ");
                if (!fgets(key, sizeof(key), stdin)) { free(arr); continue; }
                key[strcspn(key, "\n")] = 0;

                if (!search_members("member.txt", (SearchBy)by, key, &arr, &cnt)) { free(arr); continue; }
                if (cnt == 0) { printf("ไม่พบข้อมูล\n"); free(arr); continue; }
                if (cnt == 1) idx = 0;
                else {
                    for (size_t i = 0; i < cnt; i++) printf("%zu) %d | %s (%s)\n", i + 1, arr[i].id, arr[i].fullname, arr[i].nickname);
                    printf("เลือกหมายเลข 1-%zu (0 ยกเลิก): ", cnt);
                    if (scanf("%d", &pick) != 1 || pick <= 0 || (size_t)pick > cnt) { free(arr); continue; }
                    idx = pick - 1;
                }

                if (idx < 0) { free(arr); continue; }

                printf("จำนวนลูกที่เพิ่ม: ");
                if (scanf("%d", &add_qty) != 1 || add_qty <= 0) { free(arr); continue; }

                if (!upsert_daily_entry(daily_path, &prices, &arr[idx], add_qty, PAY_NONE, 0, 0))
                    printf("บันทึกล้มเหลว\n");
                else
                    printf("บันทึกแล้ว: %s (+%d ลูก)\n", arr[idx].nickname, add_qty);

                free(arr);
            }
        } else if (sub == 2) {
            int by, c, pick, idx = 0, act;
            char key[128];
            DailyEntry *arr = NULL; size_t cnt = 0;

            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("เลือกวิธีค้นหา 1=เลขสมาชิก 2=ชื่อเล่น 3=ชื่อ-นามสกุล: ");
            if (scanf("%d", &by) != 1 || by < 1 || by > 3) { free(arr); continue; }

            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("กรอกคำค้นหา: ");
            if (!fgets(key, sizeof(key), stdin)) { free(arr); continue; }
            key[strcspn(key, "\n")] = 0;

            if (!search_daily(daily_path, (SearchBy)by, key, &arr, &cnt)) { free(arr); continue; }
            if (cnt == 0) { printf("ไม่พบข้อมูลวันนี้\n"); free(arr); continue; }

            if (cnt > 1) {
                for (size_t i = 0; i < cnt; i++)
                    printf("%zu) %d | %s (%s) ลูก:%d ยอดวันนี้:%d\n", i + 1, arr[i].member_id, arr[i].fullname, arr[i].nickname, arr[i].shuttle_qty, arr[i].amount_today);
                printf("เลือกหมายเลข 1-%zu (0 ยกเลิก): ", cnt);
                if (scanf("%d", &pick) != 1 || pick <= 0 || (size_t)pick > cnt) { free(arr); continue; }
                idx = pick - 1;
            }

            if (idx < 0) { free(arr); continue; }

            printf("รหัส:%d | %s (%s) | ลูก:%d | ยอดวันนี้:%d\n",
                   arr[idx].member_id, arr[idx].fullname, arr[idx].nickname, arr[idx].shuttle_qty, arr[idx].amount_today);
            printf("เลือกการชำระ 1=เงินสด 2=โอน 3=ค้าง 0=ยกเลิก: ");
            if (scanf("%d", &act) != 1 || act == 0) { free(arr); continue; }

            Member m;
            m.id = arr[idx].member_id;
            strncpy(m.fullname, arr[idx].fullname, sizeof m.fullname);
            m.fullname[sizeof m.fullname - 1] = '\0';
            strncpy(m.nickname, arr[idx].nickname, sizeof m.nickname);
            m.nickname[sizeof m.nickname - 1] = '\0';
            m.gender = arr[idx].gender;
            m.created_at[0] = '\0';

            if (act == 1) {
                if (!upsert_daily_entry(daily_path, &prices, &m, 0, PAY_CASH, arr[idx].amount_today, 0))
                    printf("อัปเดตล้มเหลว\n");
                else
                    printf("รับชำระเงินสดแล้ว\n");
            } else if (act == 2) {
                if (!upsert_daily_entry(daily_path, &prices, &m, 0, PAY_TRANSFER, arr[idx].amount_today, 0))
                    printf("อัปเดตล้มเหลว\n");
                else
                    printf("รับชำระโอนแล้ว\n");
            } else if (act == 3) {
                if (!append_os("OSpayment.txt", &m, date, arr[idx].amount_today, "OS"))
                    printf("บันทึกค้างชำระล้มเหลว\n");
                else if (!upsert_daily_entry(daily_path, &prices, &m, 0, PAY_OS, 0, 0))
                    printf("อัปเดตรายวันล้มเหลว\n");
                else
                    printf("เปลี่ยนเป็นค้างชำระแล้ว\n");
            }
            free(arr);
        } else if (sub == 3) {
            int by, c, pick, idx = 0, how;
            char key[128];
            OSEntry *arr = NULL; size_t cnt = 0;

            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("เลือกวิธีค้นหา 1=เลขสมาชิก 2=ชื่อเล่น 3=ชื่อ-นามสกุล: ");
            if (scanf("%d", &by) != 1 || by < 1 || by > 3) { free(arr); continue; }

            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("กรอกคำค้นหา: ");
            if (!fgets(key, sizeof(key), stdin)) { free(arr); continue; }
            key[strcspn(key, "\n")] = 0;

            if (!search_os("OSpayment.txt", (SearchBy)by, key, &arr, &cnt)) { free(arr); continue; }
            if (cnt == 0) { printf("ไม่พบหนี้ค้าง\n"); free(arr); continue; }

            if (cnt > 1) {
                for (size_t i = 0; i < cnt; i++)
                    printf("%zu) %d | %s (%s) | %s | ค้าง %d\n", i + 1, arr[i].member_id, arr[i].fullname, arr[i].nickname, arr[i].date, arr[i].os_amount);
                printf("เลือกหมายเลข 1-%zu (0 ยกเลิก): ", cnt);
                if (scanf("%d", &pick) != 1 || pick <= 0 || (size_t)pick > cnt) { free(arr); continue; }
                idx = pick - 1;
            }

            if (idx < 0) { free(arr); continue; }

            printf("ชำระค้าง %d บาท ของ %s วันที่ %s วิธี 1=เงินสด 2=โอน 0=ยกเลิก: ",
                   arr[idx].os_amount, arr[idx].nickname, arr[idx].date);
            if (scanf("%d", &how) != 1 || how == 0) { free(arr); continue; }

            if (!remove_os_entry("OSpayment.txt", &arr[idx]))
                printf("ลบค้างชำระล้มเหลว\n");
            else
                printf("ชำระแล้ว และลบออกจาก OSpayment.txt\n");
            free(arr);
        } else if (sub == 4) {
            summarize_daily(daily_path, 1);
        } else if (sub == 5) {
            printf("ราคาลูกเดิม: %d บาท  ใส่ราคาใหม่: ", prices.shuttle_price);
            if (scanf("%d", &prices.shuttle_price) == 1) save_prices("config.txt", &prices);
        } else if (sub == 6) {
            printf("ค่าสนาม/คน เดิม: %d บาท  ใส่ราคาใหม่: ", prices.court_fee_per_person);
            if (scanf("%d", &prices.court_fee_per_person) == 1) save_prices("config.txt", &prices);
        } else {
            printf("เมนูไม่ถูกต้อง\n");
        }
    }
}
