#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

typedef struct 
{
    char *name;
    char *surname;
    char *nickname;
    char *memid;
} member;


void delay(int number_of_seconds)
{
	// Converting time into milli_seconds
	int milli_seconds = 1000 * number_of_seconds;

	// Storing start time
	clock_t start_time = clock();

	// looping till required time is not achieved
	while (clock() < start_time + milli_seconds);
}

void registerMember(FILE *memberfiles,member *m, int *count) {
    *count++;
    char buff[50];
    printf("ชื่อ:"); scanf("%s", buff);
    m[*count].name = malloc(strlen(buff) +1); 
    strcpy(m[*count].name, buff);

    printf("นามสกุล:"); scanf("%s", buff);
    m[*count].surname = malloc(strlen(buff) +1); 
    strcpy(m[*count].surname, buff);

    printf("ชื่อเล่น:"); scanf("%s", buff);
    m[*count].nickname = malloc(strlen(buff) +1); 
    strcpy(m[*count].nickname, buff);
    
    *m[*count].memid = *m[*count-1].memid + 1;
    printf("\n\tเลขสมาชิกของคุณคือ %d", *m[*count].memid);

    delay(3);
}
 
void input_file(){ 
    FILE *memberfiles;
    char mem[100], buff[5][35];
    int menu1;
    int count = 0;
    int capacity = 10;
    member *m = malloc(capacity * sizeof(member));
    memberfiles = fopen("member.txt","r");
    if (memberfiles != NULL) {
        while (fscanf(memberfiles, "%s %s %s %s", buff[0], buff[1], buff[2], buff[3]) == 4) { // 1 -> name / 2 -> surname / 3-> nickname / 4-> memid
            m[count].name = malloc(strlen(buff[0]) + 1); strcpy(m[count].name, buff[0]);
            m[count].surname = malloc(strlen(buff[1]) + 1); strcpy(m[count].name, buff[1]);
            m[count].nickname = malloc(strlen(buff[2]) + 1); strcpy(m[count].name, buff[2]);
            m[count].memid = malloc(strlen(buff[3]) + 1); strcpy(m[count].name, buff[3]);
            count++;
            printf("%s", buff[0]);
        }
    }
    else if (memberfiles == NULL) {
        printf("FALIED OPEN. PLS TRY AGAIN");
        
    }
    delay(2); // Delay function (maybe we need to use);
    
    do {
        system("cls");
        printf("\n1.ลงทะเบียนสมาชิกก๊วน\n");
        printf("2.ค้นหารายชื่อในระบบ\n");
        printf("3.รายชื่อสมาชิกทั้งหมด\n");
        printf("กรอกหมายเลขตามเมนูที่ต้องการ : ");
        scanf("%d", &menu1);
        if (menu1 == 1) {
           registerMember(memberfiles,m,&count);
        }
        else if (menu1 == 2) {
        
        }
        else if (menu1 == 3) {
        
        }
    } while (menu1 != 0);
    return;
}