#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Library/menu2.h"
#include "Library/delay.h"

/*
 * dupstr: สำเนาสตริงโดยคืน pointer ที่ต้อง free เมื่อเลิกใช้
 * - พารามิเตอร์: `s` สตริงต้นฉบับ (ไม่เป็น NULL)
 * - คืนค่า: pointer ใหม่ ที่ต้องถูก free โดยผู้เรียกเมื่อไม่ใช้งาน
 */
static char *dupstr(const char *s) // คืนค่าเป็น pointer
{
    int n = strlen(s) + 1; // พื้นที่ byte รวม  '\0'
    char *p = malloc(n);   // จอง byte ขนาดเดียวกับข้อมูลที่ส่งมา
    if (p)                 // ถ้าจองสำเร็จ
        memcpy(p, s, n);   // คัดลอกข้อมูลจาก s มาเก็บใน p
    return p;
}

/* helper: trim both sides in local scope so we can sanitize keys read from config */
static void trim_str_local(char *s)
{
    char *start = s;
    while (*start && isspace((unsigned char)*start))
        start++;
    if (start != s)
        memmove(s, start, strlen(start) + 1);
    for (int i = (int)strlen(s) - 1; i >= 0; i--)
    {
        if (isspace((unsigned char)s[i]))
            s[i] = '\0';
        else
            break;
    }
}

/*
 * open_or_create_daily: ตรวจว่ามีไฟล์รายวันอยู่หรือไม่ ถ้าไม่มีให้สร้าง
 * - พารามิเตอร์: `daily_path` ชื่อไฟล์รายวัน (เช่น "DD-MM-YYYY.txt")
 * - คืนค่า: 1 เมื่อสำเร็จ (ไฟล์พร้อมใช้งาน) หรือ 0 เมื่อเกิดข้อผิดพลาด
 */
int open_or_create_daily(const char *daily_path) // เปิดหรือสร้างไฟล์ใหม่  const char ให้เป็นค่าคงที่ที่แก้ไขไม่ได้
{
    FILE *fp = fopen(daily_path, "r"); // เปิดไฟล์ path ที่ส่งมา
    if (fp)                            /// ถ้าเปิดได้
    {
        fclose(fp);
        return 1;
    }
    fp = fopen(daily_path, "w");
    if (!fp)
    {
        printf("ไม่สามารถเปิดหรือสร้างไฟล์รายวันได้ เปิดหาโฟล์เดอร์ \"input/Daily data\" ไม่เจอ\n");
        delay(3);
        return 0;
    }
    fclose(fp);
    return 1;
}

/*
 * load_prices: โหลดค่าราคา (`SHUTTLE_PRICE`, `COURT_FEE_PER_PERSON`) จากไฟล์ config
 * - พารามิเตอร์: `config_path` เส้นทางไฟล์ config, `out` โครงสร้างผลลัพธ์
 * - คืนค่า: 1 เมื่ออ่านและพบค่าทั้งสอง, 0 เมื่อล้มเหลว
 */
int load_prices(const char *config_path, Prices *out) //
{
    FILE *fp = fopen(config_path, "r");
    if (!fp)
    {
        printf("ไม่สามารถเปิดไฟล์ config.txt ได้ เปิดหาโฟล์เดอร์ \"input\" ไม่เจอ\n");
        delay(3);
        return 0;
    }
    char buf[256];
    int s = 0, c = 0;
    while (fgets(buf, sizeof(buf), fp))
    {
        char key[64];
        int val;
        /* akzept various space around = */
        if (sscanf(buf, " %63[^=]= %d", key, &val) != 2 &&
            sscanf(buf, " %63[^=] = %d", key, &val) != 2 &&
            sscanf(buf, " %63[^=]=%d", key, &val) != 2)
            continue;
        trim_str_local(key);
        if (strcmp(key, "SHUTTLE_PRICE") == 0)
        {
            out->shuttle_price = val;
            s = 1;
        }
        else if (strcmp(key, "COURT_FEE_PER_PERSON") == 0)
        {
            out->court_fee_per_person = val;
            c = 1;
        }
    }
    fclose(fp);
    return s && c; // ถ้ามีค่าใดค่าหนึ่งเป็น 0 จะเท็จ
}

/*
 * save_prices: เขียนค่าราคาไปที่ไฟล์ config
 * - พารามิเตอร์: `config_path` เส้นทางไฟล์ config, `in` ค่าราคาที่ต้องการบันทึก
 * - คืนค่า: 1 เมื่อเขียนสำเร็จ, 0 เมื่อไม่สามารถเปิดไฟล์ได้
 */
int save_prices(const char *config_path, const Prices *in)
{
    FILE *fp = fopen(config_path, "w");
    if (!fp)
    {
        printf("ไม่สามารถเปิดไฟล์ config.txt ได้ เปิดหาโฟล์เดอร์ \"input\" ไม่เจอ\n");
        delay(3);
        return 0;
    }

    fprintf(fp, "SHUTTLE_PRICE = %d\n", in->shuttle_price);
    fprintf(fp, "COURT_FEE_PER_PERSON = %d\n", in->court_fee_per_person);
    fclose(fp);
    return 1;
}

