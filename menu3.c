#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Library/menu1.h"
#include "Library/menu2.h"
#include "Library/menu3.h"
#include "Library/delay.h"

/*
 * file_exists: ช่วยตรวจว่าไฟล์ที่กำหนดมีอยู่หรือไม่
 * - พารามิเตอร์: `path` เส้นทางไฟล์ที่จะตรวจ
 * - ผลลัพธ์: คืนค่า 1 เมื่อไฟล์เปิดได้ (มีอยู่), 0 เมื่อไม่พบหรือเปิดไม่ได้
 */
static int file_exists(const char *path)
{ // เช็คว่าเปิด path ได้ไหมใน Output
    FILE *fp = fopen(path, "r");
    if (!fp)
        return 0;
    fclose(fp);
    return 1;
}

static int summarize_month_and_write(const char *month_yyyy, const char *out_path)
{
    char path[256]; // เพิ่ม buffer ให้ใหญ่พอ "input/Daily data/<DD-MM-YYYY>.txt"
    int day, total_days = 0;
    int total_players = 0, total_shuttle = 0, max_shuttle = 0, min_shuttle = 1 << 30;
    int total_income = 0, paid_cash = 0, paid_transfer = 0, paid_from_os = 0, court_fee_total = 0, court_fee_pp_last = 0;

    for (day = 1; day <= 31; day++)
    {
        char dd[3];
        snprintf(dd, sizeof(dd), "%02d", day);
        // path should match menu2 file path
        snprintf(path, sizeof(path), "input/Daily data/%s-%s.txt", dd, month_yyyy);

        FILE *fp = fopen(path, "r");
        if (!fp)
            continue;
        total_days++;
        char line[512];
        while (fgets(line, sizeof(line), fp))
        {
            if (!isdigit((unsigned char)line[0]))
                continue;
            int member_id, shuttle_qty, court_fee, amount_today, paid_today, paid_os;
            char fullname[NAME_MAXLEN], nickname[NICK_MAXLEN];
            char gender[GENDER_MAXLEN];
            char method_today[method_today_MAXLEN];
            if (sscanf(line, "%d|%127[^|]|%63[^|]|%31[^|]|%d|%d|%d|%d|%d|%31[^\n]",
                       &member_id, fullname, nickname, gender,
                       &shuttle_qty, &court_fee, &amount_today,
                       &paid_today, &paid_os, method_today) == 10)
            {
                total_players++;
                total_shuttle += shuttle_qty;
                if (shuttle_qty > max_shuttle)
                    max_shuttle = shuttle_qty;
                if (shuttle_qty < min_shuttle)
                    min_shuttle = shuttle_qty;
                total_income += amount_today;
                court_fee_pp_last = court_fee;
                court_fee_total += court_fee;
                if (strcmp(method_today, "เงินสด") == 0)
                    paid_cash += paid_today;
                else if (strcmp(method_today, "โอน") == 0)
                    paid_transfer += paid_today;
                // เก็บยอดที่ชำระจากการตัดค้าง
                if (paid_os > 0)
                    paid_from_os += paid_os;
            }
        }
        fclose(fp);
    }

    int outstanding_this_month = 0;
    FILE *fos = fopen("input/OSpayment.txt", "r"); // ใช้ path เดิมในโฟลเดอร์ input
    if (fos)
    {
        char line[512];
        while (fgets(line, sizeof(line), fos))
        {
            if (!isdigit((unsigned char)line[0]))
                continue;
            int member_id, os_amount;
            char fullname[NAME_MAXLEN], nickname[NICK_MAXLEN], date[DATE_MAXLEN], note[NOTE_MAXLEN], gender[GENDER_MAXLEN];
            if (sscanf(line, "%d|%127[^|]|%63[^|]|%31[^|]|%15[^|]|%d|%127[^\n]",
                       &member_id, fullname, nickname, gender, date, &os_amount, note) == 7)
            {
                if (strlen(date) >= 7)
                {
                    if (strncmp(date + 3, month_yyyy, strlen(month_yyyy)) == 0)
                        outstanding_this_month += os_amount;
                }
            }
        }
        fclose(fos);
    }

    FILE *fo = fopen(out_path, "w");
    if (!fo)
    {
        printf("ไม่สามารถเขียนไฟล์สรุปรายเดือน: %s\n", out_path);
        delay(3);
        return 0;
    }

    fwrite("\xEF\xBB\xBF", 1, 3, fo);

    fprintf(fo, "เดือน/ปี|%s\n", month_yyyy);
    fprintf(fo, "จำนวนวันที่มีข้อมูล|%d\n", total_days);
    fprintf(fo, "จำนวนลูกทั้งหมด (เดือน)|%d\n", total_shuttle);
    fprintf(fo, "ค่าเฉลี่ยลูก/คน (เดือน)|%d\n", total_players ? total_shuttle / total_players : 0);
    fprintf(fo, "ลูกสูงสุด/วัน|%d\n", max_shuttle);
    fprintf(fo, "ลูกต่ำสุด/วัน|%d\n", (min_shuttle == 1 << 30) ? 0 : min_shuttle);
    fprintf(fo, "จำนวนผู้เล่นรวม|%d\n", total_players);
    fprintf(fo, "ค่าสนามต่อคน (อ้างอิงล่าสุด)|%d\n", court_fee_pp_last);
    fprintf(fo, "ค่าสนามรวมทั้งเดือน|%d\n", court_fee_total);
    fprintf(fo, "ยอดที่ต้องได้รับทั้งเดือน|%d\n", total_income);
    fprintf(fo, "รับจริง (เงินสด)|%d\n", paid_cash);
    fprintf(fo, "รับจริง (โอน)|%d\n", paid_transfer);
    fprintf(fo, "รับจากชำระค้าง|%d\n", paid_from_os);
    fprintf(fo, "รวมรับจริงทั้งหมด|%d\n", paid_cash + paid_transfer + paid_from_os);
    fprintf(fo, "ยอดค้างจ่ายในเดือนนี้|%d\n", outstanding_this_month);
    fclose(fo);
    return 1;
}

