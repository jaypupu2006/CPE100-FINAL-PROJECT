#ifndef MENU2_H
#define MENU2_H

#include <stddef.h> // ใช้งานเกี่ยวกับ  pointer และ memory

#define NAME_MAXLEN 128
#define NICK_MAXLEN 64
#define DATE_MAXLEN 11
#define GENDER_MAXLEN 32
#define NOTE_MAXLEN 128
#define method_today_MAXLEN 32

typedef enum {  // ประกาศ enumeration เก็บ int เท่านั้น
    BY_ID = 1,
    BY_NICKNAME = 2,   // define ค่าไว้ กันเขียนไปเขียนมาแล้วลืม
    BY_FULLNAME = 3
} SearchBy; // SearchBy สามารถ define ค่าได้แค่ 3 ตัวนี้เท่านั้น

typedef enum {
    PAY_CASH = 1,
    PAY_TRANSFER = 2,
    PAY_OS = 3
} PayMethod;

typedef struct { //ประกาศ struct ปกติ
    int id;
    char fullname[NAME_MAXLEN];
    char nickname[NICK_MAXLEN];
    char gender[GENDER_MAXLEN];
    char created_at[DATE_MAXLEN];
} Member;

typedef struct {
    int member_id;                              // 1
    char fullname[NAME_MAXLEN];                 // 2
    char nickname[NICK_MAXLEN];                 // 3
    char gender[GENDER_MAXLEN];                 // 4
    int shuttle_qty;                            // 5
    int court_fee;                              // 6
    int amount_today;                           // 7
    int paid_today;                             // 8
    int paid_os;                                // 9
    char method_today[method_today_MAXLEN];     // 10
} DailyEntry;

typedef struct {
    int member_id;
    char fullname[NAME_MAXLEN];
    char nickname[NICK_MAXLEN];
    char gender[GENDER_MAXLEN]; 
    char date[DATE_MAXLEN];
    int os_amount;
    char note[NOTE_MAXLEN];
} OSEntry;

typedef struct {
    int shuttle_price;
    int court_fee_per_person;
} Prices;

int open_or_create_daily(const char *daily_path);
int load_prices(const char *config_path, Prices *out);
int save_prices(const char *config_path, const Prices *in);

int search_members(SearchBy by, const char *key, Member **out_arr, int *out_count);
int search_daily(const char *daily_path, SearchBy by, const char *key, DailyEntry **out_arr, int *out_count);
int search_os(const char *os_path, SearchBy by, const char *key, OSEntry **out_arr, int *out_count);

int upsert_daily_entry(const char *daily_path, const Prices *prices, const Member *m, int add_shuttle_qty, int count_player, PayMethod method, int pay_today, int pay_os);

int append_os(const char *os_path, const Member *m, const char *date_ddmmyyyy, int os_amount, const char *note);
int remove_os_entry(const char *os_path, const OSEntry *entry);

int summarize_daily(const char *daily_path, int verbose);

void menu2_choose(void);

#endif