/*
 * search_members: ค้นหาสมาชิกจากไฟล์ `member.txt`
 * - พารามิเตอร์: `member_path` เส้นทางไฟล์สมาชิก,
 *   `by` วิธีค้นหา (BY_ID/BY_NICKNAME/BY_FULLNAME), `key` คำค้น,
 *   `out_arr` จะรับ pointer ไปยัง array ที่จองด้วย malloc (ต้อง free โดยผู้เรียก),
 *   `out_count` จำนวนรายการที่พบ
 * - คืนค่า: 1 เมื่อทำงานสำเร็จ (แม้ไม่พบก็คืน 1 แต่ `out_count` = 0), 0 เมื่อเกิดข้อผิดพลาด I/O
 */
int search_members(SearchBy by, const char *key, Member **out_arr, int *out_count)
{
    FILE *fp = fopen("input/member.txt", "r");
    if (fp == NULL)
    {
        printf("ไม่สามารถเปิด member.txt\n");
        delay(3);
        return 0;
    }
    Member *member = NULL; // NULL เพื่อให้สามารถใช้ realloc ข้างล่างได้
    int count = 0;
    char line[500];
    while (fgets(line, sizeof(line), fp))
    {
        Member m;
        if (sscanf(line, "%d|%127[^|]|%63[^|]|%31[^|]|%10[^\n]", &m.id, m.fullname, m.nickname, m.gender, m.created_at) == 5)
        {
            int match = 0;
            if (by == BY_ID && atoi(key) == m.id) // atoi = ASCII TO integer "123" to 123 -- stdlib.h
                match = 1;
            else if (by == BY_NICKNAME && strstr(m.nickname, key)) // strstr find sub string -- string.h
                match = 1;
            else if (by == BY_FULLNAME && strstr(m.fullname, key)) // strstr find sub string -- string.h
                match = 1;
            if (match)
            {
                Member *tmp = realloc(member, sizeof(Member) * (count + 1)); // pointer to struct = ขยายหน่วยความจำที่ member ชี้อยู่ เป็นขนาดที่ต้องการ realloc สามารถขยาย หรือ ลด ก็ได้ตามต้องการ
                if (!tmp)                                                    // ถ้าขยายหน่วยความจำไม่สำเร็จ
                {
                    fclose(fp);
                    free(member);
                    return 0;
                }
                member = tmp;        // หน่วยความจำ
                member[count++] = m; //
            }
        }
    }
    fclose(fp);
    *out_arr = member;
    *out_count = count;
    return 1;
}

/*
 * search_daily: ค้นหารายการในไฟล์รายวัน (daily file)
 * - พารามิเตอร์และการคืนค่าเหมือนกับ `search_members` แต่อ่านโครงข้อมูล `DailyEntry`
 */
int search_daily(const char *daily_path, SearchBy by, const char *key, DailyEntry **out_arr, int *out_count)
{
    FILE *fp = fopen(daily_path, "r");
    if (!fp)
        return 0;
    DailyEntry *arr = NULL;
    int count = 0;
    char line[512];
    while (fgets(line, sizeof(line), fp))
    {
        if (!isdigit((unsigned char)line[0]))
            continue;
        DailyEntry e;
        if (sscanf(line, "%d|%127[^|]|%63[^|]|%31[^|]|%d|%d|%d|%d|%d|%31[^\n]",
                   &e.member_id, e.fullname, e.nickname, e.gender,
                   &e.shuttle_qty, &e.court_fee, &e.amount_today,
                   &e.paid_today, &e.paid_os, e.method_today) == 10)
        {
            int match = 0;
            if (by == BY_ID && atoi(key) == e.member_id) // atoi = ASCII TO integer "123" to 123 -- stdlib.h
                match = 1;
            else if (by == BY_NICKNAME && strstr(e.nickname, key)) // strstr find sub string -- string.h
                match = 1;
            else if (by == BY_FULLNAME && strstr(e.fullname, key)) // strstr find sub string -- string.h
                match = 1;
            if (match)
            {
                DailyEntry *tmp = realloc(arr, sizeof(DailyEntry) * (count + 1));
                if (!tmp)
                {
                    fclose(fp);
                    free(arr);
                    return 0;
                }
                arr = tmp;
                arr[count++] = e; // e เป็น [pointer อยู่แล้ว = ได้เลย] ถ้า e ไม่ใช่ pointer ต้อง molloc จอง mem สำหรับ arr ก่อน
            }
        }
    }
    fclose(fp);
    *out_arr = arr;
    *out_count = count;
    return 1;
}

/*
 * search_os: ค้นหารายการค้างชำระจาก `OSpayment.txt`
 * - พารามิเตอร์และการคืนค่าเหมือนกับ `search_members` แต่อ่านโครงข้อมูล `OSEntry`
 */
