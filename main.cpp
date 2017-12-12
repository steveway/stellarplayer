//#define _GNU_SOURCE

#include "stdio.h"
#include <stdint.h>
#include "mbed.h"
#undef MBED_RTOS_SINGLE_THREAD
#include "ArduinoAPI.h"
#include "ESP8266.h"
//#include "player.c"
#define PLAYERSRC 1
//#define _cplusplus 1
#include "global.h"
//#undef _cplusplus
#undef PLAYERSRC
//#ifndef MBED_H

//#endif

#include "cmsis_os.h"
#include "rtos.h"
#include "string"
#include "Arial12x12.h"
#include "Arial24x23.h"
//#include "Terminal6x8.h"
//#include "Arial43x48_numb.h"
//#include "pict.h"
//#include "pavement_48x34.h"
//#include "modplayer.h"
#include "FastPWM.h"
extern "C" {
//#include "diskio.h"

#include "mod32.h"
#include "s3m32.h"
}
//#include "IST3020.h"
//#include "UC1608.h"
//#include "ST7565.h"
//#include "ILI932x.h"
#include "ILI9341.h"
//#include "ILI9486.h"
//#include "TFT_MIPI.h"
//#include "SSD1306.h"
//#include "BUS8.h"

#define wifienabled 1
#include "TouchPanel.h"
#include "SDFileSystem.h"
extern void setup(void);
SDFileSystem sd(D11, D12, D13, D10, "sd",NC,SDFileSystem::SWITCH_NONE, 48000000);

//#include "ff.cpp"
#ifdef wifienabled
ArduinoSerial esp_uart(PC_10, PC_11);
ESP8266 wifi(esp_uart);
#endif
Serial pc(USBTX, USBRX);
FATFS_DIR  dir;          // The DIR structure is used for the work area to read a directory by f_oepndir, f_readdir function

PinName buspins[8]= {D8,D9,D2,D3,D4,D5,D6,D7};
//IST3020 myLCD(PAR_8, PortC, PC_8, PC_9, PA_0, PA_1, PA_4,"myLCD", 192, 64); // Parallel 8bit, Port, CS, reset, A0, WR, RD for F302
//UC1608 myLCD(SPI_16, 10000000, D11, D12, D13, D10, D9, D8,"myLCD", 240, 120); // Spi 16bit, 10MHz, mosi, miso, sclk, cs, reset, dc
//ST7565 myLCD(PAR_8, PortC, PC_8, PC_9, PA_0, PA_1, PA_4,"myLCD", 128, 64); // Parallel 8bit, Port, CS, reset, A0, WR, RD
//ILI932x myLCD(PAR_8, PortC, PH_0, PH_1, PA_0, PA_1, PA_4,"myLCD"); // Parallel 8bit, Port, CS, reset, RS, WR, RD for F401
//ILI932x myLCD(SPI_16, 10000000, D11, D12, D13, D10, D9,"myLCD"); // Spi 16bit, 12MHz, mosi, miso, sclk, cs, reset
//ILI9341 myLCD(SPI_16, 12000000, D11, D12, D13, D10, D9, D8,"myLCD"); // Spi 16bit, 12MHz, mosi, miso, sclk, cs, reset, dc
ILI9341 myLCD(BUS_8, buspins, A3, A4, A2, A1, A0,"myLCD"); // Parallel 16bit, Port, CS, reset, DC, WR, RD for F401
//ILI9486 myLCD(PAR_16, PortC, PH_0, PH_1, PA_0, PA_1, PA_4,"myLCD"); // Parallel 16bit, Port, CS, reset, DC, WR, RD for F401
//TFT_MIPI myLCD(PAR_16, PortC, PH_0, PH_1, PA_0, PA_1, PA_4,"myLCD"); // Parallel 16bit, Port, CS, reset, DC, WR, RD for F401
//SSD1306 myLCD(SPI_16, 10000000, D11, D12, D13, D10, D9, D8,"myLCD", 240, 120); // Spi 16bit, 10MHz, mosi, miso, sclk, cs, reset, dc
//ILI9486 myLCD(SPI_8, 12000000, PA_7, PA_6, PA_5, PB_6, PA_8, PA_9,"myLCD"); // Spi 16bit, 12MHz, mosi, miso, sclk, cs, reset, dc for minimaple
//ILI9341 myLCD(SPI_8, 12000000, PA_7, PA_6, PA_5, PB_6, PA_8, PA_9,"myLCD"); // Spi 16bit, 12MHz, mosi, miso, sclk, cs, reset, dc for minimaple
char orient=3;
volatile int playstatus=0;
//touch_tft myLCD(BUS_8, buspins,D6,A4,A1,D7,A3, A4, A2, A1, A0,"myLCD"); // x+,x-,y+,y-,mosi, miso, sclk, cs, reset
//TouchPanel tt(D6,A4,A1,D7);
//Timer t;
typedef struct {
    int16_t x,y;
} points;
typedef struct {
    uint16_t x,y;
} upoints;
//volatile points p;
volatile upoints ap;
volatile points orp;
/*points TouchCallback(unsigned short x, unsigned short y){
    points retval;
    retval.x = x;
    retval.y = y;
    p.x = x;
    p.y = y;
    return retval;
    }
SetTouchCallbackFunction(TouchCallback);*/
//unsigned short backgroundcolor=White;
//unsigned short foregroundcolor=Black;
unsigned short backgroundcolor=Black;
unsigned short foregroundcolor=White;
#define corners  9
#define GREEN_MASK 0x7e0 // 0000011111100000
#define RED_MASK 0xf800 // 1111100000000000
#define CONVERT_888RGB_TO_565RGB(r, g, b) ((b >> 3) | ((g << 3) & GREEN_MASK) | ((r << 8) & RED_MASK))
/*
#define playcorners 10
#define stopcorners 10
#define backcorners 13
#define forwardcorners 13

volatile float playbx[] = {7,36,36,61,82,61,36,36,7,7};
volatile float playby[] = {46,46,68,77,115,153,160,183,183,46};

volatile float stopbx[] = {285,310,310,285,285,253,236,250,285,285};
volatile float stopby[] = {46,46,183,183,160,150,115,83,69,46};

volatile float backbx[] = {020,300,300,282,282,253,238,082,066,037,037,020,020};
volatile float backby[] = {010,010,045,045,068,078,100,100,080,068,045,045,010};

volatile float forwardbx[] = {020,036,036,065,079,238,252,280,282,300,300,020,020};
volatile float forwardby[] = {186,186,161,151,132,132,148,161,181,186,230,230,186};
*/
#define buttonsetup 3

#if ( buttonsetup == 0)
#define playcorners 9
 volatile const float playbx[] =    { 10, 78, 78, 50, 50, 78, 78, 10, 10};
 volatile const float playby[] =    { 60, 60,100,100,140,140,180,180, 60};
#elif (buttonsetup == 1 )
#define playcorners 11
 volatile const float playbx[] =    {  7, 36, 36, 61, 82, 61, 36, 36,  7,  7};
 volatile const float playby[] =    { 46, 46, 68, 77,115,153,160,183,183, 46};
#elif (buttonsetup == 2)
#define playcorners 5
 volatile const float playbx[] =    {264,314,314,264,264};
 volatile const float playby[] =    {  6,  6, 54, 54,  6};
#elif (buttonsetup == 3)
#define playcorners 5
 volatile const float playbx[] =    {215,315,315,215,215};
 volatile const float playby[] =    {110,110,170,170,110};
#endif

