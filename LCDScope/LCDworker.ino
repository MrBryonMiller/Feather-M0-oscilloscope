uint16_t erasebuff[SCOPEWORDS];
#define ZERO_HELPER 26

void LCDsetup() 
{
tft.begin(HX8357D);
tft.setTextWrap(false);
splash();
// now show the Scope - don't have to erase 
// because splash() left it clear
showScope(false);
}

void showScope(bool erase)
{tft.setRotation(1);
if (erase)
     tft.fillScreen(BLACK);
tft.setTextColor(WHITE);  
tft.setTextSize(2);
printStatusLine();

if (inputADC == A1)  // 0-5 volts
     {
     float yaxis=5.0;
     for (int i=1;i<=6;i++)
          {
          int y=topy+(i-1)*56;
          tft.setCursor(0, y-7);
          tft.print(yaxis,1);
          yaxis-=1.0;
          tft.drawLine(42, y, tft.width()-1, y, YELLOW);
          if (i < 6)
               for (int j=1;j<5;j++)
                    tft.drawLine(42, y+j*11, leftx, y+j*11, YELLOW);
          }
     }
else {    // +- 15 volts
     int yaxis=15;
     for (int i=1;i<=7;i++)
          {
//          int y=topy+(i-1)*4666/100;
          int y=topy+(i-1)*46;
          int x;
          if ((yaxis > 5) || (yaxis == -5))
               x = 6*3;
          else if (yaxis == 5)
                    x = 2*6*3;
               else x = 0;
          tft.setCursor(x, y-7);
          if (yaxis)
               tft.print(yaxis);
          else tft.print("0.0");
          yaxis-=5;
          tft.drawLine(42, y, tft.width()-1, y, YELLOW);
          if (i < 7)
               for (int j=1;j<5;j++)
                    tft.drawLine(42, y+j*9, leftx, y+j*9, YELLOW);
          }
     }
}

int atime,btime;
bool fastsweep;
int wherediget;
int refreshDelay,lastDrawTime;

void LCDworker(void)
{
if (msecPerSweep >= 200)
     {
     if (!fastsweep)
          restore_grid();
     fastsweep = true;
     if (scopeBufNext < wherediget)  
          {
          wherediget = 0; 
          restore_grid();
          }
     else {for (;wherediget<scopeBufNext;wherediget++)
               {
               uint16_t y;
               if (inputADC == A1)
                    y = map(*(scopeBufLoc+wherediget),56,976,297,topy);
               else y = 5+map(*(scopeBufLoc+wherediget),56,976,297,topy);;
               y = constrain(y,topy,297);
               tft.drawPixel(leftx+wherediget,y,WHITE);
               erasebuff[wherediget] = y;
               }
          if (trigbufffull && (wherediget >= SCOPEWORDS))
               {
               trigbufffull = false;
               }
          }
     }
else {
     if (fastsweep)
          restore_grid();
//          oldrestore_grid();
     fastsweep = false;
     if (trigbufffull && ((millis()-refreshDelay) > 300))
          {
          int startDrawTime = millis();
          uint16_t localscopebuff[SCOPEWORDS];
          __disable_irq();
          scopeunload = true;
          memcpy(localscopebuff,scopeBufLoc,sizeof(localscopebuff));
          trigbufffull = false;
          scopeunload = false;
          __enable_irq();
          restore_grid();
          for (int x = 0; x < SCOPEWORDS; x++)
               {
               uint16_t y;
               if (inputADC == A1)
                    y = map(localscopebuff[x],56,976,297,topy);
               else y = 5+map(localscopebuff[x],56,976,297,topy);;
               y = constrain(y,topy,297);
			drawPixLine(x,y,WHITE,x);
//               tft.drawPixel(leftx+x,y,WHITE);
//               erasebuff[x] = y;
               }
          refreshDelay = millis();
          lastDrawTime = refreshDelay-startDrawTime;
          }
     }
}

int y_s;

void drawPixLine(int mx,int my,int color,bool notfirst)
{
erasebuff[mx] = my;
if (notfirst)
     {
//     int halfy = y_s+(my-y_s)/2;
//     tft.drawLine(leftx+mx-1,y_s,leftx+mx-1,halfy,color);
//     tft.drawLine(leftx+mx,halfy,leftx+mx,my,color);
     tft.drawLine(leftx+mx,y_s,leftx+mx,my,color);
     }
else tft.drawPixel(leftx+mx,my,color);
y_s = my;
}

