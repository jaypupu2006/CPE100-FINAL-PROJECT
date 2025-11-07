#ifndef MENU2_H
#define MENU2_H

#include <stddef.h>

#define NAME_MAXLEN 128
#define NICK_MAXLEN 64
#define DATE_MAXLEN 16
#define NOTE_MAXLEN 128

typedef enum {
    BY_ID = 1,
    BY_NICKNAME = 2,
    BY_FULLNAME = 3
} SearchBy;

typedef enum {
    PAY_NONE = 0,
    PAY_CASH = 1,
    PAY_TRANSFER = 2,
    PAY_OS = 3
} PayMethod;

typedef struct {
    int id;
    char fullname[NAME_MAXLEN];
    char nickname[NICK_MAXLEN];
    int gender;
    char created_at[DATE_MAXLEN];
} Member;

typedef struct {
    int member_id;
    char fullname[NAME_MAXLEN];
    char nickname[NICK_MAXLEN];
    int gender;
    int shuttle_qty;
    int court_fee;
    int amount_today;
    int paid_today;
    int paid_os;
    int method_today;
} DailyEntry;

typedef struct {
    int member_id;
    char fullname[NAME_MAXLEN];
    char nickname[NICK_MAXLEN];
    int gender;
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

int search_members(const char *member_path, SearchBy by, const char *key, Member **out_arr, size_t *out_count);
int search_daily(const char *daily_path, SearchBy by, const char *key, DailyEntry **out_arr, size_t *out_count);
int search_os(const char *os_path, SearchBy by, const char *key, OSEntry **out_arr, size_t *out_count);

int upsert_daily_entry(const char *daily_path, const Prices *prices, const Member *m, int add_shuttle_qty, PayMethod method, int pay_today, int pay_os);

int append_os(const char *os_path, const Member *m, const char *date_ddmmyyyy, int os_amount, const char *note);
int remove_os_entry(const char *os_path, const OSEntry *entry);

int summarize_daily(const char *daily_path, int verbose);

void menu2_choose(void);

#endif
