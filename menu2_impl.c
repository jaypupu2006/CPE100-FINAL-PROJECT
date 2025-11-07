#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "menu2.h"

static char* dupstr(const char* s){
    size_t n=strlen(s)+1;
    char* p=(char*)malloc(n);
    if(p) memcpy(p,s,n);
    return p;
}

int open_or_create_daily(const char *daily_path) {
    FILE *fp = fopen(daily_path, "r");
    if (fp) { fclose(fp); return 1; }
    fp = fopen(daily_path, "w");
    if (!fp) return 0;
    fprintf(fp, "member_id|fullname|nickname|gender|shuttle_qty|court_fee|amount_today|paid_today|paid_os|method_today\n");
    fclose(fp);
    return 1;
}

int load_prices(const char *config_path, Prices *out) {
    FILE *fp = fopen(config_path, "r");
    if (!fp) return 0;
    char key[64], val[64];
    int s=0,c=0;
    while (fscanf(fp, "%63[^=]=%63s\n", key, val) == 2) {
        if (strcmp(key, "SHUTTLE_PRICE") == 0){ out->shuttle_price = atoi(val); s=1; }
        else if (strcmp(key, "COURT_FEE_PER_PERSON") == 0){ out->court_fee_per_person = atoi(val); c=1; }
    }
    fclose(fp);
    return s&&c;
}

int save_prices(const char *config_path, const Prices *in) {
    FILE *fp = fopen(config_path, "w");
    if (!fp) return 0;
    fprintf(fp, "SHUTTLE_PRICE=%d\n", in->shuttle_price);
    fprintf(fp, "COURT_FEE_PER_PERSON=%d\n", in->court_fee_per_person);
    fclose(fp);
    return 1;
}

int search_members(const char *member_path, SearchBy by, const char *key, Member **out_arr, size_t *out_count) {
    FILE *fp = fopen(member_path, "r");
    if (!fp) return 0;
    Member *arr = NULL;
    size_t count = 0;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (!isdigit((unsigned char)line[0])) continue;
        Member m;
        if (sscanf(line, "%d|%127[^|]|%63[^|]|%d|%15s", &m.id, m.fullname, m.nickname, &m.gender, m.created_at) == 5) {
            int match = 0;
            if (by == BY_ID && atoi(key) == m.id) match = 1;
            else if (by == BY_NICKNAME && strstr(m.nickname, key)) match = 1;
            else if (by == BY_FULLNAME && strstr(m.fullname, key)) match = 1;
            if (match) {
                Member *tmp = realloc(arr, sizeof(Member) * (count + 1));
                if(!tmp){ fclose(fp); free(arr); return 0; }
                arr = tmp;
                arr[count++] = m;
            }
        }
    }
    fclose(fp);
    *out_arr = arr;
    *out_count = count;
    return 1;
}

int search_daily(const char *daily_path, SearchBy by, const char *key, DailyEntry **out_arr, size_t *out_count) {
    FILE *fp = fopen(daily_path, "r");
    if (!fp) return 0;
    DailyEntry *arr = NULL;
    size_t count = 0;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (!isdigit((unsigned char)line[0])) continue;
        DailyEntry e;
        if (sscanf(line, "%d|%127[^|]|%63[^|]|%d|%d|%d|%d|%d|%d|%d",
                   &e.member_id, e.fullname, e.nickname, &e.gender,
                   &e.shuttle_qty, &e.court_fee, &e.amount_today,
                   &e.paid_today, &e.paid_os, &e.method_today) == 10) {
            int match = 0;
            if (by == BY_ID && atoi(key) == e.member_id) match = 1;
            else if (by == BY_NICKNAME && strstr(e.nickname, key)) match = 1;
            else if (by == BY_FULLNAME && strstr(e.fullname, key)) match = 1;
            if (match) {
                DailyEntry *tmp = realloc(arr, sizeof(DailyEntry) * (count + 1));
                if(!tmp){ fclose(fp); free(arr); return 0; }
                arr = tmp;
                arr[count++] = e;
            }
        }
    }
    fclose(fp);
    *out_arr = arr;
    *out_count = count;
    return 1;
}

