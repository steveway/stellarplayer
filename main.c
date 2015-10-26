#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "inc/hw_timer.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "drivers/buttons.h"
#include "utils/uartstdio.h"
#include "driverlib/uart.h"

#include "time.h"
#include "stdlib.h"
#include "global.h"

#include ".\fatfs\ff.h"
#include ".\fatfs\diskio.h"

//*****************************************************************************
//
// Define pin to LED color mapping.
//
//*****************************************************************************

#define RED_LED   GPIO_PIN_1
#define BLUE_LED  GPIO_PIN_2
#define GREEN_LED GPIO_PIN_3

//TODO: I think this is some button input
#define ENCODERPORT 0

unsigned long g_ulFlags;
unsigned long g_ulTest;


volatile short encoderPosition = 0;
volatile char encoderSubPosition = 0;


volatile char uButton = 0;
volatile char uButtonPrev = 0;

//#define CHECKMEMORY {if(file.size!=19367){ROM_IntMasterDisable(); UARTprintf("uhOh (%s) size: %d at:%d\n",__FILE__,file.size,__LINE__); while(1){}}}

int main() {
	srand( time(NULL) );
	modFileNumber = 0;
	WORD i = 0;
	//char encoderDifference;
	BYTE mode = 0;
	//short oldEncoderPosition = 0;
	//BYTE oldEncoderBtn = 0;
	//BYTE encoderSubPositionReset = 0;
	unsigned long ulPeriod;
	//
	// Enable lazy stacking for interrupt handlers.  This allows floating-point
	// instructions to be used within interrupt handlers, but at the expense of
	// extra stack usage.
	//
	ROM_FPULazyStackingEnable();


	//
	// Setup the system clock to run at 80 Mhz from PLL with crystal reference
	//
	SysCtlClockSet(
			SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ
			| SYSCTL_OSC_MAIN);


	//Enable uart
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	//UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600,
	//                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
	 //                        UART_CONFIG_PAR_NONE));
	UARTStdioInit(0);
	UARTprintf("\033[2J");
	UARTprintf("\033[0;0H");
	UARTprintf("\033[2JS3M player\n");

	// Turn off LEDs
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

	//RED setting
	TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM|TIMER_CFG_B_PWM);
	TimerLoadSet(TIMER0_BASE, TIMER_B,  0xFFFF);
	TimerMatchSet(TIMER0_BASE, TIMER_B, 0); // PWM

	//Blue
	TimerConfigure(TIMER1_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM|TIMER_CFG_B_PWM);
	TimerLoadSet(TIMER1_BASE, TIMER_A,  0xFFFF);
	TimerMatchSet(TIMER1_BASE, TIMER_A, 0x0); // PWM

	//Green
	TimerLoadSet(TIMER1_BASE, TIMER_B,  0xFFFF);
	TimerMatchSet(TIMER1_BASE, TIMER_B, 0); // PWM

	//Invert input
    HWREG(TIMER0_BASE + TIMER_O_CTL)   |= 0x4000;
    HWREG(TIMER1_BASE + TIMER_O_CTL)   |= 0x40;
    HWREG(TIMER1_BASE + TIMER_O_CTL)   |= 0x4000;

	TimerEnable(TIMER0_BASE, TIMER_BOTH);
	TimerEnable(TIMER1_BASE, TIMER_BOTH);

	GPIOPinConfigure(GPIO_PF3_T1CCP1);
	GPIOPinTypeTimer(GPIO_PORTF_BASE, GPIO_PIN_3);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_STRENGTH_8MA_SC,
					 GPIO_PIN_TYPE_STD);

	GPIOPinConfigure(GPIO_PF2_T1CCP0);
	GPIOPinTypeTimer(GPIO_PORTF_BASE, GPIO_PIN_2);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_STRENGTH_8MA_SC,
					 GPIO_PIN_TYPE_STD);

	GPIOPinConfigure(GPIO_PF1_T0CCP1);
	GPIOPinTypeTimer(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_8MA_SC,
					 GPIO_PIN_TYPE_STD);
	//GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	//GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);




	//GPIOPinConfigure(GPIO_PF2_T1CCP0);
	//GPIOPinTypeTimer(GPIO_PORTF_BASE, GPIO_PIN_2);


	//Left stereo channel
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinConfigure(GPIO_PB1_T2CCP1); //GPIOPinConfigure(GPIO_PB4_T1CCP0);
	GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_1);


	//right stereo channel
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinConfigure(GPIO_PB0_T2CCP0);
	GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_0);

	// Configure timer left
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
	TimerConfigure(TIMER2_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM|TIMER_CFG_B_PWM);
	TimerLoadSet(TIMER2_BASE, TIMER_A,  1 << BITDEPTH);
	TimerMatchSet(TIMER2_BASE, TIMER_A, 0); // PWM
	//TimerEnable(TIMER2_BASE, TIMER_A);


	// Configure timer right
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	//TimerConfigure(TIMER2_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM);
	TimerLoadSet(TIMER2_BASE, TIMER_B,  1 << BITDEPTH);
	TimerMatchSet(TIMER2_BASE, TIMER_B, 0); // PWM
	PWMOutputInvert(TIMER2_BASE, TIMER_B, true);
	//TimerControlLevel(TIMER2_BASE, TIMER_B, true);
	TimerEnable(TIMER2_BASE, TIMER_BOTH);

    //
    // Sampler timer
    //
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
	TimerConfigure(TIMER3_BASE, TIMER_CFG_32_BIT_PER);
	ulPeriod = SYSCLK / SAMPLERATE;
	TimerLoadSet(TIMER3_BASE, TIMER_A, ulPeriod+1);

	TimerEnable(TIMER3_BASE, TIMER_A);


	//
	// Initialize the buttons
	//
	ButtonsInit();

    // Initialize the SysTick interrupt to process colors and buttons.
    //
    SysTickPeriodSet(SysCtlClockGet() / 16);
    SysTickEnable();


    //set priorities
    IntPrioritySet(INT_TIMER3A,0x00);
	IntPrioritySet(FAULT_SYSTICK,0x80);

	//
	// Enable interrupts to the processor.
	//
	ROM_IntMasterEnable();

	IntEnable(INT_TIMER3A);
	TimerIntEnable(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
	SysTickIntEnable();


	//TODO: uncomment this when we work with an SD card
	ROM_IntMasterDisable();
	UARTprintf("Init SD\n");
	while(disk_initialize(0));
	UARTprintf("Mounting FS\n");
	//f_mount(0, &fso);
	f_mount(&fso,"0",1);
	UARTprintf("\n");
	f_chdir(PATH);
	UARTprintf("Opening %s\n",PATH);
	//f_opendir(&dir,PATH);
	f_opendir(&dir, ".");

	loadNextFile();
	ROM_IntMasterEnable();

	for(;;)
	{
		while((SoundBuffer.writePos + 1 & SOUNDBUFFERSIZE - 1) != SoundBuffer.readPos)
		{
			if(!i)
			{

				//encoderDifference = encoderPosition - oldEncoderPosition;
				//if(encoderDifference)
				if(uButton!=uButtonPrev)
				{

					uButtonPrev=uButton;
					if(!mode)
					{
						if(uButton == ALL_BUTTONS)
						{
							ROM_IntMasterDisable();
							UARTprintf("Load random module\n");
							ROM_IntMasterEnable();
							loadRandomFile(rand()%20);

						}
						else if(uButton == LEFT_BUTTON)
						{
							ROM_IntMasterDisable();
							UARTprintf("Load previous module\n");
							ROM_IntMasterEnable();
							loadPreviousFile();
						}

						else if(uButton == RIGHT_BUTTON)
						{
							ROM_IntMasterDisable();
							UARTprintf("Load Next module\n");
							ROM_IntMasterEnable();
							loadNextFile();
						}

						/*if(encoderDifference > 0)
							loadNextFile();
						else
							loadPreviousFile();*/


					} else if(mode == 1)
					{

						/*Player.amiga += encoderDifference * 8000;
						if(Player.amiga < AMIGA / 2)
							Player.amiga = AMIGA / 2;
						else if(Player.amiga > AMIGA * 2)
							Player.amiga = AMIGA * 2;*/

					} else
					{

						/*Player.samplesPerTick -= encoderDifference * 10;
						if(Player.samplesPerTick < SAMPLERATE / (2 * 255 / 5))
							Player.samplesPerTick = SAMPLERATE / (2 * 255 / 5);
						else if(Player.samplesPerTick > SAMPLERATE / (2 * 32 / 5))
							Player.samplesPerTick = SAMPLERATE / (2 * 32 / 5);*/

					}

				} else
				{
					//encoderSubPositionReset++;
					//if(encoderSubPositionReset == 100)
					//{
					//	encoderSubPosition = 0;
					//	encoderSubPositionReset = 0;
					//}
				}
				//oldEncoderPosition = encoderPosition;

				//if(ENCODERBTNPIN && !oldEncoderBtn)
				//	if(++mode == 3)
				//		mode = 0;
				//oldEncoderBtn = ENCODERBTNPIN;

				player();
				i = getSamplesPerTick();
			}

			mixer();
			i--;

		}
	}

}