#if (buttonsetup == 0)
#define stopcorners 9
 volatile const float stopbx[] =    {240,310,310,240,240,270,270,240,240};
 volatile const float stopby[] =    { 60, 60,180,180,140,140,100,100, 60};
#elif (buttonsetup == 1)
#define stopcorners 11
 volatile const float stopbx[] =    {285,310,310,285,285,253,236,250,285,285};
 volatile const float stopby[] =    { 46, 46,183,183,160,150,115, 83, 69, 46};
#elif (buttonsetup == 2)
#define stopcorners 5
 volatile const float stopbx[] =    {264,314,314,264,264};
 volatile const float stopby[] =    {  6,  6, 54, 54,  6};
#elif (buttonsetup == 3)
#define stopcorners 5
 volatile const float stopbx[] =    {110,210,210,110,110};
 volatile const float stopby[] =    {110,110,170,170,110};
#endif

#if (buttonsetup == 0)
#define backcorners 9
 volatile const float backbx[] =    { 10,310,310,239,239, 80, 80, 10, 10};
 volatile const float backby[] =    { 10, 10, 60, 60,110,110, 59, 59, 10};
#elif (buttonsetup == 1)
#define backcorners 13
 volatile const float backbx[] =    { 20,300,300,282,282,253,238, 82, 66, 37, 37, 20, 20};
 volatile const floatv backby[] =    { 10, 10, 45, 45, 68, 78,100,100, 80, 68, 45, 45, 10};
#elif (buttonsetup == 2)
#define backcorners 5
 volatile const float backbx[] =    {211,259,259,211,211};
 volatile const float backby[] =    { 61, 61,109,109, 61};
#elif (buttonsetup == 3)
#define backcorners 5
 volatile const float backbx[] =    {110,210,210,110,110};
 volatile const float backby[] =    {175,175,235,235,175};
#endif

#if (buttonsetup == 0)
#define forwardcorners 9
 volatile const float forwardbx[] = { 10, 80, 80,239,239,310,310, 10, 10};
 volatile const float forwardby[] = {181,181,135,135,181,181,230,230,181};
#elif (buttonsetup == 1)
#define forwardcorners 13
 volatile const float forwardbx[] = { 20, 36, 36, 65, 79,238,252,280,282,300,300, 20, 20};
 volatile const float forwardby[] = {186,186,161,151,132,132,148,161,181,186,230,230,186};
#elif (buttonsetup == 2)
#define forwardcorners 5
 volatile const float forwardbx[] = {266,314,314,266,266};
 volatile const float forwardby[] = { 61, 61,109,109, 61};
#elif (buttonsetup == 3)
#define forwardcorners 5
 volatile const float forwardbx[] = {215,315,315,215,215};
volatile const float forwardby[] = {175,175,235,235,175};
#endif


char *appendchar(char *szString, size_t strsize, char c)
{
    size_t len = strlen(szString);
    if((len+1) < strsize)
    {
        szString[len++] = c;
        szString[len] = '\0';
        return szString;
    }
    return NULL;
}
//play.c


//static uint16_t modFileNumber = 0;

//Define file format handlers
FileHandler g_fhHandlers[]= {{".MOD",mod_player,mod_mixer,loadMod,mod_getSamplesPerTick},
    {".S3M",s3m_player,s3m_mixer,loadS3m,s3m_getSamplesPerTick}
};

//FileHandler g_fhHandlers[]={{".MOD",mod_player,mod_mixer,loadMod,mod_getSamplesPerTick}};

char easytolower(char in)
{
    if(in<='Z' && in>='A')
        return in-('Z'-'z');
    return in;
}

char *strcasestr(const volatile char *s1, const volatile char *s2)
{
    register const volatile char *s = s1;
    register const volatile char *p = s2;

    do {
        if (!*p) {
            return (char *) s1;;
        }
        if ((*p == *s)
                || (easytolower(*((unsigned char *)p)) == easytolower(*((unsigned char *)s)))
           ) {
            ++p;
            ++s;
        } else {
            p = s2;
            if (!*s) {
                return NULL;
            }
            s = ++s1;
        }
    } while (1);
}

volatile FileHandler *g_pCurrentHandler;

void player(void)
{
    if(g_pCurrentHandler!=NULL)
        (*(g_pCurrentHandler->player))();
}

void mixer(void)
{
    if(g_pCurrentHandler!=NULL)
        (*(g_pCurrentHandler->mixer))();
}

uint16_t getSamplesPerTick()
{
    if(g_pCurrentHandler!=NULL)
        return (*(g_pCurrentHandler->getSamplesPerTick))();
    return 0;
}

FileHandler *getHandler(const volatile char *fileName)
{
    int i;
    if(fileName==NULL)
        return NULL;
    if(fileName[0]=='.' && fileName[1]==NULL)
        return NULL;

    if(fileName[0]=='.' && fileName[1]=='.' && fileName[2]==NULL)
        return NULL;

    for(i=0; i<sizeof(g_fhHandlers); i++) {
        if(strcasestr(fileName,g_fhHandlers[i].szFileExt)!=NULL) {
            return &(g_fhHandlers[i]);
        }
    }
    return NULL;
}

osMutexId lcd_mutex;
osMutexDef(lcd_mutex);

osMutexId loadfile_mutex;
osMutexDef(loadfile_mutex);
volatile uint8_t updatedfile = 0;

void loadNextFile()
{
    //UARTprintf("\033[2J");
    //UARTprintf("\033[0;0H");
    osMutexWait(loadfile_mutex, osWaitForever);
    nextprevfile = 0;
    volatile char *dotPointer;
    g_pCurrentHandler = NULL;

    do {
        f_readdir(&dir, &fileInfo);

        if(strcmp(fileInfo.fname ,"GAME18~1.MOD")==0) {
            //f_unlink(fileInfo.fname);
            f_readdir(&dir, &fileInfo);
        }
        if(strcmp(fileInfo.fname ,"LOST.DIR")==0) {
            //f_unlink(fileInfo.fname);
            f_readdir(&dir, &fileInfo);
        }
        if (fileInfo.fname[0] == 0)
            break;
        dotPointer = strrchr(fileInfo.fname, '.');
        if (dotPointer != NULL) {
            g_pCurrentHandler = getHandler(dotPointer);
            if (g_pCurrentHandler != NULL)
                break;
        }

    } while (dotPointer == NULL || g_pCurrentHandler == NULL);
    /*osMutexWait(lcd_mutex, osWaitForever);
    myLCD.locate(140,220);
    myLCD.printf("Filename: %s\n",fileInfo.fname);
    osMutexRelease(lcd_mutex);*/
    if (fileInfo.fname[0] != 0 && g_pCurrentHandler!=NULL) {
        f_open(&file, fileInfo.fname, FA_READ);
        /*osMutexWait(lcd_mutex, osWaitForever);
        myLCD.locate(20,200);
        myLCD.printf("[%d]: Opened File: %s with handler: [%s]\n",modFileNumber, fileInfo.fname,g_pCurrentHandler->szFileExt);
        osMutexRelease(lcd_mutex);*/
        modFileNumber++;
        (*(g_pCurrentHandler->loader))();
        //myLCD.locate(0,20);
        //if(g_pCurrentHandler->szFileExt[0] == ".S3M"){
        updatedfile = 1;
        osMutexWait(lcd_mutex, osWaitForever);
        myLCD.locate(0,20);
        if(strcmp( (const char *)g_pCurrentHandler->szFileExt ,".S3M")==0) {
            //g_pCurrentHandler->
            //myLCD.printf("Song length: %s",uMod.S3m.songLength);
            //myLCD.printf("S3M : %d",(const char *)uMod.S3m.songLength);
        } else {
            //myLCD.printf("MOD : %s",(const char *)uMod.Mod.name);
            //myLCD.printf("Song length: %d",uMod.Mod.songLength);

        }
        osMutexRelease(lcd_mutex);
        //UARTprintf("Song name: [%s] patterns: %d\n", uMod.S3m.name,
        //      uMod.S3m.songLength);
    } else {
        //UARTprintf("Can't open any files\n");
    }
    osMutexRelease(loadfile_mutex);
}

