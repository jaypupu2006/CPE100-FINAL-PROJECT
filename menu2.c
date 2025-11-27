#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Library/menu1.h"
#include "Library/menu2.h"
#include "Library/delay.h"

/*
 * menu2_choose: เมนูหลักสำหรับการเปิดก๊วนและจัดการข้อมูลรายวัน
 * - รับวันที่จากผู้ใช้เพื่อเลือกไฟล์รายวัน (เช่น DD-MM-YYYY.txt)
 * - ตรวจสอบ/สร้างไฟล์รายวัน, โหลดค่าคราคา (จาก config.txt) และแสดงเมนูย่อย
 * - เมนูย่อยมีการลงชื่อเบิกลูก, สรุปยอดรายบุคคล, ค้นหายอดค้าง, สรุปรายวัน, และปรับราคา
 */

static void sub1(const char *daily_path)
{
    /* -------- เมนู 1: ลงชื่อเบิกลูก -------- */
    Prices prices;
    if (!load_prices("input/config.txt", &prices)) // เปิดเพื่ออ่านค่าคอร์ท และค่าลูก
    {
        printf("ไม่พบ config.txt ในโฟลเดอร์ \"input\" จะใช้ค่าเริ่มต้น\n");
        prices.shuttle_price = 100;
        prices.court_fee_per_person = 100;
    }

    int count_player; // จำนวนผู้เล่นที่จะหาร
    while (1)
    {
        system("cls");
        printf("=== เมนูลงชื่อเบิกลูก ===\n");
        printf("จำนวนผู้เล่นที่จะลงชื่อ (0 = ยกเลิก) : ");
        if (scanf("%d", &count_player) != 1)
        {
            delete_buffle();
            printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
            delay(3);
            continue;
        }
        if (count_player == 0)
            return;
        if (count_player < 0) // กรอกน้อยกว่า 0 ไม่ได้
        {
            printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
            delay(3);
            continue;
        }
        if (count_player > 100)
        {
            printf("\nจำกัดที่ 100 คน\n");
            delay(3);
            continue;
        }
        break;
    }

    for (int i = 0; i < count_player; i++) // วนให้ครบจำนวนผู้เล่น
    {
        while (1)
        {
            int mode;
            char key[NAME_MAXLEN];
            Member *member = NULL;
            int member_count = 0;

            printf("\n[ผู้เล่นคนที่ %d]\n", i + 1);
            printf("วิธีค้นหา 1 = id, 2 = ชื่อเล่น, 3 = ชื่อ-นามสกุล, 0 = ยกเลิก : ");

            if (scanf("%d", &mode) != 1) // ไม่ใช่ตัวเลข
            {
                printf("กรุณากรอกตัวเลขเท่านั้น!\n");
                delete_buffle();
                delay(3);
                continue;
            }

            if (mode == 0)
                return; // ยกเลิกเมนู

            if (mode < 1 || mode > 3) // ตัวเลขไม่ถูกต้อง
            {
                printf("กรุณากรอกตัวเลขให้ถูกต้อง!\n");
                delete_buffle();
                delay(3);
                continue;
            }

            SearchBy by;
            if (mode == 1)
                by = BY_ID;
            else if (mode == 2)
                by = BY_NICKNAME;
            else
                by = BY_FULLNAME;

            delete_buffle();
            printf("กรอกคำค้นหา (0 = ยกเลิก) : ");
            fgets(key, sizeof(key), stdin);
            key[strcspn(key, "\n")] = '\0';
            trim(key);

            if (strcmp(key, "0") == 0)
                return; // ยกเลิก

            if (!search_members(by, key, &member, &member_count) || member_count == 0) // ค้นหา
            {
                printf("ไม่พบสมาชิกที่ตรงกับคำค้น\n");
                continue; // กลับไปลูปเดิม เริ่มต้นใหม่
            }

            int pick_index = 0;
            if (member_count == 1)
            {
                printf("\n=== ผลการค้นหา ===\n");
                printf("\n   |  ID  |    ชื่อ-นามสกุล    |  ชื่อเล่น  | เพศ |   วันที่สมัคร   |\n");
                printf("----------------------------------------------------------\n");
                printf("%d) |%4d | %s | %s | %-4s | %-10s |\n", 1, member[0].id, member[0].fullname, member[0].nickname, member[0].gender, member[0].created_at);
                printf("----------------------------------------------------------\n");
            }
            else
            {
                printf("\n=== ผลการค้นหา ===\n");
                printf("\n   |  ID  |    ชื่อ-นามสกุล    |  ชื่อเล่น  | เพศ |   วันที่สมัคร   |\n");
                printf("----------------------------------------------------------\n");
                for (int j = 0; j < member_count; j++)
                {
                    printf("%d) |%4d | %s | %s | %-4s | %-10s |\n", j + 1, member[j].id, member[j].fullname, member[j].nickname, member[j].gender, member[j].created_at);
                }
                printf("----------------------------------------------------------\n");
                printf("\nพบสมาชิก %d คน:\n", member_count);
                int pick;
                printf("เลือกหมายเลข 1-%d (0 = ยกเลิก) : ", member_count);
                if (scanf("%d", &pick) != 1)
                {
                    delete_buffle();
                    printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
                    free(member);
                    delay(3);
                    continue;
                }
                if (pick == 0)
                {
                    continue;
                }
                if (pick < 1 || pick > member_count)
                {
                    printf("หมายเลขไม่ถูกต้อง\n");
                    free(member);
                    delay(3);
                    continue;
                }
                pick_index = pick - 1;
            }

            Member select = member[pick_index]; // ข้อมูลที่เลือกเก็บใน select
            free(member);

            int add_qty;
            printf("จำนวนลูกที่เบิกเพิ่ม (0 = ยกเลิก) : ");
            if (scanf("%d", &add_qty) != 1)
            {
                printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
                delete_buffle();
                delay(3);
                continue;
            }
            if (add_qty <= 0)
            {
                printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
                delete_buffle();
                delay(3);
                continue;
            }

            // When adding new quantity, we pass check_update=1 (add mode), method PAY_OS by default
            if (!upsert_daily_entry(daily_path, &prices, &select, add_qty, count_player, PAY_OS, 0, 0, 1))
            {
                printf("บันทึกข้อมูลรายวันล้มเหลว\n");
                delay(3);
            }
            else
            {
                printf("บันทึกข้อมูลเรียบร้อย: %s (%s) +%d ลูก\n", select.fullname, select.nickname, add_qty);
                delay(3);
            }
            break;
        } // end while(1)
    }
}

