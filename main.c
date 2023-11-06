#include <stdlib.h>     //exit()
#include <signal.h>     //signal()
#include <time.h>
#include <string.h>
#include "stdio.h"
#include <time.h>

#include "ADS1256.h"

int verbose = 0;
int sampleNumber = 0;

void  Handler(int signo)
{
    //System Exit
    printf("\r\nEND                  \r\n");
    DEV_ModuleExit();

    exit(0);
}

void SampleReadyISR() {
    sampleNumber++;
    if (sampleNumber>=100){
        //time_t currentTime = time(NULL);
        struct timeval currentTime;
        gettimeofday(&currentTime, NULL);
        printf("%ld:%06ld\n", currentTime.tv_sec, currentTime.tv_usec);
        sampleNumber=0;
    }
}

int main(int argc, char *argv[])
{
    for (int j=0; j<argc; j++) {
        if (!strcmp(argv[j], "-v")) verbose = 1;
    }
    UDOUBLE ADC[8],i;
    float x;
    
    printf("demo\r\n");
    DEV_ModuleInit();
    
    // Exception handling:ctrl + c
    signal(SIGINT, Handler);

    if(ADS1256_init() == 1){
        printf("\r\nEND                  \r\n");
        DEV_ModuleExit();
        exit(0);
    }
    ADS1256_SetDiffChannal(0);
    ADS1256_ConfigADC(ADS1256_GAIN_1, ADS1256_1000SPS);
    ADS1256_WriteCmd(0xFC); // sync
    ADS1256_WriteCmd(0x00); // wake up
    ADS1256_WriteCmd(0x03); // read continuously
    wiringPiISR(DEV_DRDY_PIN, INT_EDGE_FALLING, SampleReadyISR);
    while(1){
        
        // printf("0 : %f\r\n",ADS1256_GetChannalValue(0)*5.0/0x7fffff);
        // printf("1 : %f\r\n",ADS1256_GetChannalValue(1)*5.0/0x7fffff);
        // printf("2 : %f\r\n",ADS1256_GetChannalValue(2)*5.0/0x7fffff);
        // printf("3 : %f\r\n",ADS1256_GetChannalValue(3)*5.0/0x7fffff);
        // printf("4 : %f\r\n",ADS1256_GetChannalValue(4)*5.0/0x7fffff);
        // printf("5 : %f\r\n",ADS1256_GetChannalValue(5)*5.0/0x7fffff);
        // printf("6 : %f\r\n",ADS1256_GetChannalValue(6)*5.0/0x7fffff);
        // printf("7 : %f\r\n",ADS1256_GetChannalValue(7)*5.0/0x7fffff);
        
        /*
        ADS1256_GetAll(ADC);
        for(i=0;i<8;i++){
            printf("%d %f\r\n",i,ADC[i]*5.0/0x7fffff);
        }
        

        x = (ADC[0] >> 7)*5.0/0xffff;
        printf(" %f \r\n", x);
        printf("\33[9A");//Move the cursor up 8 lines
	*/
    }
    return 0;
}