void loadPreviousFile()
{
    osMutexWait(loadfile_mutex, osWaitForever);
    //UARTprintf("\033[2J");
    //UARTprintf("\033[0;0H");
    nextprevfile = 1;
    volatile uint16_t i;
    volatile char *dotPointer;
    //uint16_t modFileNumber = 2; // deletmelater
    //modFileNumber = 2;
    if(modFileNumber > 1) {
        modFileNumber--;
        f_readdir(&dir, NULL);
        for(i = 0; i < modFileNumber; i++) {
            do {
                f_readdir(&dir, &fileInfo);
                dotPointer = strrchr(fileInfo.fname, '.');
                if(dotPointer!=NULL) {
                    g_pCurrentHandler = getHandler(fileInfo.fname);

                }
            } while(dotPointer == NULL || g_pCurrentHandler == NULL);
            /*osMutexWait(lcd_mutex, osWaitForever);
            myLCD.locate(140,220);
            myLCD.printf("Filename: %s\n",fileInfo.fname);
            osMutexRelease(lcd_mutex);*/
        }
        f_open(&file, fileInfo.fname, FA_READ);
        //UARTprintf("Opened File: %s ",fileInfo.fname);
        (*(g_pCurrentHandler->loader))();
        updatedfile = 1;
        //UARTprintf("Song name: [%s]\n",uMod.S3m.name);
    }
    osMutexRelease(loadfile_mutex);
}

void loadRandomFile(uint8_t skips)
{
    osMutexWait(loadfile_mutex, osWaitForever);
    //UARTprintf("\033[2J");
    //UARTprintf("\033[0;0H");
    nextprevfile = 0;
    volatile char *dotPointer;
    volatile uint8_t i;
    g_pCurrentHandler = NULL;

    do {
        for(i=0; i<skips; i++) {
            f_readdir(&dir, &fileInfo);
        }
        if (fileInfo.fname[0] == 0)
            break;
        dotPointer = strrchr(fileInfo.fname, '.');
        if (dotPointer != NULL) {
            g_pCurrentHandler = getHandler(dotPointer);
            if (g_pCurrentHandler != NULL)
                break;

        }
    } while (dotPointer == NULL || g_pCurrentHandler == NULL);
    /*osMutexWait(lcd_mutex, osWaitForever);
    myLCD.locate(140,220);
    myLCD.printf("Filename: %s\n",fileInfo.fname);
    osMutexRelease(lcd_mutex);*/


    if (fileInfo.fname[0] != 0 && g_pCurrentHandler!=NULL) {
        f_open(&file, fileInfo.fname, FA_READ);

        //UARTprintf("Opened File: %s with handler: [%s]\n", fileInfo.fname,g_pCurrentHandler->szFileExt);
        modFileNumber+= skips;
        (*(g_pCurrentHandler->loader))();
        updatedfile = 1;
        //UARTprintf("Song name: [%s] paterns: %d\n", uMod.S3m.name,
        //      uMod.S3m.songLength);
    } else {
        //UARTprintf("Can't open any files\n");
    }
    osMutexRelease(loadfile_mutex);
}

//enplayer.c

volatile uint8_t touchstate = 0; // 1 downnow, 0 upnow, 2 downstill, 3 upstill
volatile uint8_t pressedbutton = 0; // 0 nothing, 1 play, 2 stop, 3 forward, 4 back
volatile uint8_t laststate = 1;
volatile WORD isampl = 0;

void drawpoly(int npol, volatile const float  xp[], volatile const float  yp[], int x, int y)
{
    //schleife von 0 bis <npol
    //line von x+xp[schleifenwert] y+yp[schleifenwert] bis x+xp[schleifenwer+1] y+yp[schleifenwert+1]
}


int pnpoly(int npol,volatile const float xp[],volatile const float  yp[], int x, int y)
{

    volatile int i, j, c= 0;
    volatile float px, py;
    px = x;
    py = y;

    for (i = 0, j = npol-1; i < npol; j = i++) {
        //myLCD.printf("Y: %.0f\n\r",yp[i]);
        //myLCD.printf("X: %.0f\n\r",xp[i]);

        if ((((float(yp[i]) <= py) && (py < float(yp[j]))) ||
                ((float(yp[j]) <= py) && (py < float(yp[i])))) &&
                (px < (float(xp[j]) - float(xp[i])) * (py - float(yp[i])) / (float(yp[j]) - float(yp[i])) + float(xp[i])))
            c = !c;
    }
    //pc.printf("--\n\r");
    return c;
}



//PwmOut leftchan(PC_8);
FastPWM leftchan(PC_8);
FastPWM rightchan(PC_6);
//PwmOut left(LED1);
//PwmOut rightchan(PC_6);

int timeri = 0;
void fillsoundbuffer(void const *data)
{
    while(1) {

        //timeri++;
        if(SoundBuffer.writePos != SoundBuffer.readPos) {
            //Sound output
            leftchan.write(SoundBuffer.left[SoundBuffer.readPos]);
            rightchan.write(SoundBuffer.right[SoundBuffer.readPos]);
            //TimerMatchSet(TIMER2_BASE, TIMER_B, SoundBuffer.left[SoundBuffer.readPos]); // PWM
            //TimerMatchSet(TIMER2_BASE, TIMER_A, SoundBuffer.right[SoundBuffer.readPos]); // PWM

            //Visualizer
            //RED led
            //TimerMatchSet(TIMER0_BASE, TIMER_B, (SoundBuffer.left[SoundBuffer.readPos]-850)<<5);
            //Blue led
            //TimerMatchSet(TIMER1_BASE, TIMER_A, (SoundBuffer.right[SoundBuffer.readPos]-850)<<5);
            SoundBuffer.readPos++;
            SoundBuffer.readPos &= SOUNDBUFFERSIZE - 1;
        }
        //Thread::wait(500);

        //wait_us(5);
        //if(osMutexWait(lcd_mutex, 0)==0) {
        //    myLCD.locate(0,30);
        //    myLCD.printf("Soundbuffer: %08d",SoundBuffer.readPos);
        //}
        //osMutexRelease(lcd_mutex);
        osDelay(20);

    }
}

volatile int timer2i = 0;



