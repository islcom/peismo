/*****************************************************************************
* | File      	:   ADS1256.c
* | Author      :   Waveshare team
* | Function    :   ADS1256 driver
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2019-03-25
* | Info        :
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "ADS1256.h"
#include <stdio.h>
#include <unistd.h>
#define CLK 11
#define	MOSI 10
#define	MISO 9 
UBYTE ScanMode = 0;

/******************************************************************************
function:   Module reset
parameter:
Info:
******************************************************************************/
void ADS1256_reset(void)
{
    DEV_Digital_Write(DEV_RST_PIN, 1);
    DEV_Delay_ms(200);
    DEV_Digital_Write(DEV_RST_PIN, 0);
    DEV_Delay_ms(200);
    DEV_Digital_Write(DEV_RST_PIN, 1);
}

/******************************************************************************
function:   send command
parameter: 
        Cmd: command
Info:
******************************************************************************/
void ADS1256_WriteCmd(UBYTE Cmd)
{
    DEV_Digital_Write(DEV_CS_PIN, 0);
    DEV_SPI_WriteByte(Cmd);
    DEV_Digital_Write(DEV_CS_PIN, 1);
}

/******************************************************************************
function:   Write a data to the destination register
parameter: 
        Reg : Target register
        data: Written data
Info:
******************************************************************************/
void ADS1256_WriteReg(UBYTE Reg, UBYTE data) // was static
{
    DEV_Digital_Write(DEV_CS_PIN, 0);
    DEV_SPI_WriteByte(CMD_WREG | Reg);
    DEV_SPI_WriteByte(0x00);
    DEV_SPI_WriteByte(data);
    DEV_Digital_Write(DEV_CS_PIN, 1);
}

/******************************************************************************
function:   Read a data from the destination register
parameter: 
        Reg : Target register
Info:
    Return the read data
******************************************************************************/


UBYTE ADS1256_Read_data(UBYTE Reg) // was static
{
    UBYTE temp = 0;
    DEV_Digital_Write(DEV_CS_PIN, 0);
    DEV_SPI_WriteByte(CMD_RREG | Reg);
    DEV_SPI_WriteByte(0x00);
    //DEV_Delay_ms(1);
    DEV_Digital_Write(DEV_CS_PIN, 0);
    temp = DEV_SPI_ReadByte();
    DEV_Digital_Write(DEV_CS_PIN, 1);
    return temp;
}


/******************************************************************************
function:   Waiting for a busy end
parameter: 
Info:
    Timeout indicates that the operation is not working properly.
******************************************************************************/
static void ADS1256_WaitDRDY(void)
{   
    UDOUBLE i = 0;
    for(i=0;i<4000000;i++){
        if(DEV_Digital_Read(DEV_DRDY_PIN) == 0)
            break;
    }
    if(i >= 4000000){
       printf("Time Out ...\r\n"); 
    }
}

/******************************************************************************
function:  Read device ID
parameter: 
Info:
******************************************************************************/


UBYTE ADS1256_ReadChipID(void)
{
    UBYTE id;
    ADS1256_WaitDRDY();
    id = ADS1256_Read_data(REG_STATUS);
    return id>>4;
}


/******************************************************************************
function:  Configure ADC gain and sampling speed
parameter: 
    gain : Enumeration type gain
    drate: Enumeration type sampling speed
Info:
******************************************************************************/
void ADS1256_ConfigADC(ADS1256_GAIN gain, ADS1256_DRATE drate)
{
    ADS1256_WaitDRDY();
    UBYTE buf[4] = {0,0,0,0};
    buf[0] = (0<<3) | (0<<2) | (0<<1);
    buf[1] = 0x01; //0x01 = A0-A1
    buf[2] = (0<<5) | (0<<3) | (gain<<0);
    buf[3] = ADS1256_DRATE_E[drate];
    DEV_Digital_Write(DEV_CS_PIN, 0);
    DEV_SPI_WriteByte(CMD_WREG | 0);
    DEV_SPI_WriteByte(0x03);
    
    DEV_SPI_WriteByte(buf[0]);
    DEV_SPI_WriteByte(buf[1]);
    DEV_SPI_WriteByte(buf[2]);
    DEV_SPI_WriteByte(buf[3]);
    DEV_Digital_Write(DEV_CS_PIN, 1);
    DEV_Delay_ms(1);
}

