#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "lcd.c"

############################

#define GPIO_PATH "/sys/class/gpio"
#define GPIO_LENGTH 10

int main(void){

        //set_init();

        int shmid1;
        int shmid2;

        int *cal_num1;
        void *shared_memory1 = (void *)0;

 	int *cal_num2;
        void *shared_memory2 = (void *)0;

        //lcd <- touch
        shmid1 = shmget((key_t)1234,1028,066|IPC_CREAT);
        shared_memory1 = shmat(shmid1,(void *)0,SHM_RDONLY);
        cal_num1= (int *)shared_memory1;

        //lcd < -tempture
        shmid2 = shmget((key_t)1235,1028,066|IPC_CREAT);
        shared_memory2 = shmat(shmid2,(void *)0,SHM_RDONLY);
        cal_num2 = (int *)shared_memory2;

        if(fork() == 0){

//----------------------------TOUCH AND BUZZER

                //---- shared set ---
                shmid1 = shmget((key_t)1234,1028,IPC_CREAT);
                shared_memory1 = shmat(shmid1,(void *)0,066|IPC_CREAT);
		cal_num1 = (int*)shared_memory1;

                // ------------------

                FILE *touch, *tmp,*buz;
                unsigned int value, b=0;
                int count_time = 0;
                //int count_p =0;

                tmp = fopen(GPIO_PATH "/export" ,"w");
                fprintf(tmp, "61");
                fclose(tmp);

                touch = fopen(GPIO_PATH "/gpio61/direction","w");
                fprintf(touch,"in");
                fclose(touch);

                buz = fopen(GPIO_PATH "/export" ,"w");
                fprintf(buz, "27");
                fclose(buz);

		buz = fopen(GPIO_PATH "/gpio27/direction","w");
                fprintf(buz,"out");
                fclose(buz);

                while(1){
                        touch = fopen(GPIO_PATH "/gpio61/value","r");
                        buz = fopen(GPIO_PATH "/gpio27/value","w");
                        fscanf(touch,"%d",&value);

                        if(value == 1 && b != value){
                                count_time += 100;
                        }
                        b=value;
                        fclose(touch);

                        if ( count_time <= 0 ) {
                                count_time = 0;
                                fprintf(buz,"0");
                                fclose(buz);
                        }else {
                                int count_p = count_time/10;
                                count_time -= 1;
                                if ( count_p <= 5 && count_p >=1 ) {
                                        printf("buzzer\n");
                                        fprintf(buz,"1");
                                }else{
                                        fprintf(buz,"0");
                                }
                                //shared
                                *cal_num1 = count_p;
                                //

                                usleep(100000);
                                fclose(buz);
                        }
                }
//----------------------------------------------------------------------

  	}else{


                if(fork()==0){
                //      tempature and LED---------------------------

                        shmid2 = shmget((key_t)1235,1028,IPC_CREAT);
                        shared_memory2 = shmat(shmid2,(void *)0,066|IPC_CREAT);
                        cal_num2 = (int *)shared_memory2;

                        FILE *red,*yel,*gre;

                        red = fopen(GPIO_PATH "/export" ,"w");
                        fprintf(red, "49");
                        fclose(red);

                        red = fopen(GPIO_PATH "/gpio49/direction","w");
                        fprintf(red,"out");
                        fclose(red);

                        yel = fopen(GPIO_PATH "/export" ,"w");
                        fprintf(yel, "51");
                        fclose(yel);

			  yel = fopen(GPIO_PATH "/gpio51/direction","w");
                        fprintf(yel,"out");
                        fclose(yel);

                        gre = fopen(GPIO_PATH "/export" ,"w");
                        fprintf(gre, "50");
                        fclose(gre);

                        gre = fopen(GPIO_PATH "/gpio50/direction","w");
                        fprintf(gre,"out");
                        fclose(gre);

                        while(1){
                                char tempc[100];
                                FILE *vfp;
                                int temp=0;

                                red = fopen(GPIO_PATH "/gpio49/value","w");
                                yel = fopen(GPIO_PATH "/gpio51/value","w");
                                gre = fopen(GPIO_PATH "/gpio50/value","w");

                                vfp = fopen("/sys/devices/w1_bus_master1/28-0004
75d85dff/w1_slave","r");

				fgets(tempc,sizeof(tempc)-1,vfp);
                                fgets(tempc,sizeof(tempc)-1,vfp);

                                //LED funtion
                                sscanf(tempc+29,"%d",&temp);

                                //change int to char

                                if(temp/1000 < 28){
                                        fprintf(red,"1");
                                        fprintf(yel,"0");
                                        fprintf(gre,"0");

                                }else if(28<=temp/1000 && temp/1000 < 30){
                                        fprintf(red,"0");
                                        fprintf(yel,"1");
                                        fprintf(gre,"0");
                                }else{
                                        fprintf(red,"0");
                                        fprintf(yel,"0");
                                        fprintf(gre,"1");
  				}	

                               	fclose(red);
                                fclose(yel);
                                fclose(gre);

                                *cal_num2=temp;
                                usleep(100000);
                        }
                //-----------------------------------
                }else{

//                                       RS  E   D0  D1  D2  D3
                        char gpio[10] = {47, 46, 23, 26, 45, 44,
//                                       D4  D5  D6  D7
                                        69, 68, 66, 67};

                        FILE *gpio_fp[10];
                        FILE *tmp_fp;

                        char tmp_path[255];
                        int i;
				
			char *string1,*string2;

                        for (i = 0; i < GPIO_LENGTH; i++){
                                tmp_fp = fopen(GPIO_PATH "/export", "w");
                                fprintf(tmp_fp, "%d", gpio[i]);
                                fclose(tmp_fp);

                                sprintf(tmp_path, GPIO_PATH "/gpio%d/direction",
 gpio[i]);
                                tmp_fp = fopen(tmp_path, "w");
                                fprintf(tmp_fp, "out");
                                fclose(tmp_fp);

                                sprintf(tmp_path, GPIO_PATH "/gpio%d/value", gpi
o[i]);
                                gpio_fp[i] = fopen(tmp_path, "w");
                        }
			 // LCD INITIALIZATION
                        lcd_init(gpio_fp);
                        while(1){


                //#sharedmemory
                                string1=(char *)malloc(sizeof(char)*16);
                                string2=(char *)malloc(sizeof(char)*16);

                                if(*cal_num1<10){
                                sprintf(string1,"Timer = 0%d",*cal_num1);
                                }else{
                                sprintf(string1,"Timer = %d",*cal_num1);
                                }
                                //printf("cal_num1 : %d\n",*cal_num1);
                                //printf("string1 : %s",string1);
                                lcd_send(FIRST_LINE ,CMD_MODE);
                                lcd_write(string1);


                                sprintf(string2,"Tempc = %d.%d",*cal_num2/1000,*
cal_num2%1000);
	
 //printf("cal_num2 : %d\n",*cal_num2);
                                lcd_send(SECOND_LINE, CMD_MODE);
                                lcd_write(string2);
                                usleep(100000);
                                free(string1);
                                free(string2);
                        }

                        tmp_fp = fopen(GPIO_PATH "/unexport", "w");
                        for (i = 0; i < GPIO_LENGTH; i++){
                                fclose(gpio_fp[i]);
                                fprintf(tmp_fp, "%d", gpio[i]);
                        }

                        fclose(tmp_fp);
                }
        }
}