void inputthread(void const *data)
{

    while(1) {
        osMutexWait(lcd_mutex, osWaitForever);
        myLCD.BusEnable(false);
        ap.x = readTouchPanelX_Analog();
        ap.y = readTouchPanelY_Analog();
        //p.y  = simpleypos();
        //p.x = simplexpos();
        orp.x = getTouchPanelPosX();
        orp.y = getTouchPanelPosY();
        myLCD.BusEnable(true);
        //myLCD.locate(120,110);
        //myLCD.printf("Y: %7d",ap.y);
        //myLCD.locate(120,118);
        //myLCD.printf("X: %7d",ap.x);
        osMutexRelease(lcd_mutex);

        if((abs(ap.x-30000)>100)&&ap.x<63500) {
            if(orp.x != -1) {
                if(orp.y != -1) {


                    if(pnpoly(playcorners,playbx, playby, int(orp.x),int(orp.y))!=0) {
                        pressedbutton = 1;
                        //myLCD.printf("                         ");
                        //myLCD.locate(64,117);
                        //myLCD.printf("Play %d",pnpoly(corners,playbx, playby, int(orp.x),int(orp.y)));
                        if(touchstate == 1) {
                            touchstate = 2;
                        } else if(touchstate == 2) {

                        } else {
                            touchstate = 1;
                        }
                    } else if(pnpoly(backcorners,backbx, backby, int(orp.x),int(orp.y))!=0) {
                        pressedbutton = 4;
                        //myLCD.printf("                         ");
                        //myLCD.locate(64,117);
                        //myLCD.printf("Back %d",pnpoly(corners,backbx, backby, int(orp.x),int(orp.y)));
                        if(touchstate == 1) {
                            touchstate = 2;
                        } else if(touchstate == 2) {

                        } else {
                            touchstate = 1;
                        }

                    }
#if (buttonsetup != 2)
                    else if(pnpoly(stopcorners,stopbx, stopby, int(orp.x),int(orp.y))!=0) {
                        pressedbutton = 2;
                        //myLCD.printf("                         ");
                        //myLCD.locate(64,117);
                        //myLCD.printf("Stop %d",pnpoly(corners,stopbx, stopby, int(orp.x),int(orp.y)));
                        if(touchstate == 1) {
                            touchstate = 2;
                        } else if(touchstate == 2) {

                        } else {
                            touchstate = 1;
                        }
                    }
#endif
                    else if(pnpoly(forwardcorners,forwardbx, forwardby, int(orp.x),int(orp.y))!=0) {
                        pressedbutton = 3;
                        //myLCD.printf("                         ");
                        //myLCD.locate(64,117);
                        //myLCD.printf("Forward %d",pnpoly(corners,forwardbx, forwardby, int(orp.x),int(orp.y)));
                        if(touchstate == 1) {
                            touchstate = 2;
                        } else if(touchstate == 2) {

                        } else {
                            touchstate = 1;
                        }
                    } else {
                        pressedbutton = 0;
                    }
                }
            }
        }
        if(((abs(ap.x-30000)<100)||ap.x>63500)&&orp.y <240) {
            if(orp.x != -1) {
                if(touchstate == 0) {
                    touchstate = 3;
                } else if(touchstate == 3) {

                } else {
                    touchstate = 0;
                }
            }
            if(orp.y != -1) {
                if(touchstate == 0) {
                    touchstate = 3;
                } else if(touchstate == 3) {

                } else {
                    touchstate = 0;
                }
            }
        }
        //if(isampl == 0) {
        if(laststate != touchstate) {

            if(touchstate == 1) {
                switch(pressedbutton) {
                    case 0:
                        //none
                        //pc.printf("None\n\r");
                        break;
                    case 1:
                        //play
                        //pc.printf("Play\n\r");
                        loadRandomFile(rand()%80);
                        break;
                    case 2:
                        //stop
                        //pc.printf("Stop\n\r");
                        if(playstatus == 0) {
                            playstatus = 1;
                        } else {
                            playstatus = 0;
                        }
                        break;
                    case 3:
                        //forward
                        //pc.printf("Forward\n\r");
                        loadNextFile();
                        break;
                    case 4:
                        //back
                        //pc.printf("Back\n\r");
                        loadPreviousFile();
                        break;
                    default:
                        //pc.printf("Error\n\r");
                        break;
                }
            }
            laststate = touchstate;
            //}
        }



        osDelay(200);
    }

}

//Inputtimer

void inputtimer()
{

        osMutexWait(lcd_mutex, osWaitForever);
        myLCD.BusEnable(false);
        ap.x = readTouchPanelX_Analog();
        ap.y = readTouchPanelY_Analog();
        //p.y  = simpleypos();
        //p.x = simplexpos();
        orp.x = getTouchPanelPosX();
        orp.y = getTouchPanelPosY();
        myLCD.BusEnable(true);
        //myLCD.locate(120,110);
        //myLCD.printf("Y: %7d",ap.y);
        //myLCD.locate(120,118);
        //myLCD.printf("X: %7d",ap.x);
        osMutexRelease(lcd_mutex);

        if((abs(ap.x-30000)>100)&&ap.x<63500) {
            if(orp.x != -1) {
                if(orp.y != -1) {


                    if(pnpoly(playcorners,playbx, playby, int(orp.x),int(orp.y))!=0) {
                        pressedbutton = 1;
                        //myLCD.printf("                         ");
                        //myLCD.locate(64,117);
                        //myLCD.printf("Play %d",pnpoly(corners,playbx, playby, int(orp.x),int(orp.y)));
                        if(touchstate == 1) {
                            touchstate = 2;
                        } else if(touchstate == 2) {

                        } else {
                            touchstate = 1;
                        }
                    } else if(pnpoly(backcorners,backbx, backby, int(orp.x),int(orp.y))!=0) {
                        pressedbutton = 4;
                        //myLCD.printf("                         ");
                        //myLCD.locate(64,117);
                        //myLCD.printf("Back %d",pnpoly(corners,backbx, backby, int(orp.x),int(orp.y)));
                        if(touchstate == 1) {
                            touchstate = 2;
                        } else if(touchstate == 2) {

                        } else {
                            touchstate = 1;
                        }

                    }
#if (buttonsetup != 2)
                    else if(pnpoly(stopcorners,stopbx, stopby, int(orp.x),int(orp.y))!=0) {
                        pressedbutton = 2;
                        //myLCD.printf("                         ");
                        //myLCD.locate(64,117);
                        //myLCD.printf("Stop %d",pnpoly(corners,stopbx, stopby, int(orp.x),int(orp.y)));
                        if(touchstate == 1) {
                            touchstate = 2;
                        } else if(touchstate == 2) {

                        } else {
                            touchstate = 1;
                        }
                    }
#endif
                    else if(pnpoly(forwardcorners,forwardbx, forwardby, int(orp.x),int(orp.y))!=0) {
                        pressedbutton = 3;
                        //myLCD.printf("                         ");
                        //myLCD.locate(64,117);
                        //myLCD.printf("Forward %d",pnpoly(corners,forwardbx, forwardby, int(orp.x),int(orp.y)));
                        if(touchstate == 1) {
                            touchstate = 2;
                        } else if(touchstate == 2) {

                        } else {
                            touchstate = 1;
                        }
                    } else {
                        pressedbutton = 0;
                    }
                }
            }
        }
        if(((abs(ap.x-30000)<100)||ap.x>63500)&&orp.y <240) {
            if(orp.x != -1) {
                if(touchstate == 0) {
                    touchstate = 3;
                } else if(touchstate == 3) {

                } else {
                    touchstate = 0;
                }
            }
            if(orp.y != -1) {
                if(touchstate == 0) {
                    touchstate = 3;
                } else if(touchstate == 3) {

                } else {
                    touchstate = 0;
                }
            }
        }
        //if(isampl == 0) {
        if(laststate != touchstate) {

            if(touchstate == 1) {
                switch(pressedbutton) {
                    case 0:
                        //none
                        //pc.printf("None\n\r");
                        break;
                    case 1:
                        //play
                        //pc.printf("Play\n\r");
                        loadRandomFile(rand()%80);
                        break;
                    case 2:
                        //stop
                        //pc.printf("Stop\n\r");
                        if(playstatus == 0) {
                            playstatus = 1;
                        } else {
                            playstatus = 0;
                        }
                        break;
                    case 3:
                        //forward
                        //pc.printf("Forward\n\r");
                        loadNextFile();
                        break;
                    case 4:
                        //back
                        //pc.printf("Back\n\r");
                        loadPreviousFile();
                        break;
                    default:
                        //pc.printf("Error\n\r");
                        break;
                }
            }
            laststate = touchstate;
            //}
   
    }

}
//Endinputtimer


