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
/*                              Local Type Definitions          */
/* ------------------------------------------------------------ */
#define BLUE_LED  GPIO_PIN_2
#define G_LED GREEN_LED
#define R_LED RED_LED
 
 
/* ------------------------------------------------------------ */
/*                              Global Variables                */
/* ------------------------------------------------------------ */
extern int xchOledMax; // defined in OrbitOled.c
extern int ychOledMax; // defined in OrbitOled.c
 
 
/* ------------------------------------------------------------ */
/*                              Local Variables                 */
/* ------------------------------------------------------------ */
char    chSwtCur;
char    chSwtPrev;
bool    fClearOled;  
 
/* ------------------------------------------------------------ */
/*                              Forward Declarations                                                    */
/* ------------------------------------------------------------ */
void DeviceInit();
char CheckSwitches();
void OrbitSetOled();
 
char I2CGenTransmit(char * pbData, int cSize, bool fRW, char bAddr);
bool I2CGenIsNotIdle();
 
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*                      Falldown Declarations                  */
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
 
#define MENU 0
#define GAME 1
#define DEAD 2
#define MENU_ANIMATION 3
 
long btnCooldown;
 
typedef struct{
  int x;
  int y;
} ballStruct;
 
typedef struct{
  int x;
  int hole;
} platformStruct;
 
typedef struct{
  int x;
  int y;
  int lastTimeCreated;
} slowMotionPowerUpStruct;
 
/* ------------------------- Font ---------------------------- */
char f_space[] = {0, 0, 0, 0, 0, 0, 0, 0}; //space
char f_zero[] = {0b00111000,
                 0b01000100,
                 0b01000100,
                 0b01000100,
                 0b01000100,
                 0b01000100,
                 0b00111000, 0}; //0
char f_one[] = {0b00110000, 0b00010000,  0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00111000, 0}; //1
char f_two[] = {0b00111000, 0b001000100, 0b00000100, 0b0001000, 0b00010000, 0b00100100, 0b01111100, 0}; //2
char f_three[] = {0b00111000, 0b01000100, 0b00000100, 0b00011000, 0b00000100, 0b01000100, 0b00111000, 0}; //3
char f_four[] = {0b00011000, 0b00101000, 0b01001000, 0b01111100, 0b00001000, 0b00001000, 0b00001000, 0}; //4
char f_five[] = {0b01111100, 0b1000000, 0b01000000, 0b00111000, 0b00000100, 0b01000100, 0b00111100, 0}; //5
char f_six[] = {0b00010000, 0b00100000, 0b01000000, 0b01111000, 0b01000100, 0b01000100, 0b00111000, 0}; //6
char f_seven[] = {0b01111100, 0b01000100, 0b00000100, 0b000001000, 0b00001000, 0b00010000, 0b00010000, 0}; //7
char f_eight[] = {0b00111000, 0b01000100, 0b01000100, 0b00111000, 0b01000100, 0b01000100, 0b00111000, 0}; //8
char f_nine[] = {0b00111000, 0b01000100, 0b01000100, 0b00111100, 0b00001000, 0b00001000, 0b00010000, 0}; //9
char f_D[] = {0b11111100, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b11111100, 0}; //D
char f_E[] = {0b01111110, 0b11000010, 0b01001000, 0b01111000, 0b01001000, 0b01000010, 0b01111110, 0b10000000}; //E
char f_F[] = {0b11111110, 0b01000010, 0b01001010, 0b01111000, 0b01001000, 0b01000000, 0b11100000, 0}; //F
char f_G[] = {0b00111000, 0b01000100, 0b10000000, 0b10011110, 0b10000100, 0b01000100, 0b00111000, 0}; //G
char f_O[] = {0b00011100,
              0b00100010,
              0b01000001,
              0b01000001,
              0b01000001,
              0b00100010,
              0b00011100, 0}; //O
