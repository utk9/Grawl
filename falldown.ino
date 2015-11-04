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

/* ------------------------------------------------------------ */
/*				Forward Declarations							*/
/* ------------------------------------------------------------ */
void DeviceInit();
char CheckSwitches();
void OrbitSetOled();

char I2CGenTransmit(char * pbData, int cSize, bool fRW, char bAddr);
bool I2CGenIsNotIdle();

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*                      Falldown Declarations                  */
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

typedef struct{
  int x;
  int y;
} ballStruct;

typedef struct{
  int x;
  int hole;
} platformStruct;

/* ---------------------- Game Variables --------------------- */


platformStruct platform[4];
ballStruct ball;

//In game time
long gameTime = 0;

//Velocities based on +x -> down, +y -> left

float vBallX;
float vBallY;
float vPlatform;

/* -------------- Bitmap and Bitmap Size Declarations -------- */
/* When holding device vertical, width represents how tall it is,
   height represents how wide it is */
   
int platformWidth = 2;
int platformHeight = 32;

int holeWidth = 2;
int holeHeight = 8;

int ballWidth = 2;
int ballHeight = 2;

//fills in 8 by 8 grid, tries to fill in width first, and then height
char rgBMPPlatform[] = {
  0xff, 0xff,
  0xff, 0xff,
  0xff, 0xff,
  0xff, 0xff
};

//seperate definition for the hole in the platform so it can simply be shifted
char rgBMPHole[] = {
  0x00, 0x00
};

char rgBMPBall[] = {
  0x03, 0x03
};

//used to erase BMPs
char rgBMPErasePlatform[] = {
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00
};

char rgBMPEraseBall[] = {
  0x00, 0x00
};

void setup()
{
  DeviceInit();
  gameInit();
  OrbitOledClear();
}

void loop()
{ 
  demo();
}

void demo(){
  delay(50 - gameTime/1000 >= 5 ? 50 - gameTime/10000 : 5);
  gameTime++;
  updateBall();
  updatePlatforms();
  updateScreen();
}

void gameInit(){
  accelInit();
  
  //sets up the platform[4] array
  for(int i = 0; i < 4; i++){
    platform[i].x = i*32+32;
    platform[i].hole = randY();
  } 
  //temporary for demo purposes
  platform[0].hole = 13;
  
  /** Vertically, this is what x and y is
     +y <-------|
                |
                |
                |
                v
                x                        **/
  ball.x = 10;
  ball.y = 15;
  
  vBallX = 1;
  vBallY = 0;
  vPlatform = -1;
}

void accelInit(){
  char 	chPwrCtlReg = 0x2D;
  char  chY0Addr = 0x34;
  char 	rgchWriteAccl[] = {
    0, 0            };
  
  /*
     * Enable I2C Peripheral
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

    /*
     * Set I2C GPIO pins
     */
    GPIOPinTypeI2C(I2CSDAPort, I2CSDA_PIN);
    GPIOPinTypeI2CSCL(I2CSCLPort, I2CSCL_PIN);
    GPIOPinConfigure(I2CSCL);
    GPIOPinConfigure(I2CSDA);

    /*
     * Setup I2C
     */
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);

    /* Initialize the Accelerometer
     *
     */
    GPIOPinTypeGPIOInput(ACCL_INT2Port, ACCL_INT2);

    rgchWriteAccl[0] = chPwrCtlReg;
    rgchWriteAccl[1] = 1 << 3;		// sets Accl in measurement mode
    I2CGenTransmit(rgchWriteAccl, 1, WRITE, ACCLADDR);
}

int accelRead(){
  short dataY;
  
  char  chY0Addr = 0x34;
    
  char rgchReadAccl2[] = {
    0, 0, 0            };
  
  /*
     * Read the X data register
     */
    rgchReadAccl2[0] = chY0Addr;
    I2CGenTransmit(rgchReadAccl2, 2, READ, ACCLADDR);
    
    dataY = (rgchReadAccl2[2] << 8) | rgchReadAccl2[1];
    
    return (int)dataY;
}