void outputthread(void const *data)
{
    uint8_t lastdisplay = 1;
    int dispdelay = 1;
    int currsonglength = 0;
    float soundx = 0;
    float scrollx = 1;
    int colorposl = 0;
    int colorposr = 0;
    int buttonxpos = 260;
    int buttonypos = 7;
#define waveformbeginx 0
#define waveformendx 99
#define leftchany 57
#define rightchany 177
#define filenameystart 22
    int filenamex = 111;
    int filenamey = filenameystart;
    int lastplayedx = filenamex+74;
    int lastplayedy = filenameystart;
    time_t seconds;
    osDelay(200);
    while(1) {

        osMutexWait(lcd_mutex, osWaitForever);
        myLCD.locate(0,20);
        if(strcmp( (const char *)g_pCurrentHandler->szFileExt ,".S3M")==0) {

            //myLCD.printf("Song length: %d",uMod.S3m.songLength);
            currsonglength = uMod.S3m.songLength;

        } else {

            //myLCD.printf("Song length: %d",uMod.Mod.songLength);
            currsonglength = uMod.Mod.songLength;

        }

        if(playstatus ==1){
        //colorpos = (((soundx+1)/320.0f)+(((float)SoundBuffer.right[SoundBuffer.readPos])/(1 << BITDEPTH - 1)))*(1<<16);
        colorposr = CONVERT_888RGB_TO_565RGB((int)(255 * (0.5f+((float)SoundBuffer.right[SoundBuffer.readPos]/(float)(1<<BITDEPTH))/2)),(int)(128 * ((float)SoundBuffer.right[SoundBuffer.readPos]/(float)(1<<BITDEPTH))), (int)(32 * ((float)SoundBuffer.right[SoundBuffer.readPos]/(float)(1<<BITDEPTH))));
        colorposl = CONVERT_888RGB_TO_565RGB((int)(255 * (0.5f+((float)SoundBuffer.left[SoundBuffer.readPos]/(float)(1<<BITDEPTH))/2)),(int)(128 * ((float)SoundBuffer.left[SoundBuffer.readPos]/(float)(1<<BITDEPTH))), (int)(32 * ((float)SoundBuffer.left[SoundBuffer.readPos]/(float)(1<<BITDEPTH))));
        if(abs(soundx - (int)soundx) < 0.1f) {
            myLCD.line((int)soundx+5, 6,(int)soundx+5, 234, Black);
        }
        //(((((float)SoundBuffer.right[SoundBuffer.readPos]/(float)(1<<BITDEPTH))*110)%111)-55)+185
        //(((((((float)SoundBuffer.right[SoundBuffer.readPos])/(float)(1 << BITDEPTH))*2)-1)*55)+185)%55
        //myLCD.line(320, 130,320, 240, Black);
        myLCD.line((int)soundx+5,rightchany,(int)soundx+5,(((int)(((float)SoundBuffer.right[SoundBuffer.readPos]/(float)(1<<BITDEPTH))*114)%115)-57)+rightchany,colorposr); //((int)((((float)SoundBuffer.right[SoundBuffer.readPos])/(1 << BITDEPTH)*2)*110)%110)+75, colorpos);
        myLCD.line((int)soundx+5,leftchany,(int)soundx+5,(((int)(((float)SoundBuffer.left[SoundBuffer.readPos]/(float)(1<<BITDEPTH))*114)%115)-57)+leftchany,colorposl);
        //myLCD.pixel((int)soundx, (((float)SoundBuffer.right[SoundBuffer.readPos])/(1<<BITDEPTH))*240, colorpos);

        //myLCD.line(320,185,320,(((int)(((float)SoundBuffer.right[SoundBuffer.readPos]/(float)(1<<BITDEPTH))*110)%111)-55)+185,colorpos); //((int)((((float)SoundBuffer.right[SoundBuffer.readPos])/(1 << BITDEPTH)*2)*110)%110)+75, colorpos);

        myLCD.setscrollarea(waveformbeginx+5,waveformendx);
        myLCD.scroll((int)scrollx);
        
        scrollx += 1;
        if(scrollx>(waveformendx)) {
            scrollx = 0;
        }
        soundx += 1;
        //soundx = ((soundx + (dispdelay /1000.0f)));
        if(soundx>waveformendx) {
            soundx = waveformbeginx;

        }
        
        }
        osMutexRelease(lcd_mutex);
        osMutexWait(lcd_mutex, osWaitForever);
        //myLCD.locate(160,117);
        //myLCD.printf("%.6f",(((float)SoundBuffer.right[SoundBuffer.readPos])/(1<<BITDEPTH))*100);
        //myLCD.locate(50,50);
        //myLCD.printf("X: %06d Y: %06d",ap.x, ap.y);
        myLCD.locate(111,7);
        seconds = time(NULL);
        myLCD.printf("%s",ctime(&seconds));

        if(updatedfile == 1) {
            myLCD.fillrect(lastplayedx,lastplayedy,lastplayedx+6,lastplayedy+6, CONVERT_888RGB_TO_565RGB(0, 0, 0));
            //myLCD.line(5,17,259,17,CONVERT_888RGB_TO_565RGB(146, 146, 146));
            //myLCD.line(90,17,90,109,CONVERT_888RGB_TO_565RGB(146, 146, 146));
            myLCD.locate(filenamex,filenamey);
            myLCD.printf("               ");
            myLCD.locate(filenamex,filenamey);
            myLCD.printf("%s",fileInfo.fname);

            myLCD.fillrect(filenamex+74,filenamey, filenamex+80,filenamey+6,CONVERT_888RGB_TO_565RGB(128, 128, 128));
            /*if(filenamex == 6) {

                myLCD.rect(82,filenamey,82+6,filenamey+6, CONVERT_888RGB_TO_565RGB(128, 128, 128));
            } else {
                myLCD.rect(92,filenamey,92+6,filenamey+6, CONVERT_888RGB_TO_565RGB(128, 128, 128));
            }*/


            lastplayedy = filenamey;
            lastplayedx = filenamex +74;
            /*if(filenamex == 6) {
                lastplayedx = 82;
            } else {
                lastplayedx = 92;
            }*/

            filenamey += 11;
            if(filenamey > ((11*6)+filenameystart)) {
                if(filenamex == 111) {
                    filenamex = 216;
                } else {
                    filenamex = 111;
                }
                filenamey = filenameystart;
            }
            updatedfile = 0;
        }
        //SoundBuffer.left[SoundBuffer.readPos]
        osMutexRelease(lcd_mutex);
        if(lastdisplay != pressedbutton) {
            osMutexWait(lcd_mutex, osWaitForever);

            switch(pressedbutton) {
                case 0:
                    //none
                    //pc.printf("None\n\r");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("         ");

                    //myLCD.printf("Play ");
                    break;
                case 1:
                    //play
                    //pc.printf("Play\n\r");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("         ");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("Random");
                    break;
                case 2:
                    //stop
                    //pc.printf("Stop\n\r");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("         ");
                    myLCD.locate(buttonxpos,buttonypos);
                    if(playstatus == 0){
                    myLCD.printf("Stop");
                    }
                    else{
                        myLCD.printf("Resume");
                        }
                    break;
                case 3:
                    //forward
                    //pc.printf("Forward\n\r");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("         ");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("Forward");
                    break;
                case 4:
                    //back
                    //pc.printf("Back\n\r");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("         ");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("Back");
                    break;
                default:
                    //pc.printf("Error\n\r");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("         ");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("Error");
                    break;
            }
            osMutexRelease(lcd_mutex);
            lastdisplay = pressedbutton;
        }
        osDelay(dispdelay);
    }
}

