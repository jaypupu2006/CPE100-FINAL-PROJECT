#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "menu2.h"
#include "menu3.h"
#include "delay.h"

/*
 * file_exists: ช่วยตรวจว่าไฟล์ที่กำหนดมีอยู่หรือไม่
 * - พารามิเตอร์: `path` เส้นทางไฟล์ที่จะตรวจ
 * - ผลลัพธ์: คืนค่า 1 เมื่อไฟล์เปิดได้ (มีอยู่), 0 เมื่อไม่พบหรือเปิดไม่ได้
 */
static int file_exists(const char *path){
    FILE *fp=fopen(path,"r");
    if(!fp) return 0;
    fclose(fp);
    return 1;
}

/*
 * print_file: พิมพ์เนื้อหาทั้งหมดของไฟล์ไปยัง stdout
 * - พารามิเตอร์: `path` เส้นทางไฟล์ที่จะพิมพ์
 * - พฤติกรรม: ถ้าไม่พบไฟล์ จะแสดงข้อความแจ้งและคืนค่าออกโดยไม่ทำอะไร
 */
static void print_file(const char *path){
    FILE *fp=fopen(path,"r");
    if(!fp){
        printf("ไม่พบไฟล์: %s\n", path);
        delay(3);
        return;
    }
    char buf[512];
    while(fgets(buf,sizeof(buf),fp)) fputs(buf,stdout);
    int _tmp;
    printf("พิมพ์ 0 เพื่อย้อนกลับ :");
    scanf("%d", &_tmp);
    fclose(fp);
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
static int summarize_daily_and_write(const char *daily_path, const char *out_path, int full){
    FILE *fp=fopen(daily_path,"r");
    if(!fp){
        printf("ไม่พบไฟล์รายวัน: %s\n", daily_path);
        delay(3);
        return 0;
    }
    FILE *fo=fopen(out_path,"w");
    if(!fo){
        printf("ไม่สามารถเขียนไฟล์สรุป: %s\n", out_path);
        delay(3);
        fclose(fp);
        return 0;
    }

    /* เขียน BOM UTF-8 กันภาษาไทยเพี้ยนใน Excel/Notepad บางตัว */
    fwrite("\xEF\xBB\xBF",1,3,fo);

    char line[512];
    int total_players=0,total_shuttle=0,max_shuttle=0,min_shuttle=1<<30;
    int total_income=0,paid_cash=0,paid_transfer=0,paid_os_today=0,court_fee_pp=0,court_fee_total=0;
    int idx=0;

    char date_only[32]="";
    {
        const char *bn=strrchr(daily_path,'/');
        const char *bp=strrchr(daily_path,'\\');
        const char *base=bn?bn+1:(bp?bp+1:daily_path);
        strncpy(date_only, base, sizeof(date_only)-1);
        char *dot=strrchr(date_only,'.');
        if(dot) *dot='\0';
    }

    if(full){
        fprintf(fo,"วันที่|%s\n",date_only);
        fprintf(fo,"ลำดับ|ชื่อเล่น|เลขสมาชิก|จำนวนลูก|ยอดวันนี้|ชำระวันนี้|ค้างสะสม|วิธีชำระ\n");
    } else {
        fprintf(fo,"วันที่|%s\n",date_only);
    }

    while(fgets(line,sizeof(line),fp)){
        if(!isdigit((unsigned char)line[0])) continue;
        int member_id,gender,shuttle_qty,court_fee,amount_today,paid_today,paid_os,method;
        char fullname[NAME_MAXLEN], nickname[NICK_MAXLEN];
        if(sscanf(line,"%d|%127[^|]|%63[^|]|%d|%d|%d|%d|%d|%d|%d",
                  &member_id, fullname, nickname, &gender,
                  &shuttle_qty, &court_fee, &amount_today,
                  &paid_today, &paid_os, &method)==10){
            total_players++;
            total_shuttle+=shuttle_qty;
            if(shuttle_qty>max_shuttle) max_shuttle=shuttle_qty;
            if(shuttle_qty<min_shuttle) min_shuttle=shuttle_qty;
            total_income+=amount_today;
            court_fee_pp=court_fee;
            court_fee_total+=court_fee;
            if(method==PAY_CASH) paid_cash+=paid_today;
            else if(method==PAY_TRANSFER) paid_transfer+=paid_today;
            else if(method==PAY_OS) paid_os_today+=amount_today;

            if(full){
                const char *m="ไม่ระบุ";
                if(method==PAY_CASH) m="เงินสด";
                else if(method==PAY_TRANSFER) m="โอน";
                else if(method==PAY_OS) m="ค้างจ่าย";
                fprintf(fo,"%d|%s|%d|%d|%d|%d|%d|%s\n",
                        ++idx, nickname, member_id, shuttle_qty, amount_today, paid_today, paid_os, m);
            }
        }
    }
    fclose(fp);

    {
        int avg = total_players? total_shuttle/total_players : 0;
        int received_today = paid_cash + paid_transfer;
        int received_from_os = 0;

        fprintf(fo,"รวมจำนวนลูกทั้งหมด|%d\n", total_shuttle);
        fprintf(fo,"ค่าเฉลี่ยลูก/คน|%d\n", avg);
        fprintf(fo,"จำนวนลูกสูงสุด|%d\n", max_shuttle);
        fprintf(fo,"จำนวนลูกต่ำสุด|%d\n", (min_shuttle==1<<30)?0:min_shuttle);
        fprintf(fo,"จำนวนผู้เล่นทั้งหมด|%d\n", total_players);
        fprintf(fo,"ค่าสนามต่อคน|%d\n", court_fee_pp);
        fprintf(fo,"ค่าสนามรวม|%d\n", court_fee_total);
        fprintf(fo,"ยอดที่ต้องได้รับทั้งหมด|%d\n", total_income);
        fprintf(fo,"ยอดที่ได้รับวันนี้|%d\n", received_today);
        fprintf(fo,"เป็นเงินสด|%d\n", paid_cash);
        fprintf(fo,"เป็นเงินโอน|%d\n", paid_transfer);
        fprintf(fo,"ยอดค้างจ่ายของวันนี้|%d\n", paid_os_today);
        fprintf(fo,"ยอดที่ได้รับจากการชำระค้าง|%d\n", received_from_os);
        fprintf(fo,"ยอดรับรวมทั้งหมด|%d\n", received_today + received_from_os);
    }
    int _tmp;
    printf("พิมพ์ 0 เพื่อย้อนกลับ :");
    scanf("%d", &_tmp);
    fclose(fo);
    return 1;
}

/*
 * summarize_month_and_write: สรุปรายเดือนจากไฟล์รายวัน (DD-MM-YYYY.txt)
 * - พารามิเตอร์:
 *     `month_yyyy` - เดือนและปี รูปแบบ "MM-YYYY" เพื่อค้นหาไฟล์รายวันทั้งหมดของเดือน
 *     `out_path` - ไฟล์สรุปรายเดือนที่จะเขียนผล
 * - พฤติกรรม: สแกนไฟล์รายวันตั้งแต่ 01 ถึง 31 ของเดือนที่กำหนด,
 *   รวมยอดลูก, รายได้, การชำระ และยอดค้างที่อยู่ใน `OSpayment.txt` สำหรับเดือนนั้น
 * - คืนค่า: 1 เมื่อเขียนไฟล์สรุปรายเดือนสำเร็จ, 0 เมื่อไม่สามารถเขียนไฟล์ผลลัพธ์ได้
 */
static int summarize_month_and_write(const char *month_yyyy, const char *out_path){
    char path[128];
    int day, total_days=0;
    int total_players=0,total_shuttle=0,max_shuttle=0,min_shuttle=1<<30;
    int total_income=0,paid_cash=0,paid_transfer=0,paid_from_os=0,court_fee_total=0,court_fee_pp_last=0;

    for(day=1; day<=31; day++){
        char dd[3];
        snprintf(dd,sizeof(dd),"%02d",day);
        snprintf(path,sizeof(path),"%s-%s.txt",dd,month_yyyy);

        FILE *fp=fopen(path,"r");
        if(!fp) continue;
        total_days++;
        char line[512];
        while(fgets(line,sizeof(line),fp)){
            if(!isdigit((unsigned char)line[0])) continue;
            int member_id,gender,shuttle_qty,court_fee,amount_today,paid_today,paid_os,method;
            char fullname[NAME_MAXLEN], nickname[NICK_MAXLEN];
            if(sscanf(line,"%d|%127[^|]|%63[^|]|%d|%d|%d|%d|%d|%d|%d",
                      &member_id, fullname, nickname, &gender,
                      &shuttle_qty, &court_fee, &amount_today,
                      &paid_today, &paid_os, &method)==10){
                total_players++;
                total_shuttle+=shuttle_qty;
                if(shuttle_qty>max_shuttle) max_shuttle=shuttle_qty;
                if(shuttle_qty<min_shuttle) min_shuttle=shuttle_qty;
                total_income+=amount_today;
                court_fee_pp_last=court_fee;
                court_fee_total+=court_fee;
                if(method==PAY_CASH) paid_cash+=paid_today;
                else if(method==PAY_TRANSFER) paid_transfer+=paid_today;
                paid_from_os+=paid_os;
            }
        }
        fclose(fp);
    }

    int outstanding_this_month=0;
    FILE *fos=fopen("OSpayment.txt","r");
    if(fos){
        char line[512];
        while(fgets(line,sizeof(line),fos)){
            if(!isdigit((unsigned char)line[0])) continue;
            int member_id,gender,os_amount;
            char fullname[NAME_MAXLEN], nickname[NICK_MAXLEN], date[DATE_MAXLEN], note[NOTE_MAXLEN];
            if(sscanf(line,"%d|%127[^|]|%63[^|]|%d|%15[^|]|%d|%127[^\n]",
                      &member_id, fullname, nickname, &gender, date, &os_amount, note)==7){
                if(strlen(date)>=7){
                    if(strncmp(date+3, month_yyyy, strlen(month_yyyy))==0)
                        outstanding_this_month+=os_amount;
                }
            }
        }
        fclose(fos);
    }

    FILE *fo=fopen(out_path,"w");
    if(!fo){
        printf("ไม่สามารถเขียนไฟล์สรุปรายเดือน: %s\n", out_path);
        delay(3);
        return 0;
    }

    fwrite("\xEF\xBB\xBF",1,3,fo);

    fprintf(fo,"เดือน/ปี|%s\n",month_yyyy);
    fprintf(fo,"จำนวนวันที่มีข้อมูล|%d\n", total_days);
    fprintf(fo,"จำนวนลูกทั้งหมด (เดือน)|%d\n", total_shuttle);
    fprintf(fo,"ค่าเฉลี่ยลูก/คน (เดือน)|%d\n", total_players? total_shuttle/total_players : 0);
    fprintf(fo,"ลูกสูงสุด/วัน|%d\n", max_shuttle);
    fprintf(fo,"ลูกต่ำสุด/วัน|%d\n", (min_shuttle==1<<30)?0:min_shuttle);
    fprintf(fo,"จำนวนผู้เล่นรวม|%d\n", total_players);
    fprintf(fo,"ค่าสนามต่อคน (อ้างอิงล่าสุด)|%d\n", court_fee_pp_last);
    fprintf(fo,"ค่าสนามรวมทั้งเดือน|%d\n", court_fee_total);
    fprintf(fo,"ยอดที่ต้องได้รับทั้งเดือน|%d\n", total_income);
    fprintf(fo,"รับจริง (เงินสด)|%d\n", paid_cash);
    fprintf(fo,"รับจริง (โอน)|%d\n", paid_transfer);
    fprintf(fo,"รับจากชำระค้าง|%d\n", paid_from_os);
    fprintf(fo,"รวมรับจริงทั้งหมด|%d\n", paid_cash+paid_transfer+paid_from_os);
    fprintf(fo,"ยอดค้างจ่ายในเดือนนี้|%d\n", outstanding_this_month);
    fclose(fo);
    return 1;
}

/*
 * menu3_choose: เมนูสรุปข้อมูล (รายวัน/รายเดือน)
 * - ไม่มีพารามิเตอร์
 * - พฤติกรรม: ผู้ใช้เลือกว่าจะสรุปรายวันหรือรายเดือน,
 *   โปรแกรมจะแสดงไฟล์สรุปถ้ามี, หรือสร้างแล้วพิมพ์ออกหน้าจอ
 */
void menu3_choose(void){
    int sub;
    system("cls");
    printf("=== เมนูสรุปข้อมูล ===\n");
    printf("1. สรุปข้อมูลรายวัน\n");
    printf("2. สรุปข้อมูลรายเดือน\n");
    printf("0. ย้อนกลับ\n");
    printf("เลือก: ");
    if(scanf("%d",&sub)!=1){
        while(getchar()!='\n' && !feof(stdin)){}
        printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
        delay(2);
        return;
    }
    if(sub==0) return;

    if(sub==1){
        int mode;
        char date[DATE_MAXLEN], inpath[128], out_full[160], out_brief[160];

        printf("กรอกวันที่ (DD-MM-YYYY) (0 = ย้อนกลับ): ");
        if(scanf("%15s",date)!=1){
            while(getchar()!='\n' && !feof(stdin)){}
            printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
            delay(2);
            return;
        }
        if(strcmp(date,"0")==0) return;

        snprintf(inpath,sizeof(inpath),"%s.txt",date);
        snprintf(out_full,sizeof(out_full),"output/%s.สรุปแบบละเอียด.txt",date);
        snprintf(out_brief,sizeof(out_brief),"output/%s.สรุปแบบย่อ.txt",date);

        printf("1. สรุปแบบมีรายชื่อ\n");
        printf("2. สรุปแบบย่อ\n");
        printf("0. ย้อนกลับ\n");
        printf("เลือก: ");
        if(scanf("%d",&mode)!=1){
            while(getchar()!='\n' && !feof(stdin)){}
            printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
            delay(2);
            return;
        }
        if(mode==0) return;

        if(mode==1){
            if(file_exists(out_full)) {
                print_file(out_full);
            }
            else if(file_exists(inpath)) {
                summarize_daily(inpath,1);
                if (!summarize_daily_and_write(inpath,out_full,1)) {
                    printf("บันทึกไฟล์สรุปแบบละเอียดล้มเหลว\n");
                    delay(3);
                }
                else 
                    print_file(out_full);
            }
            else {
                printf("ไม่พบไฟล์รายวัน: %s\n", inpath);
                delay(3);
            }
        }else if(mode==2){
            if(file_exists(out_brief)) {
                print_file(out_brief);
            }
            else if(file_exists(inpath)) {
                summarize_daily(inpath,1);
                if (!summarize_daily_and_write(inpath,out_brief,0)) {
                    printf("บันทึกไฟล์สรุปแบบย่อล้มเหลว\n");
                    delay(3);
                }
                else
                    print_file(out_brief);
            } 
            else {
                printf("ไม่พบไฟล์รายวัน: %s\n", inpath);
                delay(3);
            }
        }else {
            printf("เมนูไม่ถูกต้อง\n");
            delay(3);
        }

    } else if(sub==2){
        char month_yyyy[8], out_path[160];
        printf("กรอกเดือน/ปี (MM-YYYY) (0 = ย้อนกลับ): ");
        if(scanf("%7s",month_yyyy)!=1){
            while(getchar()!='\n' && !feof(stdin)){}
            printf("\nกรอกหมายเลขผิดพลาด โปรดลองอีกครั้ง\n");
            delay(2);
            return;
        }
        if(strcmp(month_yyyy,"0")==0) return;

        snprintf(out_path,sizeof(out_path),"output/%s.สรุปรายเดือน.txt",month_yyyy);

        if(file_exists(out_path)){
            print_file(out_path);
        }else{
            if(!summarize_month_and_write(month_yyyy,out_path)){
                printf("สรุปรายเดือนล้มเหลวหรือไม่พบข้อมูลไฟล์รายวัน (.txt)\n");
                delay(3);
            }else{
                print_file(out_path);
            }
        }
    }
}