void ADS1256_SetDiffChannal(UBYTE Channal)
{
    sleep(0.1);
    if (Channal == 0) {
        ADS1256_WriteReg(REG_MUX, (0 << 4) | 1);	//DiffChannal  AIN0-AIN1
    }
    else if (Channal == 1) {
        ADS1256_WriteReg(REG_MUX, (2 << 4) | 3);	//DiffChannal   AIN2-AIN3
    }
    else if (Channal == 2) {
        ADS1256_WriteReg(REG_MUX, (4 << 4) | 5); 	//DiffChannal    AIN4-AIN5
    }
    else if (Channal == 3) {
        ADS1256_WriteReg(REG_MUX, (6 << 4) | 7); 	//DiffChannal   AIN6-AIN7
    }
    sleep(0.1);
    printf(" Setting Diff Channel %u by setting reg %u to %x \r\n", Channal, REG_MUX, ADS1256_Read_data(REG_MUX));

/*
    sleep(0.1);
    if (Channal == 0) {
        ADS1256_WriteReg(REG_MUX, (1 << 4) | 0);	//DiffChannal  AIN0-AIN1
    }
    else if (Channal == 1) {
        ADS1256_WriteReg(REG_MUX, (3 << 4) | 2);	//DiffChannal   AIN2-AIN3
    }
    else if (Channal == 2) {
        ADS1256_WriteReg(REG_MUX, (5 << 4) | 4); 	//DiffChannal    AIN4-AIN5
    }
    else if (Channal == 3) {
        ADS1256_WriteReg(REG_MUX, (7 << 4) | 6); 	//DiffChannal   AIN6-AIN7
    }
    sleep(0.1);
    printf(" Setting Diff Channel %u by setting reg %u to %x \r\n", Channal, REG_MUX, ADS1256_Read_data(REG_MUX));
    */
}

/******************************************************************************
function:  Read ADC data FAST
parameter:
Info:
******************************************************************************/
UDOUBLE ADS1256_Fast_Read_ADC_Data(void) // was static
{
    UDOUBLE read = 0;
    UBYTE buf[3] = { 0,0,0 };

    //ADS1256_WaitDRDY();
    //DEV_Delay_ms(1);
    DEV_Digital_Write(DEV_CS_PIN, 0);
    DEV_SPI_WriteByte(CMD_RDATA);
    DEV_Digital_Write(DEV_CS_PIN, 0); // just for the sake of wasting time. need 7.6uS between CMD Write and data read
    //DEV_Delay_ms(1);
    buf[0] = SPI_ReadByte();
    buf[1] = SPI_ReadByte();
    buf[2] = SPI_ReadByte();
    DEV_Digital_Write(DEV_CS_PIN, 1);
    read = ((UDOUBLE)buf[0] << 16) & 0x00FF0000;
    read |= ((UDOUBLE)buf[1] << 8);  /* Pay attention to It is wrong   read |= (buf[1] << 8) */
    read |= buf[2];
    //printf("%d  %d  %d \r\n",buf[0],buf[1],buf[2]);
    //if (read & 0x800000)
    //    read &= 0xFF000000;
    return read;
}

UDOUBLE ADS1256_Faster_Read_ADC_Data(void) // was static
{
    uint32_t read = 0xFFFFFFFF;
    DEV_Digital_Write(DEV_CS_PIN, 0);
    //DEV_SPI_WriteByte(CMD_RDATA);
    //DEV_Digital_Write(DEV_CS_PIN, 0); // just for the sake of wasting time. need 7.6uS between CMD Write and data read
    int read_data = wiringPiSPIDataRW(0,(unsigned char*)&read, 3);
    (void)read_data; // don't need to use
    DEV_Digital_Write(DEV_CS_PIN, 1);
    read = ((read & 0x0000FF) << 16) |
        ((read & 0x00FF00) << 0) |
        ((read & 0xFF0000) >> 16);
    //printf("raw: %u ", read);
    //if (read & 0x800000)
    //    read |= 0xFF000000;
    //else
        read &= 0x00FFFFFF;
    //printf("sample: %d\n", read);
    return read;
}

