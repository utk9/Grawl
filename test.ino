extern "C" {
#include <delay.h>
#include <FillPat.h>
#include <I2CEEPROM.h>
#include <LaunchPad.h>
#include <OrbitBoosterPackDefs.h>
#include <OrbitOled.h>
#include <OrbitOledChar.h>
#include <OrbitOledGrph.h>
}


/* ------------------------------------------------------------ */
/*				Local Type Definitions		*/
/* ------------------------------------------------------------ */
#define DEMO_0		0
#define DEMO_1		2
#define DEMO_2		1
#define DEMO_3		3
#define RED_LED   GPIO_PIN_1
#define BLUE_LED  GPIO_PIN_2
#define GREEN_LED GPIO_PIN_3


/* ------------------------------------------------------------ */
/*				Global Variables		*/
/* ------------------------------------------------------------ */
extern int xchOledMax; // defined in OrbitOled.c
extern int ychOledMax; // defined in OrbitOled.c


/* ------------------------------------------------------------ */
/*				Local Variables			*/
/* ------------------------------------------------------------ */
char	chSwtCur;
char	chSwtPrev;
bool	fClearOled;



char restartStr[38] = "Flip the switch off then on to begin.";  

/* ------------------------------------------------------------ */
/*				Forward Declarations							*/
/* ------------------------------------------------------------ */
void DeviceInit();
char CheckSwitches();
void OrbitSetOled();

char I2CGenTransmit(char * pbData, int cSize, bool fRW, char bAddr);
bool I2CGenIsNotIdle();

int platformHeight = 32;
int platformWidth = 8;

int ballWidth = 4;
int ballHeight = 8;

