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
    if (!fp){
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

/*
 * upsert_daily_entry: เพิ่มหรืออัปเดตรายการของสมาชิกในไฟล์รายวัน
 * - พารามิเตอร์:
 *   `daily_path` - ไฟล์รายวัน, `prices` - ข้อมูลราคา, `m` - สมาชิกที่ต้องการอัปเดต,
 *   `add_shuttle_qty` - จำนวนลูกที่เพิ่ม (ถ้าเพิ่มเป็นรายการใหม่จะใช้ค่านี้),
 *   `method` - วิธีการชำระเงิน (PAY_NONE/PAY_CASH/PAY_TRANSFER/PAY_OS),
 *   `pay_today` - จำนวนเงินที่ชำระวันนี้ (ถ้ามี), `pay_os` - จำนวนที่ชำระจากค้าง (ถ้ามี)
 * - พฤติกรรม: อ่านไฟล์ทั้งหมดมาไว้ในหน่วยความจำ, แก้บรรทัดที่ตรงกับ `m->id` หรือเพิ่มใหม่ถ้าไม่พบ
 * - คืนค่า: 1 เมื่อสำเร็จ, 0 เมื่อเกิดข้อผิดพลาด (I/O หรือหน่วยความจำ)
 */
int upsert_daily_entry(const char *daily_path, const Prices *prices, const Member *m, int add_shuttle_qty, int count_player, PayMethod method, int pay_today, int pay_os)
{
    FILE *fp = fopen(daily_path, "r"); // เปิดไฟล์ Daily
    if (fp == NULL)
    {
        printf("ไม่สามารถเปิดหรือสร้างไฟล์รายวันได้ เปิดหาโฟล์เดอร์ \"input/Daily data\" ไม่เจอ\n");
        delay(3);
        return 0;
    }

    char **lines = NULL;
    int n = 0;
    char buf[512];
    while (fgets(buf, sizeof(buf), fp)) // อ่านข้อมูลทุกบรรทัด ในไฟล์ .txt
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
        lines[n] = malloc(strlen(buf) + 1); // จองพื้นที่สำหรับ define ค่า ตามขนาดของ buf คล้าย ๆ เพิ่มขนาดของ j ใน arr[i][j]
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

    fp = fopen(daily_path, "w");
    if (!fp)
    {
        for (int i = 0; i < n; i++)
            free(lines[i]);
        free(lines);
        printf("ไม่สามารถเปิดไฟล์รายวันได้ เปิดหาโฟล์เดอร์ \"input/Daily data\" ไม่เจอ\n");
        delay(3);
        return 0;
    }
    int updated = 0; // 0 = ข้อมูลดังกล่าวยังไม่มีใน daily
    for (int i = 0; i < n; i++)
    {
        DailyEntry e;

        /*
            ถ้า char แรก เป็นตัวเลข isdigit -- ctype.h
            ถ้าอ่านข้อมูลได้ทั้ง 10 ตัวแปร
            ถ้า id ตรงกับที่ pointer m ชี้อยู่
        */
        if (isdigit((unsigned char)lines[i][0]) &&
            sscanf(lines[i], "%d|%127[^|]|%63[^|]|%31[^|]|%d|%d|%d|%d|%d|%31[^|]",
                   &e.member_id, e.fullname, e.nickname, e.gender,
                   &e.shuttle_qty, &e.court_fee, &e.amount_today,
                   &e.paid_today, &e.paid_os, e.method_today) == 10 &&
            e.member_id == m->id &&
            strcmp(e.method_today, "ค้างจ่าย") == 0)
        {                                                                       // บรรทัดข้างล่างอยู่ใน if
            int players = count_player > 0 ? count_player : 1;
            e.shuttle_qty += add_shuttle_qty;                                   // เพิ่มจำนวนลูก
            e.court_fee = prices->court_fee_per_person;                         // ค่าคอร์ท
            int cal = (prices->shuttle_price / players) * add_shuttle_qty;      // คิดราคาค่าลูกรายบุคคล
            e.amount_today += cal;                                              // รวมยอด

            if (method != PAY_OS)
            {
                if (method == PAY_CASH)
                    strcpy(e.method_today, "เงินสด");
                else if (method == PAY_TRANSFER)
                    strcpy(e.method_today, "โอน");
                else // invalid method
                {
                    printf("กรอกข้อมูลผิดพลาด\n");
                    for (int j = 0; j < n; j++)
                        free(lines[j]);
                    free(lines);
                    fclose(fp);
                    return 0;
                }
            }
            // update paid fields: overwrite or add depending on logic desired — maintain current behavior
            if (pay_today)
                e.paid_today = pay_today;
            if (pay_os)
                e.paid_os = pay_os;

            fprintf(fp, "%d|%s|%s|%s|%d|%d|%d|%d|%d|%s\n",
                    e.member_id, e.fullname, e.nickname, e.gender,
                    e.shuttle_qty, e.court_fee, e.amount_today,
                    e.paid_today, e.paid_os, e.method_today);
            updated = 1;
        }
        else
        {
            fputs(lines[i], fp); // คล้าย ๆ fprintf -- แต่เป็นการเขียนแค่ string เท่านั้นเลยเร็วกว่านิดนึงแหละ
        }
        free(lines[i]); // free ทิ้งเปลือง mem
    }
    free(lines);

    if (!updated) // ถ้าไม่มีข้อมูลเดิมใน Daily จะเพิ่มข้อมูลแถวใหม่
    {
        DailyEntry e;
        int players = count_player > 0 ? count_player : 1;
        strcpy(e.method_today, (method == PAY_CASH ? "เงินสด" : (method == PAY_TRANSFER ? "โอน" : "ค้างจ่าย")));
        int cal = (prices->shuttle_price / players) * add_shuttle_qty;
        e.shuttle_qty = add_shuttle_qty;
        e.member_id = m->id;
        strncpy(e.fullname, m->fullname, NAME_MAXLEN);
        strncpy(e.nickname, m->nickname, NICK_MAXLEN);
        strncpy(e.gender, m->gender, GENDER_MAXLEN);
        e.court_fee = prices->court_fee_per_person;
        e.amount_today = cal + prices->court_fee_per_person;
        e.paid_today = (method == PAY_CASH || method == PAY_TRANSFER) ? pay_today : 0;
        e.paid_os = (method == PAY_OS) ? pay_os : 0;

        fprintf(fp, "%d|%s|%s|%s|%d|%d|%d|%d|%d|%s\n",
                e.member_id, e.fullname, e.nickname, e.gender,
                e.shuttle_qty, e.court_fee, e.amount_today,
                e.paid_today, e.paid_os, e.method_today);
    }
    fclose(fp);
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
            if (e.member_id == entry->member_id && strcmp(e.date, entry->date) == 0) //ถ้าเป็น ID ที่ตรงกัน KEEP = 0
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