static void sub2(char *daily_path)
{
    Prices prices;
    if (!load_prices("input/config.txt", &prices)) // เปิดเพื่ออ่านค่าคอร์ท และค่าลูก
    {
        printf("ไม่พบ config.txt ในโฟลเดอร์ \"input\" จะใช้ค่าเริ่มต้น\n");
        prices.shuttle_price = 100;
        prices.court_fee_per_person = 100;
    }

    while (1)
    {
        int mode;
        char key[NAME_MAXLEN];
        system("cls");
        printf("\n=== สรุปยอดที่ต้องชำระ (รายบุคคล) ===\n");
        printf("วิธีค้นหา 1 = ID, 2 = ชื่อเล่น, 3 = ชื่อ-นามสกุล 0 = ยกเลิก : ");
        if (scanf("%d", &mode) != 1)
        {
            printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
            delete_buffle();
            delay(3);
            continue;
        }
        if (mode == 0)
            return;

        SearchBy by;
        if (mode == 1)
            by = BY_ID;
        else if (mode == 2)
            by = BY_NICKNAME;
        else if (mode == 3)
            by = BY_FULLNAME;
        else
        {
            printf("เมนูค้นหาไม่ถูกต้อง\n");
            delay(3);
            continue;
        }

        printf("กรอกคำค้น (0 = ยกเลิก) : ");
        delete_buffle();
        fgets(key, sizeof(key), stdin);
        key[strcspn(key, "\n")] = '\0';
        trim(key);
        if (strcmp(key, "0") == 0)
            continue;

        DailyEntry *daily = NULL;
        int dcount = 0;
        if (!search_daily(daily_path, by, key, &daily, &dcount) || dcount == 0)
        {
            printf("\n=== ผลการค้นหา ===\n");
            printf("ไม่พบข้อมูลในไฟล์รายวันวันนี้\n");
            delay(3);
            free(daily);
            continue;
        }

        int pick_index = 0;
        if (dcount == 1)
        {
            pick_index = 0;
            printf("\n=== ผลการค้นหา ===\n");
            printf("พบรายการ 1 รายการของสมาชิกนี้\n");
            printf("---------------------------------------------------------------------------------------------\n");
            printf("\n   |  ID  |    ชื่อ-นามสกุล    |  ชื่อเล่น  | เพศ | ค่าคอร์ท | ค่าลูก | จำนวนลูกที่เล่น | รวมยอด | สถานะ |\n");
            printf("---------------------------------------------------------------------------------------------\n");
            printf("  |%4d  | %s | %s | %-4s | %5d  | %5d  | %5d      | %5d  | %s |\n",
                   daily[0].member_id,
                   daily[0].fullname,
                   daily[0].nickname,
                   daily[0].gender,
                   daily[0].court_fee,
                   prices.shuttle_price,
                   daily[0].shuttle_qty,
                   daily[0].amount_today,
                   daily[0].paid_today > 0 ? "ชำระแล้ว" : "ยังไม่ชำระ");
            printf("-----------------------------------------------------------------------------\n");
        }
        else
        {
            printf("\n=== ผลการค้นหา ===\n");
            printf("พบรายการ %d รายการ :\n", dcount); // พบหลายรายการ
            printf("---------------------------------------------------------------------------------------------\n");
            printf("\n   |  ID  |    ชื่อ-นามสกุล    |  ชื่อเล่น  | เพศ | ค่าคอร์ท | ค่าลูก | จำนวนลูกที่เล่น | รวมยอด | สถานะ |\n");
            printf("---------------------------------------------------------------------------------------------\n");
            for (int j = 0; j < dcount; j++)
            {
                printf("%d) |%4d  | %s | %s | %-4s | %5d  | %5d  | %5d      | %5d  | %s |\n",
                       j + 1,
                       daily[j].member_id,
                       daily[j].fullname,
                       daily[j].nickname,
                       daily[j].gender,
                       daily[j].court_fee,   // แสดงค่าสนามตรงนี้
                       prices.shuttle_price, //  แสดงราคาลูกที่โหลดมาจาก config.txt
                       daily[j].shuttle_qty,
                       daily[j].amount_today,
                       daily[j].paid_today > 0 ? "ชำระแล้ว" : "ยังไม่ชำระ");
            }
            printf("---------------------------------------------------------------------------------------------\n");
            int pick;
            printf("\nเลือกหมายเลข 1-%d ตามลำดับ (0 = ยกเลิก) : ", dcount); // ให้ผู้ใช้เลือก
            if (scanf("%d", &pick) != 1)
            {
                printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
                delete_buffle();
                free(daily);
                delay(3);
                continue;
            }
            if (pick == 0)
            {
                free(daily);
                continue;
            }
            if (pick < 1 || pick > dcount)
            {
                printf("หมายเลขไม่ถูกต้อง\n");
                free(daily);
                delay(3);
                continue;
            }
            pick_index = pick - 1;
        }

        DailyEntry d = daily[pick_index];
        free(daily);

        // Display and choose payment
        int act;
        printf("เลือกการชำระ 1 = เงินสด, 2 = โอน, 3 = ค้างจ่าย, 0 = ยกเลิก : ");
        if (scanf("%d", &act) != 1)
        {
            delete_buffle();
            printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
            delay(3);
            continue;
        }
        if (act == 0)
            continue;

        Member m;
        m.id = d.member_id;
        strncpy(m.fullname, d.fullname, NAME_MAXLEN);
        strncpy(m.nickname, d.nickname, NICK_MAXLEN);
        strncpy(m.gender, d.gender, GENDER_MAXLEN);

        if (act == 1)
        {
            // pay cash: set paid_today to full amount, check_update=0 (payment)
            int unpaid = d.amount_today - (d.paid_today + d.paid_os);
            if (unpaid < 0) unpaid = 0;
            if (!upsert_daily_entry(daily_path, &prices, &m, 0, 1, PAY_CASH, unpaid, 0, 0))
            {
                printf("อัปเดตการชำระเงินสดล้มเหลว\n");
                delay(3);
            }
            else
            {
                printf("บันทึกการชำระเงินสดเรียบร้อย\n");
                delay(3);
            }
        }
        else if (act == 2)
        {
            int unpaid = d.amount_today - (d.paid_today + d.paid_os);
            if (unpaid < 0) unpaid = 0;
            if (!upsert_daily_entry(daily_path, &prices, &m, 0, 1, PAY_TRANSFER, unpaid, 0, 0))
            {
                printf("อัปเดตการชำระแบบโอนล้มเหลว\n");
                delay(3);
            }
            else
            {
                printf("บันทึกการชำระแบบโอนเรียบร้อย\n");
                delay(3);
            }
        }
        else if (act == 3)
        {
            // เปลี่ยนสถานะเป็นค้างจ่าย (PAY_OS) — ไม่ถามจำนวนเงินที่ชำระจากค้างที่นี่อีกต่อไป
            if (!upsert_daily_entry(daily_path, &prices, &m, 0, 1, PAY_OS, 0, 0, 0))
            {
                printf("อัปเดตสถานะค้างจ่ายล้มเหลว\n");
                delay(3);
            }
            else
            {
                // คำนวณยอดค้างและเพิ่ม/อัปเดตใน OSpayment.txt
                char date_only[DATE_MAXLEN] = "";
                const char *bn = strrchr(daily_path, '/');
                const char *bp = strrchr(daily_path, '\\');
                const char *base = bn ? bn + 1 : (bp ? bp + 1 : daily_path);
                strncpy(date_only, base, sizeof(date_only) - 1);
                char *dot = strrchr(date_only, '.');
                if (dot)
                    *dot = '\0';

                // note "ค้างจ่าย"
                if (!upsert_os_entry("input/OSpayment.txt", &m, date_only, d.amount_today, "ค้างจ่าย"))
                {
                    printf("เพิ่มรายการค้างใน OSpayment.txt ล้มเหลว\n");
                }
                else
                {
                    printf("บันทึกสถานะค้างจ่ายเรียบร้อย (เพิ่มใน OSpayment: %d บาท)\n", d.amount_today);
                }

                delay(3);
            }
        }
        else
        {
            printf("เมนูผิดพลาด\n");
            delay(3);
        }
    }
}

