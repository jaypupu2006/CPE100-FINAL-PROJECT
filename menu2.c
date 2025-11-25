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
 * - ตรวจสอบ/สร้างไฟล์รายวัน, โหลดค่าราคา (จาก config.txt) และแสดงเมนูย่อย
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

        printf("จำนวนผู้เล่นที่จะลงชื่อหารค่าลูก มากที่สุด 4 คน (0 = ยกเลิก) : ");
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
            continue;
        if (count_player > 0 && count_player <= 4)
            break;
    }

    for (int i = 0; i < count_player; i++) // วนให้ครบจำนวนผู้เล่นที่หาร
    {
        while (1)
        {
            int mode;
            char key[NAME_MAXLEN];
            Member *member = NULL; // pointer arr ชี้ไปที่ struct ชื่อ Member ถูกประกาศใน menu2.h -- Define NULL ไว้ก่อน
            int member_count = 0;  // int ตัวแปรชนิดหนึ่งที่

            printf("\n[ผู้เล่นคนที่ %d]\n", i + 1);
            printf("วิธีค้นหา 1 = id, 2 = ชื่อเล่น, 3 = ชื่อ-นามสกุล, 0 = ยกเลิก : ");

            if (scanf("%d", &mode) != 1) // ไม่ใช่ตัวเลข
            {
                printf("กรุณากรอกตัวเลขเท่านั้น!\n");
                delete_buffle();
                continue;
            }

            if (mode == 0)
                return; // ยกเลิกเมนู

            if (mode < 0 || mode > 3) // ไม่มีเมนูเลขนี้
            {
                printf("ตัวเลือกไม่ถูกต้อง โปรดลองใหม่!\n");
                continue;
            }

            SearchBy by; // enum ที่ประกาศไว้ เป็นได้แค่ตัวเลข และค่าที่กำหนดไว้เท่านั้น
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

            if (!upsert_daily_entry(daily_path, &prices, &select, add_qty, count_player, PAY_OS, 0, 0)) // PAY_OS คือ enum = 3
            {
                printf("บันทึกข้อมูลรายวันล้มเหลว\n");
            }
            else
            {
                printf("บันทึกข้อมูลเรียบร้อย: %s (%s) +%d ลูก\n", select.fullname, select.nickname, add_qty);
            }
            break; // ออกจากลูป while (1) ของผู้เล่นคนนี้
        }
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
            by = BY_ID; // enum ถูกประกาศไว้แล้วใน menu2.h
        else if (mode == 2)
            by = BY_NICKNAME;
        else if (mode == 3)
            by = BY_FULLNAME;
        else
        {
            printf("เมนูค้นหาไม่ถูกต้อง\n");
            continue;
        }

        printf("กรอกคำค้น (0 = ยกเลิก) : ");
        delete_buffle();
        fgets(key, sizeof(key), stdin);
        key[strcspn(key, "\n")] = '\0';
        trim(key);
        if (strcmp(key, "0") == 0) // กรอก 0 ออก
            continue;

        DailyEntry *daily = NULL;
        int dcount = 0;
        if (!search_daily(daily_path, by, key, &daily, &dcount) || dcount == 0) // ฟังก์ชันหาข้อมูล ข้อมูลที่พบจะส่งกลับมาใน daily struct
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
        }
        else
        {
            printf("\n=== ผลการค้นหา ===\n");
            printf("พบรายการ %d รายการ :\n", dcount); // พบหลายรายการ
            printf("-----------------------------------------------------------------------------\n");
            printf("\n   |  ID  |    ชื่อ-นามสกุล    |  ชื่อเล่น  | เพศ | ค่าคอร์ท | จำนวนลูกที่เล่น | รวมยอด |\n");
            printf("-----------------------------------------------------------------------------\n");
            for (int j = 0; j < dcount; j++)
            {
                printf("%d) |%4d  | %s | %s | %-4s | %5d  | %5d      | %5d  |\n", // แสดงทีละรายการ
                       j + 1,
                       daily[j].member_id,
                       daily[j].fullname,
                       daily[j].nickname,
                       daily[j].gender,
                       prices.shuttle_price,
                       daily[j].shuttle_qty,
                       daily[j].amount_today);
            }
            printf("-----------------------------------------------------------------------------\n");
            int pick;
            printf("\nเลือกหมายเลข 1-%d ตามลำดับ (0 = ยกเลิก): ", dcount); // ให้ผู้ใช้เลือก
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
            if (pick < 1 || (int)pick > dcount)
            {
                printf("หมายเลขไม่ถูกต้อง\n");
                free(daily);
                delay(3);
                continue;
            }
            pick_index = pick - 1;
        }

        DailyEntry d = daily[pick_index];
        free(daily); // free ทิ้ง

        printf("\n=== ข้อมูลวันนี้ของสมาชิก ===\n");
        printf("-----------------------------------------------------------------------------\n");
        printf("\n |  ID  |    ชื่อ-นามสกุล    |  ชื่อเล่น  | เพศ | ค่าคอร์ท | จำนวนลูกที่เล่น | รวมยอด |\n");
        printf("-----------------------------------------------------------------------------\n");
        // แสดงทีละรายการ
        printf(" |%4d  | %s | %s | %-4s | %5d  | %5d      | %5d  |\n",
               d.member_id,
               d.fullname,
               d.nickname,
               d.gender,
               prices.shuttle_price,
               d.shuttle_qty,
               d.amount_today);
        printf("-----------------------------------------------------------------------------\n");

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

        Member m; // ประกาศ struct เพราะต้องส่งค่าเข้า function upsert_daily_entry
        m.id = d.member_id;
        strncpy(m.fullname, d.fullname, NAME_MAXLEN);
        strncpy(m.nickname, d.nickname, NICK_MAXLEN);
        strncpy(m.gender, d.gender, GENDER_MAXLEN);

        //  ------ อัพเดทการชำระเงิน -------
        if (act == 1)
        {
            if (!upsert_daily_entry(daily_path, &prices, &m, 0, 1, PAY_CASH, d.amount_today, 0))
                printf("อัปเดตการชำระเงินสดล้มเหลว\n");
            else
                printf("บันทึกการชำระเงินสดเรียบร้อย\n");
        }
        else if (act == 2)
        {
            if (!upsert_daily_entry(daily_path, &prices, &m, 0, 1, PAY_TRANSFER, d.amount_today, 0))
                printf("อัปเดตการชำระแบบโอนล้มเหลว\n");
            else
                printf("บันทึกการชำระแบบโอนเรียบร้อย\n");
        }
        else if (act == 3)
        {
            if (!upsert_daily_entry(daily_path, &prices, &m, 0, 1, PAY_OS, 0, 0))
                printf("อัปเดตรายวันสำหรับค้างจ่ายล้มเหลว\n");
            else
                printf("เปลี่ยนสถานะเป็นค้างจ่ายสำเร็จ\n");
        }
        else
        {
            printf("เมนูผิดพลาด\n");
            delay(3);
        }
    }
}