int search_os(const char *os_path, SearchBy by, const char *key, OSEntry **out_arr, size_t *out_count) {
    FILE *fp = fopen(os_path, "r");
    if (!fp) return 0;
    OSEntry *arr = NULL;
    size_t count = 0;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (!isdigit((unsigned char)line[0])) continue;
        OSEntry e;
        if (sscanf(line, "%d|%127[^|]|%63[^|]|%d|%15[^|]|%d|%127[^\n]",
                   &e.member_id, e.fullname, e.nickname, &e.gender, e.date, &e.os_amount, e.note) == 7) {
            int match = 0;
            if (by == BY_ID && atoi(key) == e.member_id) match = 1;
            else if (by == BY_NICKNAME && strstr(e.nickname, key)) match = 1;
            else if (by == BY_FULLNAME && strstr(e.fullname, key)) match = 1;
            if (match) {
                OSEntry *tmp = realloc(arr, sizeof(OSEntry) * (count + 1));
                if(!tmp){ fclose(fp); free(arr); return 0; }
                arr = tmp;
                arr[count++] = e;
            }
        }
    }
    fclose(fp);
    *out_arr = arr;
    *out_count = count;
    return 1;
}

int upsert_daily_entry(const char *daily_path, const Prices *prices, const Member *m, int add_shuttle_qty, PayMethod method, int pay_today, int pay_os) {
    FILE *fp = fopen(daily_path, "r");
    if (!fp) return 0;
    char **lines = NULL;
    size_t n = 0;
    char buf[512];
    while (fgets(buf, sizeof(buf), fp)) {
        char* d = dupstr(buf);
        if(!d){ fclose(fp); for(size_t i=0;i<n;i++) free(lines[i]); free(lines); return 0; }
        char **tmp = realloc(lines, sizeof(char*) * (n + 1));
        if(!tmp){ fclose(fp); free(d); for(size_t i=0;i<n;i++) free(lines[i]); free(lines); return 0; }
        lines = tmp;
        lines[n++] = d;
    }
    fclose(fp);

    fp = fopen(daily_path, "w");
    if (!fp) { for(size_t i=0;i<n;i++) free(lines[i]); free(lines); return 0; }
    int updated = 0;
    for (size_t i = 0; i < n; i++) {
        DailyEntry e;
        if (isdigit((unsigned char)lines[i][0]) &&
            sscanf(lines[i], "%d|%127[^|]|%63[^|]|%d|%d|%d|%d|%d|%d|%d",
                   &e.member_id, e.fullname, e.nickname, &e.gender,
                   &e.shuttle_qty, &e.court_fee, &e.amount_today,
                   &e.paid_today, &e.paid_os, &e.method_today) == 10 &&
            e.member_id == m->id) {
            e.shuttle_qty += add_shuttle_qty;
            e.court_fee = prices->court_fee_per_person;
            e.amount_today = e.shuttle_qty * prices->shuttle_price + e.court_fee;
            if (method != PAY_NONE) e.method_today = method;
            if (pay_today) e.paid_today = pay_today;
            if (pay_os) e.paid_os = pay_os;
            fprintf(fp, "%d|%s|%s|%d|%d|%d|%d|%d|%d|%d\n",
                    e.member_id, e.fullname, e.nickname, e.gender,
                    e.shuttle_qty, e.court_fee, e.amount_today,
                    e.paid_today, e.paid_os, e.method_today);
            updated = 1;
        } else {
            fputs(lines[i], fp);
        }
        free(lines[i]);
    }
    free(lines);

    if (!updated) {
        fprintf(fp, "%d|%s|%s|%d|%d|%d|%d|%d|%d|%d\n",
                m->id, m->fullname, m->nickname, m->gender,
                add_shuttle_qty, prices->court_fee_per_person,
                add_shuttle_qty * prices->shuttle_price + prices->court_fee_per_person,
                pay_today, pay_os, method);
    }
    fclose(fp);
    return 1;
}