static void sub3(const char *exclude_date)
{
    while (1)
    {
        int mode;
        char key[NAME_MAXLEN];
        Prices prices;
        if (!load_prices("input/config.txt", &prices))
        {
            // default if not present
            prices.shuttle_price = 100;
            prices.court_fee_per_person = 100;
        }

        system("cls");
        printf("\n=== ค้นหายอดค้างชำระ ===\n");
        printf("วิธีค้นหา 1 = id, 2 = ชื่อเล่น, 3 = ชื่อ-นามสกุล 0 = ยกเลิก : ");
        if (scanf("%d", &mode) != 1)
        {
            delete_buffle();
            printf("เมนูค้นหาไม่ถูกต้อง\n");
            delay(3);
            continue;
        }
        if (mode == 0)
            return;

        SearchBy by;
        if (mode == 1)
            by = BY_ID;
        else if (mode == 2)
            by = BY_NICKNAME;
        else if (mode == 3)
            by = BY_FULLNAME;
        else
        {
            printf("เมนูค้นหาไม่ถูกต้อง\n");
            delay(3);
            continue;
        }

        printf("กรอกคำค้นหา (0 = ยกเลิก) : ");
        getchar();
        fgets(key, sizeof(key), stdin);
        key[strcspn(key, "\n")] = '\0';
        trim(key);

        if (strcmp(key, "0") == 0) // กรอก 0 ออก
            continue;

        OSEntry *os_arr = NULL; // pointer to struct ที่ยังไม่ได้ define ค่า
        int os_cnt = 0;
        if (!search_os("input/OSpayment.txt", by, key, &os_arr, &os_cnt) || os_cnt == 0) // ได้ค่ากลับมาเก็บใน os_arr -- ข้อมูลรายการ และ os_cnt จำนวนรายการ
        {
            printf("ไม่พบยอดค้างของสมาชิกนี้\n");
            delay(3);
            free(os_arr);
            continue;
        }

        // filter out entries that match exclude_date (i.e., the currently opened daily file)
        OSEntry *filtered = malloc(sizeof(OSEntry) * os_cnt);
        int fcnt = 0;
        for (int i = 0; i < os_cnt; i++)
        {
            if (exclude_date && strcmp(os_arr[i].date, exclude_date) == 0)
                continue;
            filtered[fcnt++] = os_arr[i];
        }
        free(os_arr);

        if (fcnt == 0)
        {
            printf("ไม่พบยอดค้างของสมาชิกนี้ (หลังกรองตามวันที่เปิดอยู่)\n");
            delay(3);
            free(filtered);
            continue;
        }

        printf("\n=== ผลการค้นหา ===\n");
        printf("พบยอดค้าง %d รายการ:\n", fcnt);
        printf("--------------------------------------------------------------------\n");
        printf("\n   |  ID  |    ชื่อ-นามสกุล    |  ชื่อเล่น  | เพศ |   วันที่ค้าง   |  ยอด  |\n");
        printf("--------------------------------------------------------------------\n");

        for (int j = 0; j < fcnt; j++)
        {
            printf("%d) |%4d | %s | %s | %-4s | %-10s | %-3d  |\n",
                   j + 1,
                   filtered[j].member_id,
                   filtered[j].fullname,
                   filtered[j].nickname,
                   filtered[j].gender,
                   filtered[j].date,
                   filtered[j].os_amount);
        }

        int pick;
        printf("เลือกหมายเลข 1-%d ตามลำดับ (0 = ยกเลิก) : ", fcnt);
        if (scanf("%d", &pick) != 1)
        {
            delete_buffle();
            free(filtered);
            printf("หมายเลขไม่ถูกต้อง\n");
            delay(3);
            continue;
        }
        if (pick == 0)
        {
            free(filtered);
            continue;
        }
        if (pick < 1 || (int)pick > fcnt)
        {
            printf("หมายเลขไม่ถูกต้อง\n");
            free(filtered);
            delay(3);
            continue;
        }

        OSEntry chosen = filtered[pick - 1]; // เลือก 1 index

        int act;
        printf("ชำระหนี้ค้างนี้หรือไม่? 1 = เงินสด, 2 = โอน, 0 = ยกเลิก : ");
        if (scanf("%d", &act) != 1)
        {
            delete_buffle();
            printf("หมายเลขไม่ถูกต้อง\n");
            delay(3);
            free(filtered);
            continue;
        }
        if (act == 0 || act > 2 || act < 1)
        {
            printf("หมายเลขไม่ถูกต้อง\n");
            free(filtered);
            delay(3);
            continue;
        }

        // ask how much they pay now from OS (moved from sub2 logic)
        int pay_amt = 0;
        printf("จำนวนเงินที่ชำระจากค้าง (0 = ยกเลิก) : ");
        if (scanf("%d", &pay_amt) != 1)
        {
            delete_buffle();
            printf("กรอกจำนวนเงินไม่ถูกต้อง\n");
            delay(2);
            free(filtered);
            continue;
        }
        if (pay_amt <= 0)
        {
            free(filtered);
            continue;
        }

        if (pay_amt > chosen.os_amount)
            pay_amt = chosen.os_amount; // cap to outstanding

        // prepare member struct for upsert on original daily file
        Member m;
        m.id = chosen.member_id;
        strncpy(m.fullname, chosen.fullname, NAME_MAXLEN);
        strncpy(m.nickname, chosen.nickname, NICK_MAXLEN);
        strncpy(m.gender, chosen.gender, GENDER_MAXLEN);

        // build daily path for the original due date
        char orig_daily_path[256];
        snprintf(orig_daily_path, sizeof(orig_daily_path), "input/Daily data/%s.txt", chosen.date);

        // perform daily update: add paid_today by the amount paid; method is cash/transfer accordingly
        PayMethod method = act == 1 ? PAY_CASH : PAY_TRANSFER;
        if (!upsert_daily_entry(orig_daily_path, &prices, &m, 0, 1, method, 0, pay_amt, 0))
        {
            printf("อัปเดตการชำระในไฟล์รายวันไม่สำเร็จ\n");
            delay(3);
            free(filtered);
            continue;
        }
        else
        {
            printf("อัปเดตการชำระ %d บาท เรียบร้อย\n", pay_amt);
            delay(3);
        }

        free(filtered);
    }
}