static void sub3()
{
    while (1)
    {
        int mode;
        char key[NAME_MAXLEN];
        system("cls");
        printf("\n=== ค้นหายอดค้างชำระ ===\n");
        printf("วิธีค้นหา 1 = id, 2 = ชื่อเล่น, 3 = ชื่อ-นามสกุล 0 = ยกเลิก : ");
        if (scanf("%d", &mode) != 1)
        {
            delete_buffle();
            continue;
        }
        if (mode == 0)
            continue;

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

        printf("\n=== ผลการค้นหา ===\n");
        printf("พบยอดค้าง %d รายการ:\n", os_cnt);
        printf("--------------------------------------------------------------------\n");
        printf("\n   |  ID  |    ชื่อ-นามสกุล    |  ชื่อเล่น  | เพศ |   วันที่ค้าง   |  ยอด  |\n");
        printf("--------------------------------------------------------------------\n");

        for (int j = 0; j < os_cnt; j++)
        {
            printf("%d) |%4d | %s | %s | %-4s | %-10s | %-3d  |\n",
                   j + 1,
                   os_arr[j].member_id,
                   os_arr[j].fullname,
                   os_arr[j].nickname,
                   os_arr[j].gender,
                   os_arr[j].date,
                   os_arr[j].os_amount);
        }

        int pick;
        printf("เลือกหมายเลข 1-%d ตามลำดับ (0 = ยกเลิก) : ", os_cnt);
        if (scanf("%d", &pick) != 1)
        {
            delete_buffle();
            free(os_arr);
            printf("หมายเลขไม่ถูกต้อง\n");
            delay(3);
            continue;
        }
        if (pick == 0)
        {
            free(os_arr);
            continue;
        }
        if (pick < 1 || (int)pick > os_cnt)
        {
            printf("หมายเลขไม่ถูกต้อง\n");
            free(os_arr);
            delay(3);
            continue;
        }

        OSEntry chosen = os_arr[pick - 1]; // เลือก 1 index
        free(os_arr);

        int act;
        printf("ชำระหนี้ค้างนี้หรือไม่? 1=เงินสด 2=โอน 0=ยกเลิก: ");
        if (scanf("%d", &act) != 1)
        {
            delete_buffle();
            printf("หมายเลขไม่ถูกต้อง\n");
            delay(3);
            continue;
        }
        if (act == 0)
            continue;

        if (!remove_os_entry("input/OSpayment.txt", &chosen)) // ลบยอดค้าง ส่ง OSEntry struct ไปด้วย
        {
            printf("ลบยอดค้างไม่สำเร็จ\n");
        }
        else
        {
            printf("ชำระหนี้ค้างเรียบร้อย และลบออกจากค้างจ่ายแล้ว\n");
        }
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
    char daily_path[256];    // วัน-เดือน-ปี -> ขยาย buffer เพื่อรองรับ path "input/Daily data/<DD-MM-YYYY>.txt"
    Prices prices;          // struct ที่ประกาศไว้ใน menu2.h
    int sub;

    printf("\n=== เมนูเปิดก๊วน / จัดการรายวัน ===\n");
    printf("กรอกวันที่ (DD-MM-YYYY) (0 = ย้อนกลับ): ");
    if (scanf("%11s", date) != 1)
    {
        delete_buffle();
        printf("ข้อมูลไม่ถูกต้อง ยกเลิกการลงทะเบียน\n");
        delay(3);
        return;
    }

    if (strcmp(date, "0") == 0) // 0 ย้อนกลับ
        return;

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
        printf("4. สรุปข้อมูลรายวัน (แสดงบนหน้าจอ)\n");
        printf("5. เปลี่ยนราคาลูกแบด\n");
        printf("6. เปลี่ยนค่าสนามต่อคน\n");
        printf("0. ย้อนกลับ\n");
        printf("เลือกเมนู: ");

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
            sub3();
        }
        else if (sub == 4)
        {
            /* -------- เมนู 4: สรุปข้อมูลรายวันบนหน้าจอ -------- */
            summarize_daily(daily_path, 1);
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