//Outputtimer
    uint8_t lastdisplay = 1;
    int dispdelay = 1;
    int currsonglength = 0;
    float soundx = 0;
    float scrollx = 1;
    int colorposl = 0;
    int colorposr = 0;
    int buttonxpos = 260;
    int buttonypos = 7;
#define waveformbeginx 0
#define waveformendx 99
#define leftchany 57
#define rightchany 177
#define filenameystart 22
    int filenamex = 111;
    int filenamey = filenameystart;
    int lastplayedx = filenamex+74;
    int lastplayedy = filenameystart;
void outputtimer()
{

    time_t seconds;
    //osDelay(200);
    //while(1) {

        osMutexWait(lcd_mutex, osWaitForever);
        myLCD.locate(0,20);
        if(strcmp( (const char *)g_pCurrentHandler->szFileExt ,".S3M")==0) {

            //myLCD.printf("Song length: %d",uMod.S3m.songLength);
            currsonglength = uMod.S3m.songLength;

        } else {

            //myLCD.printf("Song length: %d",uMod.Mod.songLength);
            currsonglength = uMod.Mod.songLength;

        }

        if(playstatus ==1){
        //colorpos = (((soundx+1)/320.0f)+(((float)SoundBuffer.right[SoundBuffer.readPos])/(1 << BITDEPTH - 1)))*(1<<16);
        colorposr = CONVERT_888RGB_TO_565RGB((int)(255 * (0.5f+((float)SoundBuffer.right[SoundBuffer.readPos]/(float)(1<<BITDEPTH))/2)),(int)(128 * ((float)SoundBuffer.right[SoundBuffer.readPos]/(float)(1<<BITDEPTH))), (int)(32 * ((float)SoundBuffer.right[SoundBuffer.readPos]/(float)(1<<BITDEPTH))));
        colorposl = CONVERT_888RGB_TO_565RGB((int)(255 * (0.5f+((float)SoundBuffer.left[SoundBuffer.readPos]/(float)(1<<BITDEPTH))/2)),(int)(128 * ((float)SoundBuffer.left[SoundBuffer.readPos]/(float)(1<<BITDEPTH))), (int)(32 * ((float)SoundBuffer.left[SoundBuffer.readPos]/(float)(1<<BITDEPTH))));
        if(abs(soundx - (int)soundx) < 0.1f) {
            myLCD.line((int)soundx+5, 6,(int)soundx+5, 234, Black);
        }
        //(((((float)SoundBuffer.right[SoundBuffer.readPos]/(float)(1<<BITDEPTH))*110)%111)-55)+185
        //(((((((float)SoundBuffer.right[SoundBuffer.readPos])/(float)(1 << BITDEPTH))*2)-1)*55)+185)%55
        //myLCD.line(320, 130,320, 240, Black);
        myLCD.line((int)soundx+5,rightchany,(int)soundx+5,(((int)(((float)SoundBuffer.right[SoundBuffer.readPos]/(float)(1<<BITDEPTH))*114)%115)-57)+rightchany,colorposr); //((int)((((float)SoundBuffer.right[SoundBuffer.readPos])/(1 << BITDEPTH)*2)*110)%110)+75, colorpos);
        myLCD.line((int)soundx+5,leftchany,(int)soundx+5,(((int)(((float)SoundBuffer.left[SoundBuffer.readPos]/(float)(1<<BITDEPTH))*114)%115)-57)+leftchany,colorposl);
        //myLCD.pixel((int)soundx, (((float)SoundBuffer.right[SoundBuffer.readPos])/(1<<BITDEPTH))*240, colorpos);

        //myLCD.line(320,185,320,(((int)(((float)SoundBuffer.right[SoundBuffer.readPos]/(float)(1<<BITDEPTH))*110)%111)-55)+185,colorpos); //((int)((((float)SoundBuffer.right[SoundBuffer.readPos])/(1 << BITDEPTH)*2)*110)%110)+75, colorpos);

        myLCD.setscrollarea(waveformbeginx+5,waveformendx);
        myLCD.scroll((int)scrollx);
        
        scrollx += 1;
        if(scrollx>(waveformendx)) {
            scrollx = 0;
        }
        soundx += 1;
        //soundx = ((soundx + (dispdelay /1000.0f)));
        if(soundx>waveformendx) {
            soundx = waveformbeginx;

        }
        
        }
        osMutexRelease(lcd_mutex);
        osMutexWait(lcd_mutex, osWaitForever);
        //myLCD.locate(160,117);
        //myLCD.printf("%.6f",(((float)SoundBuffer.right[SoundBuffer.readPos])/(1<<BITDEPTH))*100);
        //myLCD.locate(50,50);
        //myLCD.printf("X: %06d Y: %06d",ap.x, ap.y);
        myLCD.locate(111,7);
        seconds = time(NULL);
        myLCD.printf("%s",ctime(&seconds));

        if(updatedfile == 1) {
            myLCD.fillrect(lastplayedx,lastplayedy,lastplayedx+6,lastplayedy+6, CONVERT_888RGB_TO_565RGB(0, 0, 0));
            //myLCD.line(5,17,259,17,CONVERT_888RGB_TO_565RGB(146, 146, 146));
            //myLCD.line(90,17,90,109,CONVERT_888RGB_TO_565RGB(146, 146, 146));
            myLCD.locate(filenamex,filenamey);
            myLCD.printf("               ");
            myLCD.locate(filenamex,filenamey);
            myLCD.printf("%s",fileInfo.fname);

            myLCD.fillrect(filenamex+74,filenamey, filenamex+80,filenamey+6,CONVERT_888RGB_TO_565RGB(128, 128, 128));
            /*if(filenamex == 6) {

                myLCD.rect(82,filenamey,82+6,filenamey+6, CONVERT_888RGB_TO_565RGB(128, 128, 128));
            } else {
                myLCD.rect(92,filenamey,92+6,filenamey+6, CONVERT_888RGB_TO_565RGB(128, 128, 128));
            }*/


            lastplayedy = filenamey;
            lastplayedx = filenamex +74;
            /*if(filenamex == 6) {
                lastplayedx = 82;
            } else {
                lastplayedx = 92;
            }*/

            filenamey += 11;
            if(filenamey > ((11*6)+filenameystart)) {
                if(filenamex == 111) {
                    filenamex = 216;
                } else {
                    filenamex = 111;
                }
                filenamey = filenameystart;
            }
            updatedfile = 0;
        }
        //SoundBuffer.left[SoundBuffer.readPos]
        osMutexRelease(lcd_mutex);
        if(lastdisplay != pressedbutton) {
            osMutexWait(lcd_mutex, osWaitForever);

            switch(pressedbutton) {
                case 0:
                    //none
                    //pc.printf("None\n\r");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("         ");

                    //myLCD.printf("Play ");
                    break;
                case 1:
                    //play
                    //pc.printf("Play\n\r");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("         ");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("Random");
                    break;
                case 2:
                    //stop
                    //pc.printf("Stop\n\r");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("         ");
                    myLCD.locate(buttonxpos,buttonypos);
                    if(playstatus == 0){
                    myLCD.printf("Stop");
                    }
                    else{
                        myLCD.printf("Resume");
                        }
                    break;
                case 3:
                    //forward
                    //pc.printf("Forward\n\r");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("         ");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("Forward");
                    break;
                case 4:
                    //back
                    //pc.printf("Back\n\r");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("         ");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("Back");
                    break;
                default:
                    //pc.printf("Error\n\r");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("         ");
                    myLCD.locate(buttonxpos,buttonypos);
                    myLCD.printf("Error");
                    break;
            }
            osMutexRelease(lcd_mutex);
            lastdisplay = pressedbutton;
        //}
        //osDelay(dispdelay);
    }
}
//Endouputtimer