//*****************************************************************************
//
// The interrupt handler for the first timer interrupt.
// timer 3
//
//*****************************************************************************
void
Timer3IntHandler(void)
{
	//
	// Clear the timer interrupt.
	//
	ROM_TimerIntClear(TIMER3_BASE, TIMER_TIMA_TIMEOUT);

	//
	// Toggle the flag for the first timer.
	//
	HWREGBITW(&g_ulFlags, 0) ^= 1;

	//
	// Use the flags to Toggle the LED for this timer
	//
	/*if(++g_ulTest>=9999){
		g_ulTest = 0;
		GPIOPinWrite(GPIO_PORTF_BASE, RED_LED, g_ulFlags << 1);
	}*/



	if(SoundBuffer.writePos != SoundBuffer.readPos) {
		//Sound output
		TimerMatchSet(TIMER2_BASE, TIMER_B, SoundBuffer.left[SoundBuffer.readPos]); // PWM
		TimerMatchSet(TIMER2_BASE, TIMER_A, SoundBuffer.right[SoundBuffer.readPos]); // PWM

		//Visualizer
		//RED led
		TimerMatchSet(TIMER0_BASE, TIMER_B, (SoundBuffer.left[SoundBuffer.readPos]-850)<<5);
		//Blue led
		TimerMatchSet(TIMER1_BASE, TIMER_A, (SoundBuffer.right[SoundBuffer.readPos]-850)<<5);
		SoundBuffer.readPos++;
		SoundBuffer.readPos &= SOUNDBUFFERSIZE - 1;
	}
}