/* ------- Check Functions ------- */
void checkPlatforms(){
  
//if platform scrolls past top of screen, relabel the platform array,
//and add a new platform at the bottom of the screen

  if(platform[0].x <= 0){
    for(int i = 0; i < 3; i++){
      platform[i].x = platform[i+1].x;
      platform[i].hole = platform[i+1].hole;
    }
    platform[3].x = platform[2].x+32;
    platform[3].hole = randY();
  }
}

int checkPixel(int x, int y){
  OrbitOledMoveTo(x, y);
  return OrbitOledGetPixel();
}

/* ----- Update Functions ------ */
void updatePlatforms(){
  checkPlatforms();
  for(int i = 0; i < 4; i++){
    platform[i].x += vPlatform;
  }
}

void updateBall(){
  int aY = accelRead();
  vBallY = (aY < 50 && aY > -50) ? 0 : aY > 50 ? 2 : -2; 
  vBallX = 1;
  if(ball.x + vBallX >= 125){
    if(ball.x <= platform[3].x - 2 && ball.x + vBallX >= platform[3].x + vPlatform - 2){
      if(!(ball.y >= platform[3].hole && ball.y < platform[3].hole + 6)){
        ball.x = platform[3].x - 3;
        vBallX = vPlatform;
      }
    }else{
      ball.x = 125;
      vBallX = 0;
    }
    ball.x += vBallX;
  }else{
    for(int i = 0; i < 4; i++){
      //if ball is set to pass a platform after this update
      if(ball.x <= platform[i].x - 3 && ball.x + vBallX >= platform[i].x + vPlatform - 3){
        //if the ball is not above the hole
        if(!(ball.y >= platform[i].hole && ball.y < platform[i].hole + 7)){
          ball.x = platform[i].x - 3;
          vBallX = vPlatform;
          break;
        }
      }
    }
    ball.x += vBallX;
  }
  if(ball.y+vBallY >= 29){
    ball.y = 29;
    vBallY = 0;
  }else if(ball.y+vBallY <= 0){
    ball.y = 0;
    vBallY = 0;
  }else{
    ball.y += vBallY;
  }
  
}

void updateScreen(){
  for(int i = 0; i < 4; i++){
    //erase platform
    OrbitOledMoveTo(platform[i].x-vPlatform, 0);
    OrbitOledPutBmp(platformWidth, platformHeight, rgBMPErasePlatform);
    
    //draw new platform
    OrbitOledMoveTo(platform[i].x, 0);
    OrbitOledPutBmp(platformWidth, platformHeight, rgBMPPlatform);
      
    OrbitOledMoveTo(platform[i].x, platform[i].hole);
    OrbitOledPutBmp(holeWidth, holeHeight, rgBMPHole);
  }
  
  //erase ball
  if(ball.y == 0 || ball.y == 29){ //fixes glitch when ball was hitting walls
    OrbitOledMoveTo(ball.x - vBallX, 0);
    OrbitOledPutBmp(platformWidth, platformHeight, rgBMPErasePlatform);
  }else{
    OrbitOledMoveTo(ball.x - vBallX, ball.y - vBallY);
    OrbitOledPutBmp(ballWidth, ballHeight, rgBMPEraseBall);
  }
  
  //draw new ball
  OrbitOledMoveTo(ball.x, ball.y);
  OrbitOledPutBmp(ballWidth, ballHeight, rgBMPBall);
  
  OrbitOledMoveTo(0, 0);
  OrbitOledPutBmp(platformWidth, platformHeight, rgBMPErasePlatform);
  
  OrbitOledUpdate();
}

int randY(){
  return (int)(random(0, 25));
}

/* ~~~~~~~~~~~ BoosterPack Init and Other Stuff ~~~~~~~~~~~~~~ */

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







