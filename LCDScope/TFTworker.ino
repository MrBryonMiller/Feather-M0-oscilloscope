#include "TouchScreen.h"
// These are the four touchscreen analog pins
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 11   // can be a digital pin
#define XP 12   // can be a digital pin

#define TS_MINX 201
#define TS_MINY 802
#define TS_MAXX 868
#define TS_MAXY 350

int ACintflag;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 272);

bool ACpinsSetup;
int report=-8000;
int reporttime=1000,reportcnt;
int noInputTime;

void TFTsetup() 
{
setupAC(26);
//ts.getPoint();
}

void ShowMenu()
{
Serial.println("Enter Menu");

// stop dma
DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
tft.fillScreen(BLACK);
tft.setTextSize(4);
tft.setTextColor(BLACK); 
tft.fillRect(0, 260, 480, 60, RED);
tft.setCursor(240-2*8*4,275);
tft.println("DONE");  

tft.setTextColor(WHITE); 
tft.setTextSize(3);
tft.setCursor(0,0);
tft.println("TRIGGER");  
drawTrigFeat();

int x0=200;
tft.setCursor(x0-50,0);
tft.println("LEVEL");  
tft.fillTriangle(x0,30,x0-40,30+70,x0+40,30+70,WHITE);
showTrigLevOnMenu();
//tft.setCursor(x0-30,30+70+3);
// 
//float temp;
//if (inputADC == A1)
//     {
//     temp = triglevel/200.0;
//     tft.println(temp,2);  
//     }
//else tft.println(triglevel);  
tft.fillTriangle(x0,200,x0-40,30+70+30,x0+40,30+70+30,WHITE);

x0= 310;
tft.setCursor(x0-50,0);
tft.println("SWEEP");  
tft.fillTriangle(x0,30,x0-40,30+70,x0+40,30+70,WHITE);
tft.setCursor(x0-30,30+70+3);
tft.println(msecPerSweep);  
tft.fillTriangle(x0,200,x0-40,30+70+30,x0+40,30+70+30,WHITE);

x0= 420;
tft.setCursor(x0-50,0);
tft.println("INPUT");  
tft.fillTriangle(x0,30,x0-40,30+70,x0+40,30+70,WHITE);
tft.setCursor(x0-30,30+70+3);
if (inputADC == A1)
     tft.println("0~5");  
else tft.println("+-15");  
tft.fillTriangle(x0,200,x0-40,30+70+30,x0+40,30+70+30,WHITE);

adc_uninit();
ts.getPoint();
noInputTime = millis();
}

int blinktime,blink;

void workMenu()
{
if (millis()-noInputTime > 20000)
     {
     Serial.println("no input for some time");
     prepRetToScope();
     return;
     }
int blinkrate = 500 - 450*(millis()-noInputTime-10000)/10000;
if ((millis()-noInputTime > 10000) && ((millis()-blinktime)> blinkrate))
     {
     blinktime = millis();
     blink = !blink;
     if (blink)
          tft.fillRect(470, 0, 10, 10, BLACK);
     else tft.fillRect(470, 0, 10, 10, WHITE);
     digitalWrite(LED_BUILTIN, false);
     }
if (!readAC)
     return;
bool hit=false;
TSPoint p = ts.getPoint();

//align to rotated screen
int16_t t = p.x;
p.x = p.y;
p.y = t; 
// Scale from ~0->1000 to tft.width using the calibration #'s
p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
if ((p.z < 20) || (p.z > 1000) || (p.x > 480) || (p.y > 320))
     return;
if (!blink)
     {
     tft.fillRect(470, 0, 10, 10, BLACK);
     blink = true;
     }
digitalWrite(LED_BUILTIN, true);
noInputTime = millis();
/*
// Write cursor position on screen
tft.setCursor(300,220);
tft.fillRect(300, 220, 4*6*7+2, 30, HX8357_BLACK);
tft.setTextColor(HX8357_YELLOW);
tft.print(p.x);
tft.print(" ");
tft.println(p.y);
tft.setTextColor(HX8357_WHITE); 
*/
if (p.y > 260)
     {
     prepRetToScope();
     return;
     }
//get Trigger feature
if ((p.x < 128) && (p.y > 30))
     {
     int temp = (p.y-30)/70;
     switch (temp)
          {
          case 0: trigfeature = FREERUN;
               break;
          case 1: trigfeature = PLUSSLOPE;
               break;
          default:trigfeature = NEGSLOPE;
          }
     drawTrigFeat();
     showTrigLevOnMenu();
     hit=true;
     }
//get Trigger level
if ((p.x > 160) && (p.x < 240) && (trigfeature != FREERUN))
     {
     triglevel=(triglevel/50)*50;
     if (p.y < 120)
          {
          if (inputADC == A1)
               fltTrigLevel+=0.25; 
          else fltTrigLevel+=1.0; 
          }
     else {
          if (inputADC == A1)
               fltTrigLevel-=0.25; 
          else fltTrigLevel-=1.0; 
          }
     showTrigLevOnMenu();
     hit=true;
     }
//get Sweep speed
if ((p.x > 270) && (p.x < 350))// && (p.y > 30) && (p.y < 100))
     {
     int sweepind = 0;
     for (sweepind=0;sweepind<sweepNumssize-1;sweepind++)
          {
          if (sweepNums[sweepind] == msecPerSweep)
               break;
          }
     if (p.y < 120)
          {
          sweepind++;
          if (sweepind > sweepNumssize-1)
               sweepind = sweepNumssize-1;
          }
     else {
          sweepind--;
          if (sweepind < 0)
               sweepind = 0;
          }
     msecPerSweep = sweepNums[sweepind];
     tft.fillRect(280,30+70+3, 4*6*3+2, 28, BLACK);
     tft.setCursor(280,30+70+3);
     tft.println(msecPerSweep);  
     hit=true;
     }
//get input ADC
if ((p.x > 380) && (p.x < 460))// && (p.y > 30) && (p.y < 100))
     {
     tft.fillRect(390,30+70+3, 4*6*3+2, 28, BLACK);
     tft.setCursor(390,30+70+3);
     if (inputADC == A1)
          {
          inputADC = A4;
          tft.println("+-15");  
          fltTrigLevel = 0.0;
          }
     else {
          inputADC = A1;
          tft.println("0~5"); 
          fltTrigLevel = 2.5; 
          }
     // show triglevel since it will be displayed differently
     showTrigLevOnMenu();
     hit=true;
     }

if (hit)
     delay(250);               
}