void restore_grid()
{
for (int x = 0; x<SCOPEWORDS; x++)
     {
     int y = erasebuff[x];
     int color;
     int mody= (inputADC == A1)?56:46;
     if ((x % 43) == 0)
          color = YELLOW;
     else color = BLACK; //   int y=topy+(i-1)*4666/100;
	int yl=min(y,y_s);
	int yh=max(y,y_s);
	drawPixLine(x,y,color,x && (y != y_s));
	for (int y1=topy;y1<=yh;y1=y1+mody)
		if (y1>=yl)
			tft.drawPixel(leftx+x,y1,YELLOW);  

//     tft.drawPixel(leftx+x,y,color);
     }
}

void oldrestore_grid()
{
for (int x = 0; x<SCOPEWORDS; x++)
     {
     int y = erasebuff[x];
     int color;
     int mody= (inputADC == A1)?56:46;
//     if (inputADC == A1)
//          if (((x % 43) == 0) || (((y-topy) % 56) == 0))
//               color = YELLOW;
//          else color = BLACK; //   int y=topy+(i-1)*4666/100;
//     else if (((x % 43) == 0) || (((y-topy) % 46) == 0))
//               color = YELLOW;
//          else color = BLACK;
     if (((x % 43) == 0) || (((y-topy) % mody) == 0))
          color = YELLOW;
     else color = BLACK;
     tft.drawPixel(leftx+x,y,color);
     }
}

void printSweepTime(bool firstTime)
{
// prints the sweep time on top of the screen
// also, if firstTime, it draws the vert grid lines
//        and the sweep time per vert line
if (firstTime)
     {
     tft.setCursor(50, 0);
     tft.print("Sweep(msec) ");
     float faxis = 0;
     int iaxis = 0;
     for (int i=1;i<=11;i++)
          {
          int x=leftx+(i-1)*43;
          tft.drawLine(x, topy, x, boty+5, YELLOW);
          tft.setCursor(x-15, boty+5);
          if (i & 1)
               {
               if (msecPerSweep <= 5)
                    {
                    tft.print(faxis,1);
                    faxis+= float (msecPerSweep)/5;
                    }
               else {
                    tft.print(iaxis,1);
                    iaxis+= msecPerSweep/5;
                    }
               }
          for (int j=1;j<5;j++)
               tft.drawLine(x+(j*43)/5, boty, x+(j*43)/5, boty+5, YELLOW);
          }
     }
else {
     // skip over first 12; clear 4 characters
     tft.fillRect(50+12*12, 0,4*12,15,BLACK);
     }
tft.setCursor(50+12*12, 0);
tft.println(msecPerSweep);
}

void printTrigFeature(bool firstTime)
{
if (firstTime)
     {
     tft.setCursor(50+17*12, 0);
     tft.print("Trig ");
     }
else {
     // skip over first 12; clear 4 characters
     tft.fillRect(50+17*12+5*12, 0,4*12,15,BLACK);
     tft.setCursor(50+17*12+5*12, 0);
     }
tft.println(trigSpell[trigfeature]);
}

void printTrigLevel(bool firstTime)
{
if (firstTime)
     {
     tft.setCursor(50+27*12, 0);
     tft.print("Lev ");
     }
else {
     // skip over first 12; clear 4 characters
     tft.fillRect(50+27*12+4*12, 0,4*12,15,BLACK);
     tft.setCursor(50+27*12+4*12, 0);
     }
tft.println(disTrigLevel);  

}

void printStatusLine()
{
printSweepTime(true);
printTrigFeature(true);
printTrigLevel(true);
}

void splash()
{
tft.setRotation(1);
tft.setTextSize(3);
int x,y,i,w;
w=480-320;
for (i=0;i<=160;i++)
     {
     x=(480-w)/2;
     y=160-i;
     tft.drawLine(x,y,x+w,y,BLACK);
     y=160+i;
     tft.drawLine(x,y,x+w,y,BLACK);
     tft.drawLine(x-1,160-i,x-1,160+i,BLACK);
     tft.drawLine(x+w+1,160-i,x+w+1,160+i,BLACK);
     w++;
     tft.drawLine(x-1,160-i,x-1,160+i,BLACK);
     tft.drawLine(x+w+1,160-i,x+w+1,160+i,BLACK);
     w++;
     }
//for (i=0;i<0;i++)
//     {
//     x=79-i;
//     tft.drawLine(x,0,x,319,BLACK);
//     x=240+160+i;
//     tft.drawLine(x,0,x,319,BLACK);
//     }
x=240-(23*6*3)/2;
tft.setCursor(x,97);
tft.print("Feather M0 oscilloscope");
x=240-(14*6*3)/2;
tft.setCursor(x,140);
tft.print("by BryonMiller");
delay(1000);
x=240-(23*6*3)/2;
tft.fillRect(x,97,23*6*3,8*3,BLACK);
delay(250);
x=240-(14*6*3)/2;
tft.fillRect(x,140,14*6*3,8*3,BLACK);
}