int append_os(const char *os_path, const Member *m, const char *date_ddmmyyyy, int os_amount, const char *note) {
    FILE *fp = fopen(os_path, "a");
    if (!fp) return 0;
    fprintf(fp, "%d|%s|%s|%d|%s|%d|%s\n", m->id, m->fullname, m->nickname, m->gender, date_ddmmyyyy, os_amount, note);
    fclose(fp);
    return 1;
}

int remove_os_entry(const char *os_path, const OSEntry *entry) {
    FILE *fp = fopen(os_path, "r");
    if (!fp) return 0;
    char **lines = NULL;
    size_t n = 0;
    char buf[512];
    while (fgets(buf, sizeof(buf), fp)) {
        char* d = dupstr(buf);
        if(!d){ fclose(fp); for(size_t i=0;i<n;i++) free(lines[i]); free(lines); return 0; }
        char **tmp = realloc(lines, sizeof(char*) * (n + 1));
        if(!tmp){ fclose(fp); free(d); for(size_t i=0;i<n;i++) free(lines[i]); free(lines); return 0; }
        lines = tmp;
        lines[n++] = d;
    }
    fclose(fp);

    fp = fopen(os_path, "w");
    if (!fp) { for(size_t i=0;i<n;i++) free(lines[i]); free(lines); return 0; }
    for (size_t i = 0; i < n; i++) {
        OSEntry e;
        int keep = 1;
        if (isdigit((unsigned char)lines[i][0]) &&
            sscanf(lines[i], "%d|%127[^|]|%63[^|]|%d|%15[^|]|%d|%127[^\n]",
                   &e.member_id, e.fullname, e.nickname, &e.gender, e.date, &e.os_amount, e.note) == 7) {
            if (e.member_id == entry->member_id && strcmp(e.date, entry->date) == 0) keep = 0;
        }
        if (keep) fputs(lines[i], fp);
        free(lines[i]);
    }
    free(lines);
    fclose(fp);
    return 1;
}

int summarize_daily(const char *daily_path, int verbose) {
    FILE *fp = fopen(daily_path, "r");
    if (!fp) return 0;
    char line[512];
    int total_players = 0, total_shuttle = 0, max_shuttle = 0, min_shuttle = 1<<30;
    int total_income = 0, paid_cash = 0, paid_transfer = 0, paid_os = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (!isdigit((unsigned char)line[0])) continue;
        DailyEntry e;
        if (sscanf(line, "%d|%127[^|]|%63[^|]|%d|%d|%d|%d|%d|%d|%d",
                   &e.member_id, e.fullname, e.nickname, &e.gender,
                   &e.shuttle_qty, &e.court_fee, &e.amount_today,
                   &e.paid_today, &e.paid_os, &e.method_today) == 10) {
            total_players++;
            total_shuttle += e.shuttle_qty;
            if (e.shuttle_qty > max_shuttle) max_shuttle = e.shuttle_qty;
            if (e.shuttle_qty < min_shuttle) min_shuttle = e.shuttle_qty;
            total_income += e.amount_today;
            if (e.method_today == PAY_CASH) paid_cash += e.paid_today;
            else if (e.method_today == PAY_TRANSFER) paid_transfer += e.paid_today;
            else if (e.method_today == PAY_OS) paid_os += e.amount_today;
        }
    }
    fclose(fp);
    int avg = total_players ? total_shuttle / total_players : 0;
    int paid_total = paid_cash + paid_transfer;
    if (verbose) {
        printf("\n=== สรุปข้อมูลรายวัน ===\n");
        printf("ผู้เล่นทั้งหมด %d คน\n", total_players);
        printf("ขายได้ %d ลูก เฉลี่ย %d ลูก/คน (สูงสุด %d ต่ำสุด %d)\n", total_shuttle, avg, max_shuttle, min_shuttle==1<<30?0:min_shuttle);
        printf("ยอดรวมที่ต้องได้รับ %d บาท\n", total_income);
        printf("ยอดได้รับจริง (เงินสด %d, โอน %d, ค้าง %d)\n", paid_cash, paid_transfer, paid_os);
        printf("รวมรายได้วันนี้ %d บาท\n", paid_total);
    }
    return 1;
}