static void sub5(Prices *prices)
{
    while (1)
    {
        system("cls");
        printf("ราคาลูกแบดปัจจุบัน : %d\n", prices->shuttle_price);
        printf("กรอกราคาลูกใหม่ (0 = ยกเลิก) : ");
        int p;
        if (scanf("%d", &p) != 1)
        {
            delete_buffle();
            printf("ค่าไม่ถูกต้อง โปรดลองใหม่อีกครั้ง\n");
            delay(3);
            continue;
        }
        if (p == 0)
            return;
        prices->shuttle_price = p;
        save_prices("input/config.txt", prices);
        printf("บันทึกราคาลูกใหม่แล้ว\n");
        printf("\nพิมพ์ 0 เพื่อย้อนกลับ : "); // พิมพ์อะไรก็ได้
        int _tmp;
        scanf("%d", &_tmp);
        return;
    }
}

static void sub6(Prices *prices)
{
    /* -------- เมนู 6: เปลี่ยนค่าสนาม -------- */
    while (1)
    {
        system("cls");
        printf("ค่าสนามต่อคนปัจจุบัน: %d\n", prices->court_fee_per_person);
        printf("กรอกค่าสนามต่อคนใหม่ (0 = ยกเลิก) : ");
        int p;
        if (scanf("%d", &p) != 1)
        {
            delete_buffle();
            continue;
        }
        if (p == 0)
            return;
        prices->court_fee_per_person = p;
        save_prices("input/config.txt", prices);
        printf("บันทึกค่าสนามใหม่แล้ว\n");
        printf("\nพิมพ์ 0 เพื่อย้อนกลับ : "); // พิมพ์อะไรก็ได้
        int _tmp;
        scanf("%d", &_tmp);
        return;
    }
}