void AppButtonHandler(unsigned long ulButtons)
{
	switch(ulButtons & ALL_BUTTONS)
	{
		case ALL_BUTTONS:
			uButton = ALL_BUTTONS;
			//ROM_IntMasterDisable();
			//UARTprintf("Both Button pressed\n");
			//ROM_IntMasterEnable();

			break;
		case LEFT_BUTTON:
			uButton = LEFT_BUTTON;
		    //ROM_IntMasterDisable();
		    //UARTprintf("Left Button pressed\n");
		    //ROM_IntMasterEnable();

			break;
		case RIGHT_BUTTON:
			uButton = RIGHT_BUTTON;
			//ROM_IntMasterDisable();
			//UARTprintf("Right Button pressed\n");
			//ROM_IntMasterEnable();
			break;

		default:
			uButton = 0;
			break;
	}
}

void SysTickIntHandler(void)
{
	unsigned long ulButtons;

	ulButtons = ButtonsPoll(0,0);
	AppButtonHandler(ulButtons);
}

//*****************************************************************************
//
// The interrupt handler for the quardature encoder.
//
//*****************************************************************************
/*void
Timer1IntHandler(void)
{
	static BYTE AB;
	static BYTE oldAB = 0;
	static const char encoderStates[] = { 0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0,
			1, 0, 1, -1, 0 };
	//
	// Clear the timer interrupt.
	//
	ROM_TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

	//
	// Toggle the flag for the second timer.
	//
	HWREGBITW(&g_ulFlags, 1) ^= 1;

	//
	// Use the flags to Toggle the LED for this timer
	//
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, g_ulFlags << 1);

	AB = ENCODERPORT & 3;

	encoderSubPosition += encoderStates[(oldAB << 2) + AB];
	oldAB = AB;

	if (encoderSubPosition > 3) {
		encoderPosition++;
		encoderSubPosition = 0;
	} else if (encoderSubPosition < -3) {
		encoderPosition--;
		encoderSubPosition = 0;
	}

}*/