int search_os(const char *os_path, SearchBy by, const char *key, OSEntry **out_arr, int *out_count)
{
    FILE *fp = fopen(os_path, "r"); // เปิดอ่าน
    if (!fp)
    {
        printf("ไม่พบไฟล์ OSpayment.txt ในโฟลเดอร์ \"input\"\n");
        delay(3);
        return 0;
    }

    OSEntry *arr = NULL; // pointer to strcut
    int count = 0;
    char line[512];
    while (fgets(line, sizeof(line), fp)) // อ่านค่าทีละบรรทัดในไฟล์
    {
        if (!isdigit((unsigned char)line[0])) // เช็ตว่าตัวแรกใช่ตัวแรกไหม ไม่ใช่ วนใหม่
            continue;
        OSEntry e;
        if (sscanf(line, "%d|%127[^|]|%63[^|]|%31[^|]|%10[^|]|%d|%127[^\n]",
                   &e.member_id, e.fullname, e.nickname, e.gender, e.date, &e.os_amount, e.note) == 7)
        {
            int match = 0;
            if (by == BY_ID && atoi(key) == e.member_id) // atoi = ASCII TO integer "123" to 123 -- stdlib.h
                match = 1;
            else if (by == BY_NICKNAME && strstr(e.nickname, key)) // strstr find sub string -- string.h
                match = 1;
            else if (by == BY_FULLNAME && strstr(e.fullname, key)) // strstr find sub string -- string.h
                match = 1;
            if (match)
            {
                OSEntry *tmp = realloc(arr, sizeof(OSEntry) * (count + 1)); // ขยาย mem
                if (!tmp)
                {
                    fclose(fp);
                    free(arr);
                    return 0;
                }
                arr = tmp;        // mem เท่ากัน
                arr[count++] = e; // ชี้ไปยัง e struct ที่อ่านมาได้ทั้งหมด
            }
        }
    }
    fclose(fp);
    *out_arr = arr;     // pointer ชี้ arr นำไปใช้ต่อ
    *out_count = count; // pointer ชี้ count นำไปใช้ต่อ
    return 1;
}