void menu2_choose()
{
    char date[DATE_MAXLEN]; // df ไว้ใน menu1.c
    char daily_path[256];   // วัน-เดือน-ปี -> ขยาย buffer เพื่อรองรับ path "input/Daily data/<DD-MM-YYYY>.txt"
    Prices prices;          // struct ที่ประกาศไว้ใน menu2.h
    int sub;

    printf("\n=== เมนูเปิดก๊วน / จัดการรายวัน ===\n");

    /* Loop until a valid DD-MM-YYYY is entered or the user enters 0 to cancel */
    while (1)
    {
        printf("กรอกวันที่ (DD-MM-YYYY) (0 = ย้อนกลับ) : ");
        if (scanf("%11s", date) != 1)
        {
            delete_buffle();
            printf("ข้อมูลไม่ถูกต้อง ยกเลิกการลงทะเบียน\n");
            delay(3);
            return;
        }

        if (strcmp(date, "0") == 0) // 0 ย้อนกลับ
            return;

        if (!validate_date_ddmmyyyy(date))
        {
            printf("รูปแบบวันที่ต้องเป็น DD-MM-YYYY เช่น 05-08-2025\n");
            delay(2);
            continue;
        }
        break;
    }

    snprintf(daily_path, sizeof(daily_path), "input/Daily data/%s.txt", date); // ต่อ - รวม string

    if (!open_or_create_daily(daily_path)) // ถ้าเปิดไฟล์ไม่ได้ -- เรียกฟังก์ชันจาก menu2.h
    {
        return;
    }

    // โหลดราคาเริ่มต้นเมื่อตั้งเมนู (จะถูกใช้โดย sub5/sub6)
    if (!load_prices("input/config.txt", &prices))
    {
        printf("ไม่พบ config.txt ในโฟลเดอร์ \"input\" จะใช้ค่าเริ่มต้น\n");
        prices.shuttle_price = 100;
        prices.court_fee_per_person = 100;
    }

    while (1)
    {
        system("cls");
        printf("\n=== เมนูจัดการวันที่ %s ===\n", date);
        printf("1. ลงชื่อเบิกลูก\n");
        printf("2. สรุปยอดที่ต้องชำระ (รายบุคคล)\n");
        printf("3. ค้นหายอดค้างชำระ\n");
        printf("4. สรุปข้อมูลรายวัน\n");
        printf("5. เปลี่ยนราคาลูกแบด\n");
        printf("6. เปลี่ยนค่าสนามต่อคน\n");
        printf("0. ย้อนกลับ\n");
        printf("เลือกเมนู : ");

        if (scanf("%d", &sub) != 1)
        {
            delete_buffle();
            printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
            delay(3);
            continue;
        }

        if (sub == 0)
        {
            return; // ย้อนกลับไป main.c
        }

        if (sub == 1)
        {
            /* -------- เมนู 1: ลงชื่อเบิกลูก -------- */
            sub1(daily_path);
        }
        else if (sub == 2)
        {
            /* -------- เมนู 2: สรุปยอดที่ต้องชำระ (รายบุคคล) -------- */
            sub2(daily_path);
        }
        else if (sub == 3)
        {
            /* -------- เมนู 3: ค้นหายอดค้างชำระ -------- */
            sub3(date);
        }
        else if (sub == 4)
        {
            // build ascii output filenames to avoid encoding problems
            char out_full[160], out_brief[160];
            snprintf(out_full, sizeof(out_full), "output/%s-Full-Summary.txt", date);
            snprintf(out_brief, sizeof(out_brief), "output/%s-Brief-Summary.txt", date);

            // ensure output folder exists (Windows). Hide errors
            system("mkdir output 2>nul");

            /* -------- เมนู 4: สรุปข้อมูลรายวัน -------- */
            while (1)
            {
                system("cls");
                printf("\n=== เมนูสรุปข้อมูลรายวัน ===\n");
                printf("1. สรุปแบบมีรายชื่อ\n");
                printf("2. สรุปแบบย่อ\n");
                printf("0. ย้อนกลับ\n");
                printf("เลือก : ");
                int mode = 0;
                if (scanf("%d", &mode) != 1)
                {
                    delete_buffle();
                    printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
                    delay(2);
                    continue;
                }
                printf("\n\n");
                if (mode == 0)
                    break;

                if (mode == 1)
                {
                        if (!summarize_daily_and_write(daily_path, out_full, 1))
                        {
                            printf("บันทึกไฟล์สรุปแบบละเอียดล้มเหลว\n");
                            delay(3);
                        }
                        else
                        {
                            print_file(out_full);
                        }
                }
                else if (mode == 2)
                {
                        if (!summarize_daily_and_write(daily_path, out_brief, 0))
                        {
                            printf("บันทึกไฟล์สรุปแบบย่อล้มเหลว\n");
                            delay(3);
                        }
                        else
                        {
                            print_file(out_brief);
                        }
                }
            }
        }
        else if (sub == 5)
        {
            /* -------- เมนู 5: เปลี่ยนราคาลูก -------- */
            sub5(&prices);
        }
        else if (sub == 6)
        {
            /* -------- เมนู 6: เปลี่ยนค่าสนาม -------- */
            sub6(&prices);
        }
        else
        {
            printf("เมนูไม่ถูกต้อง โปรดลองอีกครั้ง\n");
            delay(3);
        }
    }
}