char f_P[] = {0b11111100, 0b01000010, 0b01000010, 0b01111100, 0b01000000, 0b01000000, 0b11100000, 0}; //P
char f_S[] = {0b01111110, 0b10000010, 0b10000001, 0b01111100, 0b00000010, 0b10000010, 0b11111100, 0}; //S
char f_a[] = {0b00000000, 0b00111100, 0b01000100, 0b00111100, 0b01000100, 0b01000100, 0b00111010, 0}; //a
char f_e[] = {0b00000000, 0b00111000, 0b01000100, 0b01111100, 0b01000000, 0b01000100, 0b00111000, 0}; //e
char f_h[] = {0b11000000, 0b01000000, 0b01011000, 0b01100100, 0b01000100, 0b01000100, 0b11101110, 0}; //h
char f_l[] = {0b00110000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b01111100, 0}; //l
char f_m[] = {0b00000000, 0b11101100, 0b01010100, 0b01010100, 0b01010100, 0b01010100, 0b11010110, 0}; //m
char f_n[] = {0b00000000, 0b11011000, 0b01100100, 0b01000100, 0b01000100, 0b01000100, 0b11101110, 0}; //n
char f_o[] = {0b00000000, 0b00111000, 0b01000100, 0b01000100, 0b01000100, 0b01000100, 0b00111000, 0}; //o
char f_r[] = {0b00000000, 0b01101100, 0b00110010, 0b00100000, 0b00100000, 0b00100000, 0b01110000, 0}; //r
char f_s[] = {0b00000000, 0b00111100, 0b01000100, 0b00111000, 0b00000100, 0b01000100, 0b01111000, 0}; //s
char f_t[] = {0b00100000, 0b00100000, 0b01111000, 0b00100000, 0b00100000, 0b00100100, 0b00011000, 0}; //t
char f_u[] = {0b00000000, 0b01100110, 0b00100010, 0b00100010, 0b00100010, 0b00100110, 0b00011011, 0}; //u
char f_v[] = {0b00000000, 0b11101110, 0b01000100, 0b01000100, 0b00101000, 0b00101000, 0b00010000, 0}; //v
char f_w[] = {0b00000000, 0b11101110, 0b01000100, 0b01010100, 0b01010100, 0b00101000, 0b00101000, 0}; //w
char f_y[] = {0b00000000, 0b11101110, 0b01000100, 0b00101000, 0b00010000, 0b00010000, 0b01100000, 0}; //y
 
char spikes[] = {0b11111,
                 0b01110,
                 0b00100};
 
 
/* ---------------------- Game Variables --------------------- */
 
 
platformStruct platform[4];
ballStruct ball;
slowMotionPowerUpStruct slowMotionPowerUp;
 
//In game time
long gameTime = 0;
int delayTime;
 
int score;
int mode;
 
//Velocities based on +x -> down, +y -> left
 
float vBallX;
float vBallY;
float vPlatform;
 
/* -------------- Bitmap and Bitmap Size Declarations -------- */
/* When holding device vertical, width represents how tall it is,
   height represents how wide it is */
   
int platformWidth = 3;
int platformHeight = 32;
 
int holeWidth = 3;
int holeHeight = 8;
 
int ballWidth = 2;
int ballHeight = 2;

int timeInSlowMotion;
 
int ledDelay = 0;
 
 
 
//fills in 8 by 8 grid, tries to fill in width first, and then height
char rgBMPPlatform[] = {
  0b11111111, 0b11111111, 0b11111111,
  0b11111111, 0b11111111, 0b11111111,
  0b11111111, 0b11111111, 0b11111111,
  0b11111111, 0b11111111, 0b11111111
};
 
char rgBMPPlatformHollow[] = {
  0b11111111, 0b00000001, 0b11111111,
  0b11111111, 0b00000000, 0b11111111,
  0b11111111, 0b00000000, 0b11111111,
  0b11111111, 0b10000000, 0b11111111
};
 
char rgBMPSlowMotionPowerUp [] ={
  0b00100,
  0b01110,
  0b11111,
  0b01110,
  0b00100
};
 