/* helper: ตรวจว่ามีรายการใน OSpayment.txt ของ member+date อยู่หรือไม่ */
static int os_entry_exists(const char *os_path, int member_id, const char *date_ddmmyyyy)
{
    FILE *fp = fopen(os_path, "r");
    if (!fp)
        return 0;
    char line[512];
    while (fgets(line, sizeof(line), fp))
    {
        if (!isdigit((unsigned char)line[0]))
            continue;
        int id, os_amount;
        char fullname[NAME_MAXLEN], nickname[NICK_MAXLEN], gender[GENDER_MAXLEN], date[DATE_MAXLEN], note[NOTE_MAXLEN];
        if (sscanf(line, "%d|%127[^|]|%63[^|]|%31[^|]|%10[^|]|%d|%127[^\n]",
                   &id, fullname, nickname, gender, date, &os_amount, note) == 7)
        {
            if (id == member_id && strcmp(date, date_ddmmyyyy) == 0)
            {
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

/*
 * upsert_daily_entry: เพิ่มหรืออัปเดตรายการของสมาชิกในไฟล์รายวัน
 * - พารามิเตอร์:
 *   `daily_path` - ไฟล์รายวัน, `prices` - ข้อมูลราคา, `m` - สมาชิกที่ต้องการอัปเดต,
 *   `add_shuttle_qty` - จำนวนลูกที่เพิ่ม (ถ้าเพิ่มเป็นรายการใหม่จะใช้ค่านี้),
 *   `method` - วิธีการชำระเงิน (PAY_NONE/PAY_CASH/PAY_TRANSFER/PAY_OS),
 *   `pay_today` - จำนวนเงินที่ชำระวันนี้ (ถ้ามี), `pay_os` - จำนวนที่ชำระจากค้าง (ถ้ามี),
 *   `check_update` - 1 แปลว่า "เพิ่ม/เบิก" (เพิ่มลูก/เปลี่ยนยอด) , 0 แปลว่า "อัปเดตการชำระ"
 * - พฤติกรรม: อ่านไฟล์ทั้งหมดมาไว้ในหน่วยความจำ, แก้บรรทัดที่ตรงกับ `m->id` หรือเพิ่มใหม่ถ้าไม่พบ
 * - คืนค่า: 1 เมื่อสำเร็จ, 0 เมื่อเกิดข้อผิดพลาด (I/O หรือหน่วยความจำ)
 */
int upsert_daily_entry(const char *daily_path, const Prices *prices, const Member *m, int add_shuttle_qty, int count_player, PayMethod method, int pay_today, int pay_os, int check_update)
{
    FILE *fp = fopen(daily_path, "r");
    if (!fp)
    {
        printf("ไม่สามารถเปิดหรือสร้างไฟล์รายวันได้\n");
        delay(3);
        return 0;
    }

    char **lines = NULL;
    int n = 0;
    char buf[512];
    while (fgets(buf, sizeof(buf), fp)) // อ่านข้อมูลทุกบรรทัด ในไฟล์ .txt
    {
        char **tmp = realloc(lines, sizeof(char *) * (n + 1));
        if (!tmp) { fclose(fp); for (int i = 0; i < n; i++) free(lines[i]); free(lines); return 0; }
        lines = tmp;
        lines[n] = malloc(strlen(buf) + 1);
        if (!lines[n]) { fclose(fp); for (int i = 0; i < n; i++) free(lines[i]); free(lines); return 0; }
        strcpy(lines[n++], buf);
    }
    fclose(fp);

    fp = fopen(daily_path, "w");
    if (!fp) { for (int i = 0; i < n; i++) free(lines[i]); free(lines); printf("ไม่สามารถเปิดไฟล์รายวันได้\n"); delay(3); return 0; }

    /* extract date from path for OSpayment sync */
    char date_only[DATE_MAXLEN] = "";
    {
        const char *bn = strrchr(daily_path, '/');
        const char *bp = strrchr(daily_path, '\\');
        const char *base = bn ? bn + 1 : (bp ? bp + 1 : daily_path);
        strncpy(date_only, base, sizeof(date_only) - 1);
        char *dot = strrchr(date_only, '.');
        if (dot) *dot = '\0';
    }

    int updated = 0;
    int last_member_id = 0;
    int last_unpaid = -1;

    for (int i = 0; i < n; i++)
    {
        DailyEntry e;
        if (isdigit((unsigned char)lines[i][0]) &&
            sscanf(lines[i], "%d|%127[^|]|%63[^|]|%31[^|]|%d|%d|%d|%d|%d|%31[^|]",
                   &e.member_id, e.fullname, e.nickname, e.gender,
                   &e.shuttle_qty, &e.court_fee, &e.amount_today,
                   &e.paid_today, &e.paid_os, e.method_today) == 10 &&
            e.member_id == m->id)
        {
            /* matched row: update according to method and check_update */
            if (!check_update)
            {
                /* payment update mode */
                if (method == PAY_CASH)
                {
                    // add pay_today to existing paid_today (support partial payments)
                    e.paid_today += pay_today;
                    strcpy(e.method_today, "เงินสด");
                }
                else if (method == PAY_TRANSFER)
                {
                    e.paid_today += pay_today;
                    strcpy(e.method_today, "โอน");
                }
                else if (method == PAY_OS)
                {
                    /* reset paid_today to 0 when switching to OS */
                    e.paid_today = 0;
                    strcpy(e.method_today, "ค้างจ่าย");
                }
            }
            else
            {
                /* add/update quantities (existing behavior) */
                int players = count_player > 0 ? count_player : 1;
                e.shuttle_qty += add_shuttle_qty;
                e.court_fee = prices->court_fee_per_person;
                int cal = prices->shuttle_price * add_shuttle_qty;
                e.amount_today += cal;
                e.paid_today;
                e.paid_os;
                if (method == PAY_CASH) strcpy(e.method_today, "เงินสด");
                else if (method == PAY_TRANSFER) strcpy(e.method_today, "โอน");
                else strcpy(e.method_today, "ค้างจ่าย");
            }

            /* apply explicit pay_today if cash/transfer provided (ensure not overwrite) already handled above */
            // remove overwrite line; we intentionally add above instead of overwrite

            /* write updated row */
            fprintf(fp, "%d|%s|%s|%s|%d|%d|%d|%d|%d|%s\n",
                    e.member_id, e.fullname, e.nickname, e.gender,
                    e.shuttle_qty, e.court_fee, e.amount_today,
                    e.paid_today, e.paid_os, e.method_today);

            updated = 1;
            last_member_id = e.member_id;
            last_unpaid = e.amount_today - (e.paid_today + e.paid_os);
        }
        else
        {
            /* preserve non-empty lines, skip blank whitespace-only lines */
            int k = 0;
            while (lines[i][k] && isspace((unsigned char)lines[i][k])) k++;
            if (lines[i][k]) fputs(lines[i], fp);
        }
        free(lines[i]);
    }
    free(lines);

    if (!updated)
    {
        /* insert new line: special-case PAY_OS: zero other fields except paid_os and method */
        DailyEntry e;
        int players = count_player > 0 ? count_player : 1;
        int cal = (prices->shuttle_price / (players ? players : 1)) * add_shuttle_qty;

        if (!check_update && method == PAY_OS)
        {
            e.paid_today = pay_today;
            e.paid_os = pay_os; 
            strcpy(e.method_today, "ค้างจ่าย");
        }
        else
        {
            e.shuttle_qty = add_shuttle_qty;
            e.court_fee = prices->court_fee_per_person;
            int cal = prices->shuttle_price * add_shuttle_qty;
            e.amount_today = cal + prices->court_fee_per_person;
            e.paid_today = (method == PAY_CASH || method == PAY_TRANSFER) ? pay_today : 0;
            e.paid_os = (method == PAY_OS) ? pay_os : 0;
            if (method == PAY_CASH) strcpy(e.method_today, "เงินสด");
            else if (method == PAY_TRANSFER) strcpy(e.method_today, "โอน");
            else strcpy(e.method_today, "ค้างจ่าย");
        }

        e.member_id = m->id;
        strncpy(e.fullname, m->fullname, NAME_MAXLEN);
        strncpy(e.nickname, m->nickname, NICK_MAXLEN);
        strncpy(e.gender, m->gender, GENDER_MAXLEN);

        fprintf(fp, "%d|%s|%s|%s|%d|%d|%d|%d|%d|%s\n",
                e.member_id, e.fullname, e.nickname, e.gender,
                e.shuttle_qty, e.court_fee, e.amount_today,
                e.paid_today, e.paid_os, e.method_today);

        last_member_id = e.member_id;
        last_unpaid = e.amount_today;
    }

    fclose(fp);

    /* maintain OSpayment: update if unpaid > 0, else remove */
    if (last_unpaid >= 0)
    {
        if (last_unpaid > 0)
        {
            upsert_os_entry("input/OSpayment.txt", m, date_only, last_unpaid, "ค้างจ่าย");
        }
        else
        {
            OSEntry rem;
            memset(&rem, 0, sizeof(rem));
            rem.member_id = last_member_id;
            strncpy(rem.date, date_only, DATE_MAXLEN);
            remove_os_entry("input/OSpayment.txt", &rem);
        }
    }

    return 1;
}

/*
 * append_os: เพิ่มรายการค้างชำระใหม่ลงในไฟล์ `OSpayment.txt`
 * - พารามิเตอร์: `os_path` ไฟล์ค้างชำระ, `m` สมาชิก, `date_ddmmyyyy` วันที่, `os_amount` ยอดค้าง, `note` หมายเหตุ
 * - คืนค่า: 1 เมื่อเขียนสำเร็จ, 0 เมื่อเปิดไฟล์ล้มเหลว
 */
int append_os(const char *os_path, const Member *m, const char *date_ddmmyyyy, int os_amount, const char *note)
{
    FILE *fp = fopen(os_path, "a");
    if (!fp)
        return 0;
    fprintf(fp, "%d|%s|%s|%s|%s|%d|%s\n", m->id, m->fullname, m->nickname, m->gender, date_ddmmyyyy, os_amount, note);
    fclose(fp);
    return 1;
}

/*
 * remove_os_entry: ลบรายการค้างชำระที่ตรงกับ `entry` (โดยเทียบ member_id และ date)
 * - พารามิเตอร์: `os_path` ไฟล์ค้างชำระ, `entry` ข้อมูลที่ต้องการลบ
 * - คืนค่า: 1 เมื่อสำเร็จ, 0 เมื่อไม่สามารถอ่านไฟล์ต้นทางได้
 */
int remove_os_entry(const char *os_path, const OSEntry *entry)
{
    FILE *fp = fopen(os_path, "r");
    if (!fp)
    {
        printf("ไม่พบไฟล์ OSpayment.txt ในโฟลเดอร์ \"input\"\n");
        delay(3);
        return 0;
    }
    char **lines = NULL;
    int n = 0;
    char buf[512];
    while (fgets(buf, sizeof(buf), fp)) // อ่านทีละบรรทัด
    {
        char **tmp = realloc(lines, sizeof(char *) * (n + 1)); // ขยายขนาดของ memory lines คล้าย ๆ เพิ่มขนาดของ i ใน arr[i][]
        if (!tmp)
        {
            fclose(fp);
            for (int i = 0; i < n; i++)
                free(lines[i]);
            free(lines);
            return 0;
        }

        lines = tmp;                        // ขนาดของ mem เป็นขนาดใหม่
        lines[n] = malloc(strlen(buf) + 1); // จองพื้นที่สำหรับเก็บ string ตามขนาดของ buf คล้าย ๆ เพิ่มขนาดของ j ใน arr[i][j]
        if (!lines[n])
        {
            fclose(fp);
            for (int i = 0; i < n; i++)
                free(lines[i]);
            free(lines);
            return 0;
        }

        strcpy(lines[n++], buf); // ก็อปข้อความจาก buf ไปเก็บใน lines[n] แล้วค่อย n++
    }
    fclose(fp);

    fp = fopen(os_path, "w");
    if (!fp)
    {
        for (int i = 0; i < n; i++)
            free(lines[i]);
        free(lines);
        printf("ไม่พบไฟล์ OSpayment.txt ในโฟลเดอร์ \"input\"\n");
        delay(3);
        return 0;
    }
    for (int i = 0; i < n; i++)
    {
        OSEntry e;
        int keep = 1;
        /*
            เงื่อนไข่ if
            - ตัวแรกใน string ต้องเป็น ตัวเลข
            - ต้องอ่านค่าได้ 6 ตัว
        */
        if (isdigit((unsigned char)lines[i][0]) &&
            sscanf(lines[i], "%d|%127[^|]|%63[^|]|%31[^|]|%10[^|]|%d\n", // อ่านทีละตัว
                   &e.member_id, e.fullname, e.nickname, e.gender, e.date, &e.os_amount) == 6)
        {
            if (e.member_id == entry->member_id && strcmp(e.date, entry->date) == 0) // ถ้าเป็น ID ที่ตรงกัน KEEP = 0
                keep = 0;
        }
        if (keep) // KEEP = 0 จะไม่เข้า if ทำให้ ข้อมูลดังกล่าวถูกลบ
            fputs(lines[i], fp);
        free(lines[i]);
    }
    free(lines);
    fclose(fp);
    return 1;
}

/*
 * summarize_daily: คำนวณและแสดงสถิติจากไฟล์รายวัน
 * - พารามิเตอร์: `daily_path` ไฟล์รายวัน, `verbose` 1 = แสดงรายละเอียดบนหน้าจอ, 0 = เงียบ
 * - คำนวณจำนวนผู้เล่น, จำนวนลูกรวม, รายได้, การชำระแบบต่าง ๆ และพิมพ์เมื่อ verbose=1
 * - คืนค่า: 1 เมื่อทำงานเสร็จ (หรือ 0 หากเปิดไฟล์ล้มเหลว)
 */
int summarize_daily(const char *daily_path, int verbose)
{
    FILE *fp = fopen(daily_path, "r");
    if (!fp)
        return 0;
    char line[512];
    int total_players = 0, total_shuttle = 0, max_shuttle = 0, min_shuttle = 1 << 30;
    int total_income = 0, paid_cash = 0, paid_transfer = 0, paid_os = 0;
    int total_unpaid = 0; // ยอดค้างรวม (amount_today - (paid_today + paid_os))

    DailyEntry *arr = NULL;
    int count = 0;
    while (fgets(line, sizeof(line), fp))
    {
        if (!isdigit((unsigned char)line[0]))
            continue;
        DailyEntry e;
        if (sscanf(line, "%d|%127[^|]|%63[^|]|%31[^|]|%d|%d|%d|%d|%d|%31[^\n]",
                   &e.member_id, e.fullname, e.nickname, e.gender,
                   &e.shuttle_qty, &e.court_fee, &e.amount_today,
                   &e.paid_today, &e.paid_os, e.method_today) == 10)
        {
            DailyEntry *tmp = realloc(arr, sizeof(DailyEntry) * (count + 1));
            if (!tmp)
            {
                fclose(fp);
                for (int i = 0; i < count; i++) {}
                free(arr);
                return 0;
            }
            arr = tmp;
            arr[count++] = e;

            total_players++;
            total_shuttle += e.shuttle_qty;
            if (e.shuttle_qty > max_shuttle)
                max_shuttle = e.shuttle_qty;
            if (e.shuttle_qty < min_shuttle)
                min_shuttle = e.shuttle_qty;
            total_income += e.amount_today;

            // ยอดที่ได้รับจริง: แยกตามช่องทางชำระเงิน
            if (strcmp(e.method_today, "เงินสด") == 0)
                paid_cash += e.paid_today;
            else if (strcmp(e.method_today, "โอน") == 0)
                paid_transfer += e.paid_today;

            // ยอดที่ชำระจากค้าง (paid_os) เก็บรวมเสมอ (เพื่อนำมาคำนวณยอดรับจริง)
            paid_os += e.paid_os;

            // ยอดค้างที่ยังต้องรับ = amount_today - (paid_today + paid_os)
            int unpaid = e.amount_today - (e.paid_today + e.paid_os);
            if (unpaid > 0)
                total_unpaid += unpaid;
        }
    }
    fclose(fp);
    float avg = total_players ? total_shuttle / total_players : 0;
    int paid_total = paid_cash + paid_transfer + paid_os; // รวมเงินที่รับจริงทั้งหมด

    if (verbose)
    {
        printf("\n=== สรุปข้อมูลรายวัน ===\n");
        printf("ผู้เล่นทั้งหมด %d คน\n", total_players);
        printf("ขายได้ %d ลูก เฉลี่ย %.1f ลูก/คน (สูงสุด %d ลูก, ต่ำสุด %d ลูก)\n", total_shuttle, avg, max_shuttle, min_shuttle == 1 << 30 ? 0 : min_shuttle);
        printf("ยอดรวมที่ต้องได้รับ %d บาท\n", total_income);
        // ปรับข้อความ: แสดงยอดที่รับจริง พร้อมยอดค้างคงค้างอยู่ (ไม่ได้เอา paid_os มาแทน)
        printf("ยอดได้รับจริง (เงินสด %d, โอน %d, ชำระค้าง %d)\n", paid_cash, paid_transfer, paid_os);
        printf("ยอดค้างชำระวันนี้ %d บาท\n", total_unpaid);
        printf("รวมรายได้วันนี้ %d บาท\n", paid_total);
    }
    int choose;
    printf("\nพิมพ์ 0 เพื่อย้อนกลับ : ");
    scanf("%d", &choose);
    return 1;
}

/* print_file: พิมพ์เนื้อหาทั้งหมดของไฟล์ไปยัง stdout
 * - คืนค่า: 1 เมื่อพิมพ์/พบไฟล์, 0 เมื่อไม่พบไฟล์
 */
int print_file(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (!fp)
    {
        printf("ไม่พบไฟล์: %s\n", path);
        delay(3);
        return 0;
    }
    char buf[512];
    while (fgets(buf, sizeof(buf), fp))
        fputs(buf, stdout);
    int _tmp;
    printf("\nพิมพ์ 0 เพื่อย้อนกลับ : ");
    scanf("%d", &_tmp);
    fclose(fp);
    return 1;
}

/*
 * summarize_daily_and_write: สรุปข้อมูลจากไฟล์รายวันและเขียนไฟล์สรุป
 * - พารามิเตอร์:
 *     `daily_path` - ไฟล์รายวันที่อ่าน (เช่น "DD-MM-YYYY.txt")
 *     `out_path` - ไฟล์สรุปที่ต้องการเขียนลง (จะเขียนทับถ้ามีอยู่)
 *     `full` - ถ้าเป็น 1 จะเขียนสรุปรายละเอียดพร้อมรายชื่อผู้เล่น, 0 จะเขียนแบบย่อ
 * - พฤติกรรม: อ่านบรรทัดข้อมูลผู้เล่นจากไฟล์รายวัน, คำนวณสถิติต่าง ๆ
 *   และเขียนผลลงไฟล์สรุปพร้อม BOM UTF-8
 * - คืนค่า: 1 เมื่อสำเร็จ, 0 เมื่อไฟล์ต้นทางหรือไฟล์ผลลัพธ์ไม่สามารถเปิดได้
 */
int summarize_daily_and_write(const char *daily_path, const char *out_path, int full)
{
    FILE *fp = fopen(daily_path, "r");
    if (!fp)
    {
        printf("ไม่พบไฟล์รายวัน: %s\n", daily_path);
        delay(3);
        return 0;
    }
    FILE *fo = fopen(out_path, "w");
    if (!fo)
    {
        printf("ไม่สามารถเขียนไฟล์สรุป: %s\n", out_path);
        delay(3);
        fclose(fp);
        return 0;
    }

    char line[512];
    int total_players = 0, total_shuttle = 0, max_shuttle = 0, min_shuttle = 1 << 30;
    int total_income = 0, paid_cash = 0, paid_transfer = 0, paid_os_today = 0, court_fee_pp = 0, court_fee_total = 0;
    int idx = 0;

    int received_from_os = 0;  // ยอดที่ได้รับจากการชำระค้าง (paid_os sum)
    int outstanding_today = 0; // ยอดค้างคงเหลือ (amount_today - (paid_today + paid_os)) > 0

    char date_only[32] = "";
    {
        const char *bn = strrchr(daily_path, '/');
        const char *bp = strrchr(daily_path, '\\');
        const char *base = bn ? bn + 1 : (bp ? bp + 1 : daily_path);
        strncpy(date_only, base, sizeof(date_only) - 1);
        char *dot = strrchr(date_only, '.');
        if (dot)
            *dot = '\0';
    }

    if (full)
    {
        fprintf(fo, "วันที่|%s\n", date_only);
        fprintf(fo, "ลำดับ|ID|ชื่อ-นามสกุล|ชื่อเล่น|จำนวนลูก|ยอดวันนี้|ชำระวันนี้|ชำระจากค้าง|คงค้าง|วิธีชำระ\n");
    }
    else
    {
        fprintf(fo, "วันที่|%s\n", date_only);
    }
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
            court_fee_pp = court_fee;
            court_fee_total += court_fee;

            // เก็บยอดที่ชำระจริงตามวิธี
            if (strcmp(method_today, "เงินสด") == 0)
                paid_cash += paid_today;
            else if (strcmp(method_today, "โอน") == 0)
                paid_transfer += paid_today;

            // ยอดที่ชำระจากค้างสะสม
            if (paid_os > 0)
            {
                received_from_os += paid_os;
                paid_os_today += paid_os;
            }

            // คำนวณยอดค้างคงเหลือของแถวนี้ (บวกเฉพาะกรณียังค้าง)
            int unpaid = amount_today - (paid_today + paid_os);
            if (unpaid > 0)
                outstanding_today += unpaid;

            if (full)
            {
                const char *m = method_today[0] ? method_today : "ไม่ระบุ";
                int unpaid_row = amount_today - (paid_today + paid_os);
                if (unpaid_row < 0)
                    unpaid_row = 0;
                fprintf(fo, "%d|%s|%d|%d|%d|%d|%d|%d|%s\n",
                        ++idx, nickname, member_id, shuttle_qty, amount_today, paid_today, paid_os, unpaid_row, m);
            }
        }
    }
    fclose(fp);

    {
        int avg = total_players ? total_shuttle / total_players : 0;
        int received_today = paid_cash + paid_transfer;
        int total_received = received_today + received_from_os;

        fprintf(fo, "รวมจำนวนลูกทั้งหมด|%d\n", total_shuttle);
        fprintf(fo, "ค่าเฉลี่ยลูก/คน|%d\n", avg);
        fprintf(fo, "จำนวนลูกสูงสุด|%d\n", max_shuttle);
        fprintf(fo, "จำนวนลูกต่ำสุด|%d\n", (min_shuttle == 1 << 30) ? 0 : min_shuttle);
        fprintf(fo, "จำนวนผู้เล่นทั้งหมด|%d\n", total_players);
        fprintf(fo, "ค่าสนามต่อคน|%d\n", court_fee_pp);
        fprintf(fo, "ค่าสนามรวม|%d\n", court_fee_total);
        fprintf(fo, "ยอดที่ต้องได้รับทั้งหมด|%d\n", total_income);
        fprintf(fo, "ยอดที่ได้รับวันนี้ (เงินสด/โอน)|%d\n", received_today);
        fprintf(fo, "เป็นเงินสด|%d\n", paid_cash);
        fprintf(fo, "เป็นเงินโอน|%d\n", paid_transfer);
        fprintf(fo, "ยอดได้รับจากการชำระค้าง|%d\n", received_from_os);
        fprintf(fo, "ยอดค้างจ่ายของวันนี้|%d\n", outstanding_today);
        fprintf(fo, "ยอดรับรวมทั้งหมด|%d\n", total_received);
    }
    fclose(fo);
    return 1;
}

/*
 * upsert_os_entry: อัปเดตหรือเพิ่มรายการค้างในไฟล์ `OSpayment.txt`
 * - ถ้ามีบรรทัด (member_id + date) อยู่แล้ว -> แทนที่ด้วย os_amount ใหม่และ note ใหม่
 * - ถ้าไม่มี -> append ใหม่
 * - คืนค่า 1 เมื่อสำเร็จ, 0 เมื่อไม่สามารถเขียนไฟล์ได้
 */
int upsert_os_entry(const char *os_path, const Member *m, const char *date_ddmmyyyy, int os_amount, const char *note)
{
    // หากไฟล์ไม่อยู่ ให้ append สร้างไฟล์ใหม่
    FILE *fp = fopen(os_path, "r");
    if (!fp)
    {
        return append_os(os_path, m, date_ddmmyyyy, os_amount, note);
    }

    // อ่านไฟล์ทั้งหมดในหน่วยความจำ
    char **lines = NULL;
    int n = 0;
    char buf[512];
    while (fgets(buf, sizeof(buf), fp))
    {
        char **tmp = realloc(lines, sizeof(char *) * (n + 1));
        if (!tmp)
        {
            fclose(fp);
            for (int i = 0; i < n; i++)
                free(lines[i]);
            free(lines);
            return 0;
        }
        lines = tmp;
        lines[n] = malloc(strlen(buf) + 1);
        if (!lines[n])
        {
            fclose(fp);
            for (int i = 0; i < n; i++)
                free(lines[i]);
            free(lines);
            return 0;
        }
        strcpy(lines[n++], buf);
    }
    fclose(fp);

    // เปิดไฟล์ทับเพื่อเขียนใหม่
    fp = fopen(os_path, "w");
    if (!fp)
    {
        for (int i = 0; i < n; i++)
            free(lines[i]);
        free(lines);
        return 0;
    }

    int updated = 0;
    for (int i = 0; i < n; i++)
    {
        // parse full OS entry including note
        OSEntry e;
        if (isdigit((unsigned char)lines[i][0]) &&
            sscanf(lines[i], "%d|%127[^|]|%63[^|]|%31[^|]|%10[^|]|%d|%127[^\n]",
                   &e.member_id, e.fullname, e.nickname, e.gender, e.date, &e.os_amount, e.note) >= 6)
        {
            if (e.member_id == m->id && strcmp(e.date, date_ddmmyyyy) == 0)
            {
                // write updated entry with new amount/note
                fprintf(fp, "%d|%s|%s|%s|%s|%d|%s\n",
                        m->id, m->fullname, m->nickname, m->gender, date_ddmmyyyy, os_amount, note ? note : "");
                updated = 1;
            }
            else
            {
                fputs(lines[i], fp);
            }
        }
        else
        {
            // keep any line we couldn't parse (preserve)
            fputs(lines[i], fp);
        }
        free(lines[i]);
    }
    free(lines);

    if (!updated)
    {
        // append new line at end
        fprintf(fp, "%d|%s|%s|%s|%s|%d|%s\n", m->id, m->fullname, m->nickname, m->gender, date_ddmmyyyy, os_amount, note ? note : "");
    }

    fclose(fp);
    return 1;
}