char rgBMPPlatform[] = {
  0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xc0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

char rgBMPBall[] = {
  0x3c, 0x3c, 0x3c, 0x3c
};

int xcoBall = 64;
int ycoBall = 12;

void setup()
{
  DeviceInit();
}

void loop()
{ 
  for(int i = 128; i >= 0; i--){
    delay(20);
    OrbitOledClear();
    OrbitOledMoveTo(i, 0);
    OrbitOledPutBmp(platformWidth, platformHeight, rgBMPPlatform);
    
    OrbitOledMoveTo(i < 68 ? i-4 : 64, 12);
    OrbitOledPutBmp(ballWidth, ballHeight, rgBMPBall);
    OrbitOledUpdate();
  }
}

/* ------------------------------------------------------------ */
/***	DeviceInit
 **
 **	Parameters:
 **		none
 **
 **	Return Value:
 **		none
 **
 **	Errors:
 **		none
 **
 **	Description:
 **		Initialize I2C Communication, and GPIO
 */
void DeviceInit()
{
  /*
   * First, Set Up the Clock.
   * Main OSC		  -> SYSCTL_OSC_MAIN
   * Runs off 16MHz clock -> SYSCTL_XTAL_16MHZ
   * Use PLL		  -> SYSCTL_USE_PLL
   * Divide by 4	  -> SYSCTL_SYSDIV_4
   */
  SysCtlClockSet(SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_USE_PLL | SYSCTL_SYSDIV_4);

  /*
   * Enable and Power On All GPIO Ports
   */
  //SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOA | SYSCTL_PERIPH_GPIOB | SYSCTL_PERIPH_GPIOC |
  //						SYSCTL_PERIPH_GPIOD | SYSCTL_PERIPH_GPIOE | SYSCTL_PERIPH_GPIOF);

  SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOA );
  SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOB );
  SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOC );
  SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOD );
  SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOE );
  SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOF );
  /*
   * Pad Configure.. Setting as per the Button Pullups on
   * the Launch pad (active low).. changing to pulldowns for Orbit
   */
  GPIOPadConfigSet(SWTPort, SWT1 | SWT2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

  GPIOPadConfigSet(BTN1Port, BTN1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
  GPIOPadConfigSet(BTN2Port, BTN2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

  GPIOPadConfigSet(LED1Port, LED1, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
  GPIOPadConfigSet(LED2Port, LED2, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
  GPIOPadConfigSet(LED3Port, LED3, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
  GPIOPadConfigSet(LED4Port, LED4, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);

  /*
   * Initialize Switches as Input
   */
  GPIOPinTypeGPIOInput(SWTPort, SWT1 | SWT2);

  /*
   * Initialize Buttons as Input
   */
  GPIOPinTypeGPIOInput(BTN1Port, BTN1);
  GPIOPinTypeGPIOInput(BTN2Port, BTN2);

  /*
   * Initialize LEDs as Output
   */
  GPIOPinTypeGPIOOutput(LED1Port, LED1);
  GPIOPinTypeGPIOOutput(LED2Port, LED2);
  GPIOPinTypeGPIOOutput(LED3Port, LED3);
  GPIOPinTypeGPIOOutput(LED4Port, LED4);

  /*
   * Enable ADC Periph
   */
  SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

  GPIOPinTypeADC(AINPort, AIN);

  /*
   * Enable ADC with this Sequence
   * 1. ADCSequenceConfigure()
   * 2. ADCSequenceStepConfigure()
   * 3. ADCSequenceEnable()
   * 4. ADCProcessorTrigger();
   * 5. Wait for sample sequence ADCIntStatus();
   * 6. Read From ADC
   */
  ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
  ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH0);
  ADCSequenceEnable(ADC0_BASE, 0);

  /*
   * Initialize the OLED
   */
  OrbitOledInit();

  /*
   * Reset flags
   */
  chSwtCur = 0;
  chSwtPrev = 0;
  fClearOled = true;

}

/* ------------------------------------------------------------ */
/***	CheckSwitches()
 **
 **	Parameters:
 **		none
 **
 **	Return Value:
 **		none
 **
 **	Errors:
 **		none
 **
 **	Description:
 **		Return the state of the Switches
 */
char CheckSwitches() {

  long 	lSwt1;
  long 	lSwt2;

  chSwtPrev = chSwtCur;

  lSwt1 = GPIOPinRead(SWT1Port, SWT1);
  lSwt2 = GPIOPinRead(SWT2Port, SWT2);

  chSwtCur = (lSwt1 | lSwt2) >> 6;

  if(chSwtCur != chSwtPrev) {
    fClearOled = true;
  }

  return chSwtCur;

}

/* ------------------------------------------------------------ */
/***	I2CGenTransmit
 **
 **	Parameters:
 **		pbData	-	Pointer to transmit buffer (read or write)
 **		cSize	-	Number of byte transactions to take place
 **
 **	Return Value:
 **		none
 **
 **	Errors:
 **		none
 **
 **	Description:
 **		Transmits data to a device via the I2C bus. Differs from
 **		I2C EEPROM Transmit in that the registers in the device it
 **		is addressing are addressed with a single byte. Lame, but..
 **		it works.
 **
 */
char I2CGenTransmit(char * pbData, int cSize, bool fRW, char bAddr) {

  int 		i;
  char * 		pbTemp;

  pbTemp = pbData;

  /*Start*/

  /*Send Address High Byte*/
  /* Send Write Block Cmd*/
  I2CMasterSlaveAddrSet(I2C0_BASE, bAddr, WRITE);
  I2CMasterDataPut(I2C0_BASE, *pbTemp);

  I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);

  DelayMs(1);

  /* Idle wait*/
  while(I2CGenIsNotIdle());

  /* Increment data pointer*/
  pbTemp++;

  /*Execute Read or Write*/

  if(fRW == READ) {

    /* Resend Start condition
	** Then send new control byte
	** then begin reading
	*/
    I2CMasterSlaveAddrSet(I2C0_BASE, bAddr, READ);

    while(I2CMasterBusy(I2C0_BASE));

    /* Begin Reading*/
    for(i = 0; i < cSize; i++) {

      if(cSize == i + 1 && cSize == 1) {
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

        DelayMs(1);

        while(I2CMasterBusy(I2C0_BASE));
      }
      else if(cSize == i + 1 && cSize > 1) {
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

        DelayMs(1);

        while(I2CMasterBusy(I2C0_BASE));
      }
      else if(i == 0) {
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

        DelayMs(1);

        while(I2CMasterBusy(I2C0_BASE));

        /* Idle wait*/
        while(I2CGenIsNotIdle());
      }
      else {
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);

        DelayMs(1);

        while(I2CMasterBusy(I2C0_BASE));

        /* Idle wait */
        while(I2CGenIsNotIdle());
      }

      while(I2CMasterBusy(I2C0_BASE));

      /* Read Data */
      *pbTemp = (char)I2CMasterDataGet(I2C0_BASE);

      pbTemp++;

    }

  }
  else if(fRW == WRITE) {

    /*Loop data bytes */
    for(i = 0; i < cSize; i++) {
      /* Send Data */
      I2CMasterDataPut(I2C0_BASE, *pbTemp);

      while(I2CMasterBusy(I2C0_BASE));

      if(i == cSize - 1) {
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);

        DelayMs(1);

        while(I2CMasterBusy(I2C0_BASE));
      }
      else {
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

        DelayMs(1);

        while(I2CMasterBusy(I2C0_BASE));

        /* Idle wait */
        while(I2CGenIsNotIdle());
      }

      pbTemp++;
    }

  }

  /*Stop*/

  return 0x00;

}

/* ------------------------------------------------------------ */
/***	I2CGenIsNotIdle()
 **
 **	Parameters:
 **		pbData	-	Pointer to transmit buffer (read or write)
 **		cSize	-	Number of byte transactions to take place
 **
 **	Return Value:
 **		TRUE is bus is not idle, FALSE if bus is idle
 **
 **	Errors:
 **		none
 **
 **	Description:
 **		Returns TRUE if the bus is not idle
 **
 */
bool I2CGenIsNotIdle() {

  return !I2CMasterBusBusy(I2C0_BASE);

}

/*===============================================================*/
/*===============================================================*/
/*======================|| Falldown Code ||======================*/
/*========================V=V=V=V=V=V=V=V========================*/
/*========================V=V=V=V=V=V=V=V========================*/

void falldown(){
  
  
}