char rgBMPPlatformStripe[] = {
  0b11111111, 0b01010101, 0b11111111,
  0b11111111, 0b01010101, 0b11111111,
  0b11111111, 0b10101010, 0b11111111,
  0b11111111, 0b10101010, 0b11111111
};
 
//seperate definition for the hole in the platform so it can simply be shifted
char rgBMPHole[] = {
  0b10000001,
  0b10000001,
  0b10000001
 
};
 
char rgBMPBall[] = {
  0b11, 0b11
};
 
//used to erase BMPs
char rgBMPErase[512];
 
char bR[128];
 
void setup()
{
  DeviceInit();
  gameInit();
  OrbitOledClear();
}
 
void loop()
{
  switch(mode){
    case 0: menu();
               break;
    case 1: game();
               break;
    case 2: death();
               break;
    case 3: menuAnimation();
  }
}
 
void game(){
  delay(delayTime);
  gameTime++;
  if ((gameTime - timeInSlowMotion) > 35) delayTime = 15;
  eraseScreen();
  updateBall();
  updatePlatforms();
  if (gameTime - slowMotionPowerUp.lastTimeCreated > 270) updateSlowMotionPowerUp();
  else updateHeightSlowMotionPowerUp();
  
  checkSlowMotionPowerUp();
  
  drawScreen();
  OrbitOledUpdate();
  updateLED();
}
 
 
/**
void menu(){
  if(--btnCooldown < 0 && (digitalRead(PUSH1) == LOW || digitalRead(PUSH2) == LOW)){
    mode = GAME;
   
    //reset everything in game
    ball.x = 10;
    ball.y = 15;
    vBallX = 1;
    vBallY = 0;
    vPlatform = -1;
    gameTime = 0;
    score = 0;
  }
  eraseScreen();
  OrbitOledMoveTo(0, 0);
  OrbitOledPutBmp(platformWidth, platformHeight, rgBMPPlatform);
 
  stringDraw("Fall SE             Push to Play", 32, 8, 23);
 
  OrbitOledMoveTo(31, 0);
  OrbitOledPutBmp(platformWidth, platformHeight, rgBMPPlatform);
 
  OrbitOledUpdate();
}
**/
 
 
//Creates the falling letter animation
void fallLetter(char *letter, int xf, int yi){
  int yf = yi;
  int x;
  for(x = 0; x < xf; x += 5){
    OrbitOledMoveTo(x, yi);
    OrbitOledPutBmp(8, 8, letter);
    OrbitOledUpdate();
    delay(10);
    OrbitOledMoveTo(0, yi);
    OrbitOledPutBmp(128, 8, bR);
  }
  OrbitOledMoveTo(x, yi);
  OrbitOledPutBmp(8, 8, letter);
  OrbitOledUpdate();
}
 
void animateLetter(char *letter, int coi, int cof, int inc, int dir){
  int v = inc*(1 - 2*(coi > cof));
  //switch(dir){
        //case X:;
        //case Y:;
  //}
}
 
//Starts the menu animation, when done, cuts to static menu screen
void menuAnimation(){
  fallLetter(f_F, 50, 24);
  fallLetter(f_a, 50, 16);
  fallLetter(f_l, 50, 8);
  fallLetter(f_l, 50, 0);
  mode = MENU;
}
 
void menu(){
  if(--btnCooldown < 0 && (digitalRead(PUSH1) == LOW || digitalRead(PUSH2) == LOW)){
    mode = GAME;
  }
 
  eraseScreen();
  OrbitOledMoveTo(50, 24);
  OrbitOledPutBmp(8, 8, f_F);
  OrbitOledMoveTo(50, 16);
  OrbitOledPutBmp(8, 8, f_a);
  OrbitOledMoveTo(50, 8);
  OrbitOledPutBmp(8, 8, f_l);
  OrbitOledMoveTo(50, 0);
  OrbitOledPutBmp(8, 8, f_l);
  OrbitOledUpdate();
 
}
 
