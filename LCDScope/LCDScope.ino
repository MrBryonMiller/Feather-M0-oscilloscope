#include "dmac-adc.h"
#include "LCDworker.h"
#include "scopeworker.h"
#include "TFTworker.h"

#define SQUAREWAVEPIN 5
#define INPBUFSIZE  150
char buffer[INPBUFSIZE+1];

long lastMilliSeen,currentMilliSeen;
long blinkMilli,blinkLEDon;
long SWstatustime;

long sineTime=25; // 25 corresponds to 2Hz sine wave
volatile bool isLEDOn;
bool printSineMode;
bool printscopebuf;
bool enableSine = true;
volatile bool sqFromInt = true;

void setup()
{
Serial.begin(9600);  //Initialize serial and wait for port to open:
//normally we would wait here for Serial
//but LCDSrtup takes long enough that
//we don't have to
LCDsetup();
//int waitcnt = 0;
//while(!Serial && (waitcnt++ < 10))  // wait (only so long) for serial port to connect.
//     {
//     delay(100);
//     digitalWrite(LED_BUILTIN, waitcnt & 1);
//     }

Serial.println("LCDScope ");
Serial.print("Available RAM ");
Serial.println (FreeRam());


pinMode(LED_BUILTIN,OUTPUT);
digitalWrite(LED_BUILTIN, LOW);

analogWriteResolution(10);   // get Sine Wave DAC setup
analogWrite(A0,128);

dma_init();
adc_init(inputADC);
scope_init();


pinMode(SQUAREWAVEPIN,OUTPUT);

//start the sinewave timer at 500 Hz
startTimer(500);

buffloc1 = &adcbuf1[0];
buffloc2 = &adcbuf2[0];
buffloc = buffloc1;
//saveloc1 = &savebuf1[0];
//saveloc2 = &savebuf2[0];
//saveloc = saveloc1;
adcbuf1[HWORDS-1] = 0x7fff;
adcbuf2[HWORDS-1] = 0x7fff;
//timer[0] = micros();
adc_dma(adcbuf1,adcbuf2,HWORDS);
//LCDsetup();
TFTsetup();
}