void showTrigLevOnMenu()
{
tft.fillRect(170,30+70+3, 3*6*5+2, 28, BLACK);
if (trigfeature == FREERUN)
     disTrigLevel = "----";
else if (inputADC == A1)
          {
          fltTrigLevel=constrain(fltTrigLevel,0.0,4.75); 
          triglevel = fltTrigLevel*200;
          disTrigLevel = String(fltTrigLevel,2);
          }
     else //tft.println(triglevel);  
          {
          fltTrigLevel=constrain(fltTrigLevel,-15.0,15.0); 
          triglevel = (fltTrigLevel+15.0)*1000.0/30.0;
          disTrigLevel = String(fltTrigLevel,1);
          }
tft.setCursor(170,30+70+3);
tft.println(disTrigLevel);  
//Serial.print("triglevel ");
//Serial.println(triglevel);
}
void prepRetToScope()
{
Serial.println("Exit Menu");
ACpinsSetup = false;

adc_init(inputADC);
showScope(true);
dmabufftimelong = 0;
dmabufftimelongreported = 0;
dmabufftimeshort = 5000;
dmabufftimeshortreported = 5000;
DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
delay(3);
menuShowing = false;
dmabufftime_s = micros();
}

void drawTrigFeat()
{
tft.setTextColor(BLACK); 
for (int i=0;i<3;i++)
     {
     int featy=30+i*70;
     int color = (trigfeature==i)?GREEN:WHITE;
     tft.fillRect(0, featy, 7*6*3+2, 65, color);
     tft.setCursor(2*6*3,featy+20);
     tft.println(trigSpell[i]); 
     }

tft.setTextColor(WHITE); 
}

// Synch AC
static __inline__ void ACsync() __attribute__((always_inline, unused));
static void   ACsync() {
  while (AC->STATUSB.bit.SYNCBUSY == 1);
}
// Synch GCLK
static __inline__ void syncADC() __attribute__((always_inline, unused));
static void syncGCLK() {
  while (GCLK->STATUS.bit.SYNCBUSY == 1);
}

//  Setup the AC (Analogue Comparator) with threshold values
// AC0level and AC1level in 64 steps to VDD = +3.3 V
void setupAC(uint8_t AC0level)
{
// Set up the AC clocks
syncGCLK();
GCLK->CLKCTRL.reg = 0x4120; //enable GGCLK for AC_ana, CLKGEN1 = 32 kHz Xtal
syncGCLK();
REG_GCLK_CLKCTRL = 0x401F; //enable GGCLK for AC_dig, CLKGEN0 = 48 MHz PLL
syncGCLK();
// Set up the AC
REG_PM_APBCMASK |= PM_APBCMASK_AC;        // Set the AC bit in the APBCMASK register
REG_AC_CTRLA = 0x00;      // Dissable comparator(s)
ACsync();
REG_AC_COMPCTRL0 = 0x000524;  // MXUp0s = AIN0 = A3  pin, int on rising
ACsync();
REG_AC_INTENSET = 0x01;   // generate an interrupt
REG_AC_WINCTRL = 0x00;    // Single comparator modes
ACsync();
REG_AC_SCALER0 = AC0level;    // Set threshold level
REG_AC_COMPCTRL0 |= 0x00001;
ACsync();
REG_AC_CTRLA = 0x02;      // Enable comparator(s)
ACsync();
}


int catch91;
int clearTime;

// Read the Analogue Comparator 
uint8_t readAC()
{
if (!ACpinsSetup)
     {
     pinMode(12, INPUT);
     digitalWrite(12, LOW);
     pinMode(A3, INPUT);
     digitalWrite(A3, LOW);
     pinMode(A2, OUTPUT);
     digitalWrite(A2, HIGH);
     pinMode(11, OUTPUT);
     digitalWrite(11, LOW);
     ACpinsSetup = true;
     delayMicroseconds(1000);
     catch91=0;
     }
if ((millis() < 5000) || ((millis()-clearTime) > 1000))
     {
     catch91=0;
     clearTime = millis();
     }
if (REG_AC_STATUSA)
     catch91++;
//return catch91 > 3;
return catch91 > 0;
//uint8_t temp = ACintflag;
//ACintflag = 0;
//return temp;

}

void AC_Handler()
{
//ACintflag = 0;  // Clear any previous interrupt
ACintflag = REG_AC_INTFLAG; // Copy the interrupt flag register
REG_AC_INTFLAG = 0x01;  // Reset the interrupt
Serial.print('.');
}