void death(){
  char strBig[4];
  char strSml[2];
 
  if(--btnCooldown < 0 && (digitalRead(PUSH1) == LOW || digitalRead(PUSH2) == LOW)){
    gameInit();
  }
 
  eraseScreen();
  OrbitOledMoveTo(0, 0);
  OrbitOledPutBmp(platformWidth, platformHeight, rgBMPPlatform);
 
  stringDraw("GameOver", 8, 8, 23);
 
  if(score > 99){
    sprintf(strBig, "%04d", score);
    stringDraw(strBig, 4, 53, 23);
  }else{
    sprintf(strSml, "%02d", score);
    stringDraw(strSml, 2, 53, 15);
  }
  OrbitOledMoveTo(31, 0);
  OrbitOledPutBmp(platformWidth, platformHeight, rgBMPPlatform);
  OrbitOledUpdate();
}
 
void gameInit(){
  pinMode(G_LED, OUTPUT);
  pinMode(R_LED, OUTPUT);
 
  pinMode(PUSH1, INPUT_PULLUP);
  pinMode(PUSH2, INPUT_PULLUP);
 
  accelInit();
 
  mode = MENU_ANIMATION;
  score = 0;
  btnCooldown = 500;
 
  //sets up erase BMP
  for(int i = 0; i < 512; i++){
    rgBMPErase[i] = 0;
  }
 
  for(int i = 0; i < 128; i++){
    bR[i] = 0;
  }
 
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
 
  gameTime = 35;
  delayTime = 15;
}
 
void accelInit(){
  char  chPwrCtlReg = 0x2D;
  char  chY0Addr = 0x34;
  char  rgchWriteAccl[] = {
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
    rgchWriteAccl[1] = 1 << 3;          // sets Accl in measurement mode
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

    checkBall();
  }
}
 
 
void checkSlowMotionPowerUp (){
  if ((abs(ball.y-slowMotionPowerUp.y) < 2) && (abs(ball.x-slowMotionPowerUp.x)<5)) {
    delayTime = 30; 
    slowMotionPowerUp.x = 0;
    timeInSlowMotion = gameTime;
  }
}
 
void updateSlowMotionPowerUp(){
  slowMotionPowerUp.y = (int) random(5, 27);
  slowMotionPowerUp.x = platform[3].x-6;
  slowMotionPowerUp.lastTimeCreated = gameTime;
  
}

void updateHeightSlowMotionPowerUp(){
  //slowMotionPowerUp.x = platform[3].x-4;  
  slowMotionPowerUp.x +=vPlatform;
}
 
void checkBall(){
  if(ball.x + vBallX >= 125){
    if(!(ball.y >= platform[3].hole && ball.y < platform[3].hole + 6)){
        ball.x = platform[3].x - 3 + vPlatform;
        vBallX = vPlatform;
      }
  }
}
 
/* ----- Update Functions ------ */
void updatePlatforms(){
  checkPlatforms();
  vPlatform = -1 - (gameTime/1000);
  if(vPlatform <= -5) vPlatform = -5;
  for(int i = 0; i < 4; i++){
    platform[i].x += vPlatform;
  }
}
 