/******************************************************************************
function:  Set the channel to be read
parameter: 
    Channal : Set channel number
Info:
******************************************************************************/
static void ADS1256_SetChannal(UBYTE Channal)
{
    if(Channal > 7){
        return ;
    }
    ADS1256_WriteReg(REG_MUX, (Channal<<4) | (1<<3));
} 


/******************************************************************************
function:  Read ADC data
parameter: 
Info:
******************************************************************************/
static UDOUBLE ADS1256_Read_ADC_Data(void)
{
    UDOUBLE read = 0;
    UBYTE buf[3] = {0,0,0};
    
    ADS1256_WaitDRDY();
    DEV_Delay_ms(1);
    DEV_Digital_Write(DEV_CS_PIN, 0);
    DEV_SPI_WriteByte(CMD_RDATA);
    DEV_Delay_ms(1);
    buf[0] = DEV_SPI_ReadByte();
    buf[1] = DEV_SPI_ReadByte();
    buf[2] = DEV_SPI_ReadByte();
    DEV_Digital_Write(DEV_CS_PIN, 1);
    read = ((UDOUBLE)buf[0] << 16) & 0x00FF0000;
    read |= ((UDOUBLE)buf[1] << 8);  /* Pay attention to It is wrong   read |= (buf[1] << 8) */
    read |= buf[2];
    //printf("%d  %d  %d \r\n",buf[0],buf[1],buf[2]);
    if (read & 0x800000)
        read &= 0xFF000000;
    return read;
}

/******************************************************************************
function:  Device initialization
parameter: 
Info:
******************************************************************************/
UBYTE ADS1256_init(void)
{
    ADS1256_reset();
    if(ADS1256_ReadChipID() == 3){
        printf("ID Read success \r\n");
    }
    else{
        printf("ID Read failed \r\n");
        return 1;
    }
    ADS1256_ConfigADC(ADS1256_GAIN_1, ADS1256_30000SPS);
    
    
    // printf("ADD 0x02  = 0x%x\r\n",ADS1256_Read_data(0x02));
    // printf("ADD 0x03  = 0x%x\r\n",ADS1256_Read_data(0x03));
    return 0;
}

/******************************************************************************
function:  Read ADC specified channel data
parameter: 
    Channel: Channel number
Info:
******************************************************************************/
UDOUBLE ADS1256_GetChannalValue(UBYTE Channel)
{
    UDOUBLE Value = 0;
    while(DEV_Digital_Read(DEV_DRDY_PIN) == 1);
    if(ScanMode == 0){// 0  Single-ended input  8 channel1 Differential input  4 channe 
        if(Channel>=8){
            return 0;
        }
        ADS1256_SetChannal(Channel);
        ADS1256_WriteCmd(CMD_SYNC);
        ADS1256_WriteCmd(CMD_WAKEUP);
        Value = ADS1256_Read_ADC_Data();
    }
    else{
        if(Channel>=4){
            return 0;
        }
        ADS1256_SetDiffChannal(Channel);
        ADS1256_WriteCmd(CMD_SYNC);
        ADS1256_WriteCmd(CMD_WAKEUP);
        Value = ADS1256_Read_ADC_Data();
    }
    return Value;
}

/******************************************************************************
function:  Read data from all channels
parameter: 
    ADC_Value : ADC Value
Info:
******************************************************************************/
void ADS1256_GetAll(UDOUBLE *ADC_Value)
{
    UBYTE i;
    for(i = 0; i<8; i++){
        ADC_Value[i] = ADS1256_GetChannalValue(i);
    }

}
