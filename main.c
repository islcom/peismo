#include <stdlib.h>     //exit()
#include <signal.h>     //signal()
//#include <time.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>  // get terminal width
#include <sys/time.h>
#include <pthread.h>

#include "ADS1256.h"

#define ADS1256_TIME_OFFSET 725
#define FILE_BUFFER_LENGTH 120000 // 60 x sps needed for a minute, 120 x sps allows for arbitrary start and finish on minute with no files less than a minute
#define FULLSCALE 16777216
#define MAX_READINGS_PER_SAMPLE 100

int SPS = 0;
int SPS_DEFAULT = 200;
int DISPLAY_GAIN = 0;
int DISPLAY_GAIN_DEFAULT = 10000;
int TERMINAL_WIDTH = 0;

int verbose = 0; // not used yet
int filter = 0;
int display = 0;
int showAllChannels = 0;

int terminal_scale;

unsigned long thisSecond;
unsigned long thisSample;
unsigned long lastSecond=-1;
unsigned long lastSample=-1;
int storing = 0; // becomes 1 once we reach the first sample of a second, at which point we record the SFE
int file_to_write = 0; // becomes 1 once file buffer has data
int readingsInThisSample;
int rawReadings[MAX_READINGS_PER_SAMPLE];
int readings[MAX_READINGS_PER_SAMPLE];

// interim buffer
unsigned int file_buffer_SFE = 0; // seconds from epoch
int file_buffer[FILE_BUFFER_LENGTH] = {0};
int file_buffer_length = -1;
unsigned int interim_buffer_SFE = 0; // seconds from epoch
int interim_buffer[FILE_BUFFER_LENGTH] = {0};
int interim_buffer_length = -1;

float sy1, sy2, sy3, sy4, sy5, sx1, sx2, sx3, sx4, sx5 = 0; // sx1 = most recent sample in, sy1 = most recent filtered sample
// coefficients for a 4 pole butterworth with 3dB cutoff @ 25Hz for 1000 samples per second (ADC raw speed)
float c1 = 32011.292;
float c2 = 114912.018;
float c3 = -155295.607;
float c4 = 93602.702;
float c5 = -21223.822;

void  Handler(int signo) {
    //System Exit
    printf("\r\nEND                  \r\n");
    DEV_ModuleExit();

    exit(0);
}

void ReadingReadyISR() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    UDOUBLE reading = ADS1256_Faster_Read_ADC_Data();
    int signedReading = (((int)reading + (FULLSCALE >> 1)) % FULLSCALE) - (FULLSCALE >> 1);
    if (filter) FilterSample(&signedReading);
    thisSecond = currentTime.tv_sec;
    thisSample = ((currentTime.tv_usec - ADS1256_TIME_OFFSET )* SPS * 2 + 1) / 2000000; // work out which sample within a second this reading belongs to
    if (thisSample>=SPS) { thisSample -= SPS; thisSecond += 1; } // reading rolls into a sample in next second
    if (thisSample!=lastSample) {
        //printf("%d", readingsInThisSample);
        int averagedSample = 0;
        for (int i = 0; i < readingsInThisSample; i++) {
            averagedSample += readings[i];
        }
        if (readingsInThisSample>0) {
            averagedSample /= readingsInThisSample;
            dealWithSample(averagedSample, readingsInThisSample, readings, rawReadings);
        } else
            averagedSample = 0;
        
        readingsInThisSample = 0;
    }
    if (readingsInThisSample < MAX_READINGS_PER_SAMPLE) {
        readings[readingsInThisSample] = signedReading;
        rawReadings[readingsInThisSample] = reading;
        readingsInThisSample++;
    }
    if (thisSecond != lastSecond) {
        //printf(": %d\r\n", lastSecond);
    }
    
    lastSecond = thisSecond;
    lastSample = thisSample;
}

void FilterSample(int *sample) {
    sx5 = sx4; sx4 = sx3; sx3 = sx2; sx2 = sx1;
    sy5 = sy4; sy4 = sy3; sy3 = sy2; sy2 = sy1;
    sx1 = (float)(*sample);
    sy1 = (sx1 + 4 * sx2 + 6 * sx3 + 4 * sx4 + sx5 + c2 * sy2 + c3 * sy3 + c4 * sy4 + c5 * sy5) / c1;
    *sample = (int)sy1;
}

void dealWithSample(int sampleValue, unsigned int readingCount, int readings[], int rawReadings[]) {
    if (!storing && (thisSample==0)) {
        interim_buffer_SFE = thisSecond;
        interim_buffer_length = 0;
        storing = 1;
    }
    if (storing) {
        interim_buffer[interim_buffer_length] = sampleValue + (FULLSCALE / 2);
        interim_buffer_length++;
    }
    if ((interim_buffer_length>=200) && (thisSecond%60==0)) {
		memcpy(file_buffer, interim_buffer, FILE_BUFFER_LENGTH * sizeof *interim_buffer);
		file_buffer_length=interim_buffer_length;
		file_buffer_SFE=interim_buffer_SFE;
		memset(interim_buffer,0,FILE_BUFFER_LENGTH * sizeof *interim_buffer);
		interim_buffer_length=0;
		interim_buffer_SFE=0;	
		file_to_write=1;
	}
    if (display) {
        int min = readings[0];
        int max = readings[0];
        for (int i = 0; i < readingCount; i++) {
            if (readings[i] > max) max = readings[i];
            if (readings[i] < min) min = readings[i];
        }
	    printf("|");
        for (int i = 0; i < TERMINAL_WIDTH-3; i++)
        {
            if ((((min * DISPLAY_GAIN) + FULLSCALE/2) / terminal_scale <= i) && (((max * DISPLAY_GAIN) + FULLSCALE/2) / terminal_scale >= i))
                printf("*");
            else
                printf(" ");
        }
	    printf("|");
        //printf("%06d %06d %06d %06d ", min, max, sampleValue, readingCount);
        for (int i = 0; i < readingCount; i++) {
            //printf("%06d ", readings[i]);
        }
        for (int i = 0; i < readingCount; i++) {
            //printf("%06d ", rawReadings[i]);
        }
        printf("\r\n");
    }
}