void updateBall(){
  int aY = accelRead();
  vBallY = (aY < 20 && aY > -20) ? 0 : aY >= 30 ? vBallY+(int)(aY/60) : vBallY+(int)(aY/60);
  if(vBallY > 2) vBallY = 2;
  if(vBallY < -2) vBallY = -2;
  vBallX += 1;
  if(vBallX > 2) vBallX = 2; //limit gravity to 2
 
  if(ball.x + vBallX >= 125){
    if(ball.x <= platform[3].x - 2 && ball.x + vBallX >= platform[3].x + vPlatform - 2){
      if(!(ball.y >= platform[3].hole && ball.y < platform[3].hole + 6)){
        ball.x = platform[3].x - 3;
        vBallX = vPlatform;
      }
    }
    else{
      ball.x = 125;
      vBallX = 0;
    }
    ball.x += vBallX;
  }else if(ball.x + vBallX < 0){
    mode = DEAD;
    btnCooldown = 500;
  }else{
    for(int i = 0; i < 4; i++){
      //if ball is set to pass a platform after this update
      if(ball.x <= platform[i].x - 3 && ball.x + vBallX >= platform[i].x + vPlatform - 3){
        //if the ball is not above the hole
        if(!(ball.y >= platform[i].hole && ball.y < platform[i].hole + 5)){
          ball.x = platform[i].x - 3;
          vBallX = vPlatform;
          break;
        } else score++;
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
 
void eraseScreen(){
  OrbitOledMoveTo(0, 0);
  OrbitOledPutBmp(128, 32, rgBMPErase);
}
 
void drawScreen(){
  for(int i = 0; i < 4; i++){
   //draw new platform
    OrbitOledMoveTo(platform[i].x, 0);
    OrbitOledPutBmp(platformWidth, platformHeight, rgBMPPlatformStripe);
     
    OrbitOledMoveTo(platform[i].x, platform[i].hole);
    OrbitOledPutBmp(holeWidth, holeHeight, rgBMPHole);
  }
 
  //draw new ball
  OrbitOledMoveTo(ball.x, ball.y);
  OrbitOledPutBmp(ballWidth, ballHeight, rgBMPBall);
 
  //draw slow motion powerup if applicable
  //if (slowMotionPowerUp.val>5){
    OrbitOledMoveTo(slowMotionPowerUp.x, slowMotionPowerUp.y);
    OrbitOledPutBmp(5, 5, rgBMPSlowMotionPowerUp);
  //}
 
  //draw score
  char strBig[3];
  char strSml[2];
  char strDigit[1];
  if(score < 10) //if 1 digit
  {
    sprintf(strDigit, "%d", score);
    stringDraw(strDigit, 1, 8, 0);
  }
  if(score > 99) //if 3 digits
  {
    sprintf(strBig, "%03d", score);
    stringDraw(strBig, 3, 8, 16);
  }
  if(score <= 99 && score >= 10) //if 2 digits
  {
    sprintf(strSml, "%02d", score);
    stringDraw(strSml, 2, 8, 8);
  }
 
  //draw spikes
  for(int i = 32; i > 0; i-=5){
     OrbitOledMoveTo(0, i);
     OrbitOledPutBmp(3, 5, spikes);  
  }
}
 
int randY(){
  return (int)(random(0, 25));
}
 
void updateLED(){
  if(ball.x <= 20)
  {
    if(ledDelay >= 2){
      digitalWrite(R_LED, LOW);
    }
    if(ledDelay >= 4){
      ledDelay = 0;
    }
    if(ledDelay < 2)
    {
      digitalWrite(R_LED, HIGH);
    }
    ledDelay++;
  }
  else if(ball.x <= 42)
  {
    digitalWrite(R_LED, HIGH);
    digitalWrite(G_LED, LOW);
  }
  else if(ball.x <= 84)
  {
    digitalWrite(R_LED, HIGH);
    digitalWrite(G_LED, HIGH);
  }
  else
  {
    digitalWrite(R_LED, LOW);
    digitalWrite(G_LED, HIGH);
  }
}
 
// Font Functions
 
//prints string to screen
void stringDraw(char s[],int l, int x, int y){
  OrbitOledMoveTo(y > 23 ? 23 : y < 0 ? 0 : y, x > 120 ? 120 : x < 0 ? 0 : x);
  for(int i = 0; i < l; i++){
    if(y < -1){
      y = 23;
      x += 9;
    }
    if(x > 120) break;
    OrbitOledMoveTo(x, y);
    OrbitOledPutBmp(8, 8, chBMP(s[i]));
    y -= 8;
  }
}
 
// Returns bmp for given char
char* chBMP(char c){
        switch(c){
                case ' ': return f_space; break;
                case '0': return f_zero; break;
                case '1': return f_one; break;
                case '2': return f_two; break;
                case '3': return f_three; break;
                case '4': return f_four; break;
                case '5': return f_five; break;
                case '6': return f_six; break;
                case '7': return f_seven; break;
                case '8': return f_eight; break;
                case '9': return f_nine; break;
                case 'D': return f_D; break;
                case 'E': return f_E; break;
                case 'F': return f_F; break;
                case 'G': return f_G; break;
                case 'O': return f_O; break;
                case 'P': return f_P; break;
                case 'S': return f_S; break;
                case 'a': return f_a; break;
                case 'e': return f_e; break;
                case 'h': return f_h; break;
                case 'l': return f_l; break;
                case 'm': return f_m; break;
                case 'n': return f_n; break;
                case 'o': return f_o; break;
                case 'r': return f_r; break;
                case 's': return f_s; break;
                case 't': return f_t; break;
                case 'u': return f_u; break;
                case 'v': return f_v; break;
                case 'w': return f_w; break;
                case 'y': return f_y; break;
        }
};
 
 
/* ~~~~~~~~~~~ BoosterPack Init and Other Stuff ~~~~~~~~~~~~~~ */
 
/* ------------------------------------------------------------ */
/***    DeviceInit
 **
 **     Parameters:
 **             none
 **
 **     Return Value:
 **             none
 **
 **     Errors:
 **             none
 **
 **     Description:
 **             Initialize I2C Communication, and GPIO
 */
void DeviceInit()
{
  /*
   * First, Set Up the Clock.
   * Main OSC             -> SYSCTL_OSC_MAIN
   * Runs off 16MHz clock -> SYSCTL_XTAL_16MHZ
   * Use PLL              -> SYSCTL_USE_PLL
   * Divide by 4          -> SYSCTL_SYSDIV_4
   */
  SysCtlClockSet(SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_USE_PLL | SYSCTL_SYSDIV_4);
 
  /*
   * Enable and Power On All GPIO Ports
   */
  //SysCtlPeripheralEnable(     SYSCTL_PERIPH_GPIOA | SYSCTL_PERIPH_GPIOB | SYSCTL_PERIPH_GPIOC |
  //                                            SYSCTL_PERIPH_GPIOD | SYSCTL_PERIPH_GPIOE | SYSCTL_PERIPH_GPIOF);
 
  SysCtlPeripheralEnable(       SYSCTL_PERIPH_GPIOA );
  SysCtlPeripheralEnable(       SYSCTL_PERIPH_GPIOB );
  SysCtlPeripheralEnable(       SYSCTL_PERIPH_GPIOC );
  SysCtlPeripheralEnable(       SYSCTL_PERIPH_GPIOD );
  SysCtlPeripheralEnable(       SYSCTL_PERIPH_GPIOE );
  SysCtlPeripheralEnable(       SYSCTL_PERIPH_GPIOF );
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
/***    CheckSwitches()
 **
 **     Parameters:
 **             none
 **
 **     Return Value:
 **             none
 **
 **     Errors:
 **             none
 **
 **     Description:
 **             Return the state of the Switches
 */
char CheckSwitches() {
 
  long  lSwt1;
  long  lSwt2;
 
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
/***    I2CGenTransmit
 **
 **     Parameters:
 **             pbData  -       Pointer to transmit buffer (read or write)
 **             cSize   -       Number of byte transactions to take place
 **
 **     Return Value:
 **             none
 **
 **     Errors:
 **             none
 **
 **     Description:
 **             Transmits data to a device via the I2C bus. Differs from
 **             I2C EEPROM Transmit in that the registers in the device it
 **             is addressing are addressed with a single byte. Lame, but..
 **             it works.
 **
 */
char I2CGenTransmit(char * pbData, int cSize, bool fRW, char bAddr) {
 
  int           i;
  char *                pbTemp;
 
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
/***    I2CGenIsNotIdle()
 **
 **     Parameters:
 **             pbData  -       Pointer to transmit buffer (read or write)
 **             cSize   -       Number of byte transactions to take place
 **
 **     Return Value:
 **             TRUE is bus is not idle, FALSE if bus is idle
 **
 **     Errors:
 **             none
 **
 **     Description:
 **             Returns TRUE if the bus is not idle
 **
 */
bool I2CGenIsNotIdle() {
 
  return !I2CMasterBusBusy(I2C0_BASE);
 
}