/*
 * menu3_choose: เมนูสรุปข้อมูล (รายวัน/รายเดือน)
 * - ไม่มีพารามิเตอร์
 * - พฤติกรรม: ผู้ใช้เลือกว่าจะสรุปรายวันหรือรายเดือน,
 *   โปรแกรมจะแสดงไฟล์สรุปถ้ามี, หรือสร้างแล้วพิมพ์ออกหน้าจอ
 */
void menu3_choose(void)
{
    while (1)
    {
        int sub;
        system("cls");
        printf("=== เมนูสรุปข้อมูล ===\n");
        printf("1. สรุปข้อมูลรายวัน\n");
        printf("2. สรุปข้อมูลรายเดือน\n");
        printf("0. ย้อนกลับ\n");
        printf("เลือก: ");
        if (scanf("%d", &sub) != 1)
        {
            delete_buffle();
            printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
            delay(3);
            continue;
        }
        if (sub == 0)
            return;

        if (sub == 1)
        {
            int mode;
            char date[DATE_MAXLEN], inpath[256], out_full[160], out_brief[160];

            printf("กรอกวันที่ (DD-MM-YYYY) (0 = ย้อนกลับ) : ");
            if (scanf("%15s", date) != 1)
            {
                delete_buffle();
                printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
                delay(2);
                continue;
            }
            if (strcmp(date, "0") == 0)
                continue;

            // ปรับ inpath ให้ตรงกับไฟล์รายวันของโปรเจกต์
            snprintf(inpath, sizeof(inpath), "input/Daily data/%s.txt", date);
            snprintf(out_full, sizeof(out_full), "output/%s.สรุปแบบละเอียด.txt", date);
            snprintf(out_brief, sizeof(out_brief), "output/%s.สรุปแบบย่อ.txt", date);

            printf("1. สรุปแบบมีรายชื่อ\n");
            printf("2. สรุปแบบย่อ\n");
            printf("0. ย้อนกลับ\n");
            printf("เลือก : ");
            if (scanf("%d", &mode) != 1)
            {
                delete_buffle();
                printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
                delay(2);
                continue;
            }
            printf("\n\n");
            if (mode == 0)
                continue;

            if (mode == 1)
            {
                if (file_exists(inpath))
                {
                    if (!summarize_daily_and_write(inpath, out_full, 1))
                    {
                        printf("บันทึกไฟล์สรุปแบบละเอียดล้มเหลว\n");
                        delay(3);
                    }
                    else
                        print_file(out_full);
                }
                else
                {
                    printf("ไม่พบไฟล์รายวัน : %s\n", inpath);
                    delay(3);
                }
            }
            else if (mode == 2)
            {
                if (file_exists(inpath))
                {
                    if (!summarize_daily_and_write(inpath, out_brief, 0))
                    {
                        printf("บันทึกไฟล์สรุปแบบย่อล้มเหลว\n");
                        delay(3);
                    }
                    else
                        print_file(out_brief);
                }
                else
                {
                    printf("ไม่พบไฟล์รายวัน : %s\n", inpath);
                    delay(3);
                }
            }
            else
            {
                printf("เมนูไม่ถูกต้อง\n");
                delay(3);
            }
        }
        else if (sub == 2)
        {
            char month_yyyy[8], out_path[160];
            printf("กรอกเดือน/ปี (MM-YYYY) (0 = ย้อนกลับ) : ");
            if (scanf("%7s", month_yyyy) != 1)
            {
                delete_buffle();
                printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
                delay(2);
                continue;
            }
            printf("\n");
            if (strcmp(month_yyyy, "0") == 0)
                continue;

            snprintf(out_path, sizeof(out_path), "output/%s-Month-Summary.txt", month_yyyy);

            if (!summarize_month_and_write(month_yyyy, out_path))
            {
                printf("สรุปรายเดือนล้มเหลวหรือไม่พบข้อมูลไฟล์รายวัน (.txt)\n");
                delay(3);
            }
            else
            {
                print_file(out_path);
            }
        }
    }
}