void Write_File(void) {
    FILE *outputfile;  // Declare a FILE pointer
    char filename[20];
    sprintf(filename, "data/%d.dat", file_buffer_SFE);
    outputfile = fopen(filename, "w");
    if (outputfile == NULL) {
        fprintf(stderr, "Unable to open the file.\n");
        return;
    }
    time_t rawtime;
	time(&rawtime);
	rawtime = (time_t)file_buffer_SFE;
	struct tm * ptm;
	char date_buf[40];
	ptm = gmtime (&rawtime);
	strftime(date_buf, 40, "%FT%T.000", ptm);
    fprintf(outputfile,"TIMESERIES AU_LILH_0_HHZ, %u samples, %u sps, %s, SLIST, INTEGER, Counts\n", file_buffer_length, SPS, date_buf);
	for (int c=0;c<file_buffer_length;c++){
		fprintf(outputfile,"%u\r\n", file_buffer[c]);
	}
    fclose(outputfile);
    printf("wrote a file \r\n");
}

int main(int argc, char *argv[]) {

    for (int j=0; j<argc; j++) {
        if (!strcmp(argv[j], "-v")) verbose = 1; // not used yet
        if (!strcmp(argv[j], "-f")) filter = 1;
        if (!strcmp(argv[j], "-d")) {
            display = 1;
            if (j<argc-1) DISPLAY_GAIN = atoi(argv[j + 1]);
        };
        if (!strcmp(argv[j], "-a")) showAllChannels = 1;
        if (!strcmp(argv[j], "-sps")) SPS = atoi(argv[j + 1]);
    }
    if (!SPS) {
        SPS = SPS_DEFAULT;
        printf("(default)");
    }
    printf("-sps %d samples per second\r\n", SPS);
    if (display) {
        if (!DISPLAY_GAIN) {
            DISPLAY_GAIN = DISPLAY_GAIN_DEFAULT;
            printf("(default)");
        }
        printf("-d display gain %d\r\n", DISPLAY_GAIN);
    } else {
        printf("no -d display\r\n");
    }

    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    TERMINAL_WIDTH = w.ws_col;
    terminal_scale = (FULLSCALE / (TERMINAL_WIDTH-3))-1;
    printf("terminal width: %d", TERMINAL_WIDTH);

    sleep(3);
    DEV_ModuleInit();

    // Exception handling:ctrl + c
    signal(SIGINT, Handler);

    if(ADS1256_init() == 1){
        printf("\r\nEND                  \r\n");
        DEV_ModuleExit();
        exit(0);
    }
    ADS1256_ConfigADC(ADS1256_GAIN_1, ADS1256_1000SPS);
    ADS1256_WriteCmd(0xFC); // sync
    ADS1256_WriteCmd(0x00); // wake up

    //
    while (showAllChannels) { // if -a flag set, show all channels in single ended mode
        printf("0 : %f\r\n",ADS1256_GetChannalValue(0)*5.0/0x7fffff);
        printf("1 : %f\r\n",ADS1256_GetChannalValue(1)*5.0/0x7fffff);
        printf("2 : %f\r\n",ADS1256_GetChannalValue(2)*5.0/0x7fffff);
        printf("3 : %f\r\n",ADS1256_GetChannalValue(3)*5.0/0x7fffff);
        printf("4 : %f\r\n",ADS1256_GetChannalValue(4)*5.0/0x7fffff);
        printf("5 : %f\r\n",ADS1256_GetChannalValue(5)*5.0/0x7fffff);
        printf("6 : %f\r\n",ADS1256_GetChannalValue(6)*5.0/0x7fffff);
        printf("7 : %f\r\n",ADS1256_GetChannalValue(7)*5.0/0x7fffff);
        printf("\33[8A"); // Move the cursor up 8 lines
    }

    ADS1256_WriteCmd(0x03); // read continuously
    wiringPiISR(DEV_DRDY_PIN, INT_EDGE_FALLING, ReadingReadyISR);
    ADS1256_SetDiffChannal(0);
    while(1){
        sleep(1);
        if (file_to_write) {
            if (file_buffer_SFE!=0)
                Write_File();
            else
                printf("didn't write a file because of 0 SFE\r\n");
            file_buffer_SFE = 0;
            //file_buffer[FILE_BUFFER_LENGTH] = {0};
            file_buffer_length = -1;
            file_to_write = 0;
        }
    }
    return 0;
}