//DigitalOut foop (PC_10);
//DigitalOut barp (PC_12);

float testduty;
void soundouttimer()
{


    if((SoundBuffer.writePos != SoundBuffer.readPos)&&(playstatus !=0)) {
        //barp = !barp;
        //Sound output
        //pc.printf("%.3f,%.3f \r\n",(((float)SoundBuffer.left[SoundBuffer.readPos])/(float)(1<<BITDEPTH)), (((float)SoundBuffer.right[SoundBuffer.readPos])/(float)(1<<BITDEPTH)));

        leftchan.write(((float)SoundBuffer.left[SoundBuffer.readPos])/(1<<BITDEPTH));
        /*if(((((float)SoundBuffer.left[SoundBuffer.readPos])/(float)(1 << BITDEPTH - 1))*1.0f)>1) {
            leftchan.write(1.0);
        } else {
            leftchan.write((((float)SoundBuffer.left[SoundBuffer.readPos])/(float)(1 << BITDEPTH - 1))*1.0f);
        }*/

        rightchan.write(((float)SoundBuffer.right[SoundBuffer.readPos])/(1<<BITDEPTH));
        /*if(((((float)SoundBuffer.right[SoundBuffer.readPos])/(float)(1 << BITDEPTH - 1))*1.0f)>1) {
            rightchan.write(1.0);
        } else {
            rightchan.write((((float)SoundBuffer.right[SoundBuffer.readPos])/(float)(1 << BITDEPTH - 1))*1.0f);
        }*/
        //myLCD.locate(soundx, (((float)SoundBuffer.right[SoundBuffer.readPos])/(1<<BITDEPTH))*240);

        //left.write(SoundBuffer.left[SoundBuffer.readPos]);
        //right.write(SoundBuffer.right[SoundBuffer.readPos]);
        //TimerMatchSet(TIMER2_BASE, TIMER_B, SoundBuffer.left[SoundBuffer.readPos]); // PWM
        //TimerMatchSet(TIMER2_BASE, TIMER_A, SoundBuffer.right[SoundBuffer.readPos]); // PWM

        //Visualizer
        //RED led
        //TimerMatchSet(TIMER0_BASE, TIMER_B, (SoundBuffer.left[SoundBuffer.readPos]-850)<<5);
        //Blue led
        //TimerMatchSet(TIMER1_BASE, TIMER_A, (SoundBuffer.right[SoundBuffer.readPos]-850)<<5);
        SoundBuffer.readPos++;
        SoundBuffer.readPos &= SOUNDBUFFERSIZE - 1;
        //foop = !foop;

    }
    else{
        leftchan.write(0.5);
        rightchan.write(0.5);
        }
    //Thread::wait(500);

    //wait_us(5);
    //wait((float)1/SAMPLERATE));
    //osDelay(200);

}
#ifdef wifienabled
void setup(void)
{
    pc.printf("setup begin\r\n");
    delay(2000);
    pc.printf("FW Version: %s\r\n", wifi.getVersion().c_str());
      
    if (wifi.setOprToStationSoftAP()) {
        pc.printf("to station + softap ok\r\n");
    } else {
        pc.printf("to station + softap err\r\n");
    }

    if (wifi.joinAP("WLANSID", "WLANPASS")) {
        pc.printf("Join AP success\r\n");
        pc.printf("IP: [%s]\r\n", wifi.getLocalIP().c_str());       
    } else {
       pc.printf("Join AP failure\r\n");
    }
    
    if (wifi.disableMUX()) {
        pc.printf("single ok\r\n");
    } else {
        pc.printf("single err\r\n");
    }
    
    pc.printf("setup end\r\n");
}

void get_wifi_time(void const* data){
    //https://developer.mbed.org/teams/ESP8266/wiki/Firmware-Update
    setup();
    uint8_t buffer[512] = {0};
    char timebuffer[15];
    volatile time_t epoch;
    epoch = 1460413475;
    //get the time
    if (wifi.createTCP("www.timeapi.org", 80)) {
        pc.printf("create tcp ok\r\n");
    } else {
        pc.printf("create tcp err\r\n");
    }
    
    char *timereq = "GET /utc/in+two+hours?\\s HTTP/1.1\r\nHost: www.timeapi.org\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/49.0.2623.110 Safari/537.36\r\nReferer: http://www.timeapi.org/\r\n\r\n";
    wifi.send((const uint8_t*)timereq, strlen(timereq));
    uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
    memset(timebuffer, 0, sizeof(timebuffer));
    if (len > 0) {
        pc.printf("Received:[");
        for(uint32_t i = len-10; i < len; i++) {
            pc.printf("%c", buffer[i]);
            appendchar(timebuffer,sizeof(timebuffer),(char)buffer[i]);
            //strcat(timebuffer,(char *)buffer[i]);
        }
        pc.printf("]\r\n");
        epoch = atoi(timebuffer);
    }
    
    if (wifi.releaseTCP()) {
        pc.printf("release tcp ok\r\n");
    } else {
        pc.printf("release tcp err\r\n");
    }
    //end get time
    printf("Timebuffer: %s\r\n",timebuffer);
    pc.printf("Epoch: %d\r\n",epoch);
    set_time(epoch);
    osThreadTerminate(osThreadGetId());
    }
#endif
//AnalogOut dac_output(PA_4);
Ticker timer3sim;
Ticker timer4sim;
Ticker timer5sim;