void loop()
{
if (menuShowing)
     {
     if (menuShowing_s)
          workMenu();
     else ShowMenu();
     menuShowing_s=true;
     }
else {
     LCDworker();
     // check Analog Comparator to see if screen has been pressed
     menuShowing = readAC();
     menuShowing_s = false;
     }
//delay(100);
bool dumpMode = false;
if (Serial.available() && readline())
     {
     Serial.print("\nreceived ");
     Serial.println(buffer);
     char *p;
     p = strtok (buffer," ");
     dumpMode = !strcmp(p, "?");
     if (!strcmp(p, "sq"))
          {
          sqFromInt = !sqFromInt;
          Serial.print("sqFromInt ");
          Serial.println(sqFromInt);
          }
     if (!strcmp(p, "S"))
          {
          enableSine = !enableSine;
          Serial.print("enable sine ");
          Serial.println(enableSine);
          }
     if (!strcmp(p, "s"))
          {
          p = strtok (NULL, " ");
          char * pEnd;
          int t = strtol(p,&pEnd,10);
          if (t > 1)
               {
               Serial.print("Sine Time ");
               Serial.println(t);
               sineTime=t;
               }
          }
     if (!strcmp(p, "a"))
          {
          p = strtok (NULL, " ");
          char * pEnd;
          int t = strtol(p,&pEnd,10);
          if (t > 1)
               {
               Serial.print("Analog read ");
               Serial.println(analogRead(A0+t));
               }
          }
     if (!strcmp(p, "dr"))
          {
          p = strtok (NULL, " ");
          Serial.println(digitalRead(5));
          startTimer(250);
          }
     if (!strcmp(p, "ai"))
          {
          p = strtok (NULL, " ");
          char * pEnd;
          int t = strtol(p,&pEnd,10);
          if (t > 1)
               {
               Serial.print("Analog read ");
               Serial.println(analogRead(A0+t));
               adc_init(A4);
               }
          }
     if (!strcmp(p, "sweep"))
          {
          p = strtok (NULL, " ");
          char * pEnd;
          int t = strtol(p,&pEnd,10);
          if (t > 0)
               msecPerSweep = t;
          Serial.print("msec per Sweep ");
          Serial.println(msecPerSweep);
          printSweepTime(false);
          }
     if (!strcmp(p, "mvs"))
          {
          p = strtok (NULL, " ");
          char * pEnd;
          enableMVS = !strcmp(p, "en");
          Serial.print("MVS Enable ");
          Serial.println(enableMVS);
          }
     if ((!strcmp(p, "triglev")) | (!strcmp(p, "tl")))
          {
          p = strtok (NULL, " ");
          char * pEnd;
          int t = strtol(p,&pEnd,10);
          if (t > 0)
               triglevel = t;
          Serial.print("trigger level ");
          Serial.println(triglevel);
          printTrigLevel(false);
          }
     if ((!strcmp(p, "trigfeat")) | (!strcmp(p, "tf")))
          {
          p = strtok (NULL, " ");
          if (!strcmp(p, "free"))
               trigfeature = FREERUN;
          if (!strcmp(p, "plus"))
               trigfeature = PLUSSLOPE;
          if (!strcmp(p, "neg"))
               trigfeature = NEGSLOPE;
          Serial.print("trigger feature ");
          Serial.println(trigSpell[trigfeature]);
          printTrigFeature(false);
          }
     if ((!strcmp(p, "ACtrip")) | (!strcmp(p, "ACt")))
          {
          p = strtok (NULL, " ");
          char * pEnd;
          int t = strtol(p,&pEnd,10);
          if ((t > 0) && (t<64))
               {
               REG_AC_SCALER0 = t;    // Set threshold level
               Serial.print("ACtrip ");
               Serial.println(t);
               }
          }

     }

if (dumpMode)
     {
     dumpMode = false;

     Serial.println((long) scopeBufLoc,HEX);
     Serial.println("Showing scope data points");
     for (int j = 0; j < SCOPEWORDS/10; j++)
               {
               Serial.print(j);
               Serial.print(' ');
               for (int i= 0; i < 10; i++)
                    {
                    Serial.print(*(scopeBufLoc+j*10+i));     
                    Serial.print(" ");
                    }
               Serial.println();
               }
     Serial.println(scopeworkerstatus);
     }
if (printscopebuf)
     {
     uint16_t localscopebuff[SCOPEWORDS];
     __disable_irq();
     memcpy(localscopebuff,scopeBufLoc,sizeof(localscopebuff));
     __enable_irq();
     delay(5000);
     for (int i = 0; i < SCOPEWORDS; i++)
          Serial.println(localscopebuff[i]);
//     printscopebuf = false;
     }

if (!menuShowing)
     {
     if (dmabufftimelong != dmabufftimelongreported)
          {
          dmabufftimelongreported = dmabufftimelong;
          Serial.print("dmabufftimelong ");
          Serial.println(dmabufftimelong);
          }
     if (dmabufftimeshort != dmabufftimeshortreported)
          {
          dmabufftimeshortreported = dmabufftimeshort;
          Serial.print("dmabufftimeshort ");
          Serial.println(dmabufftimeshort);
          }
     if ((dmabufftime > 10) && (dmabufftime < 1800))
          {
          Serial.print("dmabufftime ");
          Serial.println(dmabufftime);
          }
     if (buffmakertime > buffmakertimereported)
          {
          buffmakertimereported = buffmakertime;
          Serial.print("buffmakertime ");
          Serial.println(buffmakertime);
          }
     }     

int now = millis();
if (now-SWstatustime > 10000)
     {
     SWstatustime = now;
     if (scopeworkerstatus)
          {
          Serial.print("SCOPEWORKERSTATUS ");     
          Serial.println(scopeworkerstatus,HEX);
          scopeworkerstatus = 0;
          }
     }
}

extern "C" char *sbrk(int i);

int FreeRam () {
  char stack_dummy = 0;
  return &stack_dummy - sbrk(0);
}
uint16_t readline(void)
{
return readline(buffer, INPBUFSIZE, 50);
}

uint16_t readline(char * buf, uint16_t bufsize, uint16_t timeout)
{
uint16_t replyidx = 0;
while (timeout--) 
     {
     while(Serial.available()) 
          {
          char c = Serial.read();
          //SerialDebug.println(c);
          if (c == '\r') 
               continue;
          if (c == '\n') 
               {
               // the first '\n' is ignored
//               if (replyidx == 0) 
//                    continue;
               timeout = 0;
               break;
               }
          buf[replyidx] = c;
          replyidx++;
          
          // Buffer is full
          if (replyidx >= bufsize) 
               {
               Serial.println("*overflow*");   // for my debuggin' only!
               timeout = 0;
               break;
               }
          }

     // delay if needed
     if (timeout) 
          delay(1);
     }

buf[replyidx] = 0;  // null term
return replyidx;
}