int main()
{
    pc.baud (115200);
    delay(2000);
    pc.printf("\r\nStart\r\n");
    pc.printf("\n\nSystem Core Clock = %.3f MHZ\r\n",(float)SystemCoreClock/1000000);
    pc.printf("\n\nSamplerate = %d HZ\r\n",SAMPLERATE);
    setup();
    #ifdef wifienabled
    /*setup();
    uint8_t buffer[512] = {0};
    char timebuffer[15];
    volatile time_t epoch;
    epoch = 1460413475;
    //get the time
    if (wifi.createTCP("www.timeapi.org", 80)) {
        pc.printf("create tcp ok\r\n");
    } else {
        pc.printf("create tcp err\r\n");
    }
    
    char *timereq = "GET /utc/in+two+hours?\\s HTTP/1.1\r\nHost: www.timeapi.org\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/49.0.2623.110 Safari/537.36\r\nReferer: http://www.timeapi.org/\r\n\r\n";
    wifi.send((const uint8_t*)timereq, strlen(timereq));
    uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
    memset(timebuffer, 0, sizeof(timebuffer));
    if (len > 0) {
        pc.printf("Received:[");
        for(uint32_t i = len-10; i < len; i++) {
            pc.printf("%c", buffer[i]);
            appendchar(timebuffer,sizeof(timebuffer),(char)buffer[i]);
            //strcat(timebuffer,(char *)buffer[i]);
        }
        pc.printf("]\r\n");
        epoch = atoi(timebuffer);
    }
    
    if (wifi.releaseTCP()) {
        pc.printf("release tcp ok\r\n");
    } else {
        pc.printf("release tcp err\r\n");
    }
    //end get time
    printf("Timebuffer: %s\r\n",timebuffer);
    pc.printf("Epoch: %d\r\n",epoch);
    set_time(epoch);
    */
    #endif
    lcd_mutex = osMutexCreate(osMutex(lcd_mutex));
    loadfile_mutex = osMutexCreate(osMutex(loadfile_mutex));

    sd.mount();
    testduty = 0;
    //uint32_t i = 0;
    //https://developer.mbed.org/users/loop/code/1bitcraze/file/167d81d4f79f/main.cpp
    //leftchan.period(1.0f/80000.0f);
    //rightchan.period(1.0f/80000.0f);
    leftchan.period(1.0f/((float)PWM_PERIOD *2));
    rightchan.period(1.0f/((float)PWM_PERIOD *2));
    //leftchan = 0.5;
    //rightchan = 0.5;
    //rightchan.period((float)1.0f/SAMPLERATE);
    //leftchan.pulsewidth(0.5f);
    //rightchan.pulsewidth(0.5f);
    //left.write(1 << BITDEPTH);

    //FILE *fp = fopen("/sd/mbed.txt", "w");
    //fprintf(fp, "Hello World!\n");
    //fclose(fp);
//    myLCD.set_contrast(26);//for uc1608
//   myLCD.set_contrast(46);//for ist3020
    myLCD.set_orientation(orient);
    //int time, time2;
    //time = 0;
    //int touchstate = 0; // 1 downnow, 0 upnow, 2 downstill, 3 upstill
    //int pressedbutton = 0; // 0 nothing, 1 play, 2 stop, 3 forward, 4 back
    //int laststate = 1;


    //t.start();
//   myLCD.set_font((unsigned char*) Terminal6x8);
    // myLCD.claim(stdout);      // send stdout to the LCD display
    //myLCD.claim(stderr);      // send stderr to the LCD display
    myLCD.background(backgroundcolor);    // set background to black
    myLCD.foreground(foregroundcolor);    // set chars to white
    //tt.calibrate();          // calibrate the touch
    f_mount(&fso,"0",1);
    //f_chdir("/Images");
    //f_opendir(&dir, ".");
    //int imstat = 0;
    //osMutexWait(lcd_mutex, osWaitForever);
    //FIL ImageT;
    const char * Name_BMP = "MODPLA~1.BMP";
    f_chdir("/");
    //imstat = f_open(&ImageT, Name_BMP, FA_READ);
    myLCD.BMP_16(0,0,Name_BMP);
    //osMutexRelease(lcd_mutex);
    //myLCD.Bitmap(0,0,320,240,(unsigned char *)modplayer);
    //myLCD.locate(180,20);
    //myLCD.printf("Stat: %d",imstat);
    srand( readTouchPanelY_Analog() );
    modFileNumber = 0;
    //while(disk_initialize(0)) {

    //}

    //f_chdir(PATH);
    //&dir = opendir(PATH);
    //opendir(PATH);
    f_chdir("/mods");
    f_opendir(&dir, ".");
    osMutexRelease(loadfile_mutex); //evtl neue Datei laden falls requested wurde
    loadNextFile();
    loadNextFile();
    loadNextFile();
    //myLCD.locate(6,15);
    //myLCD.printf("             ");
    //myLCD.locate(6,15);
    //myLCD.printf("%s",fileInfo.fname);
    //char pathbuf[20];
    //myLCD.locate(0,230);
    //f_getcwd(pathbuf, 19);
    //myLCD.printf("%s\r\n",pathbuf);
    //timeri = 0;
    //timer3sim.attach(&fillsoundbuffer,1);
    //osThreadDef(flip, osPriorityNormal, DEFAULT_STACK_SIZE);
    //osThreadDef(flip2, osPriorityNormal, DEFAULT_STACK_SIZE);
    //osThreadDef(fillsoundbuffer, osPriorityNormal, DEFAULT_STACK_SIZE);
    //osThreadDef(inputoutthread, osPriorityNormal, (DEFAULT_STACK_SIZE * 8));
    //////////////////////
    /*
    Thread thread(inputthread);
    Thread thread(outputthread);
    #ifdef wifienabled
    Thread thread(get_wifi_time);
    #endif
    */
    /////////////////////
    
    
    osThreadDef(inputthread, osPriorityNormal, (DEFAULT_STACK_SIZE * 6));
    osThreadDef(outputthread, osPriorityNormal, (DEFAULT_STACK_SIZE * 6));
    #ifdef wifienabled
    osThreadDef(get_wifi_time, osPriorityLow, (DEFAULT_STACK_SIZE * 6));
    osThreadCreate(osThread(get_wifi_time),NULL);
    #endif
    //osThreadDef(samplefillerthread, osPriorityNormal, (DEFAULT_STACK_SIZE * 16));
    osThreadCreate(osThread(inputthread),NULL);
    osThreadCreate(osThread(outputthread),NULL);
    timer4sim.attach(&outputtimer, 1.0f/4);
    timer5sim.attach(&inputtimer, 1.0/4);
    //osThreadCreate(osThread(samplefillerthread),NULL);
    //osThreadCreate(osThread(inputoutthread),NULL);
    //RtosTimer samplefill(samplefiller, osTimerPeriodic);
    //samplefill.start(1);

    //timer3sim.attach(&samplefiller, 1.0f / 1024);//((float)1.0/((SYSCLK / SAMPLERATE) +1)));

    //timer3sim.attach(&soundouttimer, ((float)1.0f/(((float)SYSCLK / (float)SAMPLERATE) +1)));
    timer3sim.attach(&soundouttimer, 1.0f/(SAMPLERATE));
    //samplefill.start(((float)1/SAMPLERATE)/(float)1000);
    //osThreadCreate(osThread(fillsoundbuffer),NULL);
    //osThreadCreate(osThread(flip),NULL);
    //osThreadCreate(osThread(flip2),NULL);
    //ThreadList* my_threads=NULL; //List of all Initialized threads
    //ThreadList* thread; //pointer to the last created ThreadList element
    //initThread(&my_threads,flip,&thread,1);
    //osThreadCreate(thread->thread,(void *) 1);
    //initThread(&my_threads,flip2,&thread,2);
    //initThread(&my_threads,fillsoundbuffer,&thread,2);

    //osThreadCreate(thread->thread,(void *) 2);

    //Thread thread(fillsoundbuffer);
    //timer4sim.attach(&flip,1);
    //Thread thread(flip);
    playstatus = 1;
    while(1) {
        while(playstatus !=1) {
            osDelay(10);
        }
        while((SoundBuffer.writePos + 1 & SOUNDBUFFERSIZE - 1) != SoundBuffer.readPos) {

            if(!isampl) {

                osMutexRelease(loadfile_mutex); //evtl neue Datei laden falls requested wurde
                osMutexWait(loadfile_mutex, osWaitForever); // Warten bis laden fertig ist und weiter machen. Max 100 ms
                player();
                isampl = getSamplesPerTick();

            }
            //barp = !barp;
            mixer();

            isampl--;

        }
        osMutexRelease(loadfile_mutex); //evtl neue Datei laden falls requested wurde
    }

}