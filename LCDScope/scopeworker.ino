int usNext;
uint16_t point_s;
int usperscopept;
int trigCnt;
bool pretrig;
bool trigbufffull_s=true;

void buffermaker()
{
if (scopeunload)
     scopeworkerstatus |= 0x02;
uint32_t now = micros();
if (menuShowing || (!trigbufffull && trigbufffull_s))
     {
     scopeBufNext = 0;
     usNext = 0;   
     pretrig = false;
     triggered = false;   
     trigbufffull = false;
     }
// make sure we know which buffer was filled
if (*(buffloc+HWORDS-1) != 0x7fff) //(dmadone)
     {
     dmabufftime = 2000;
     usperscopept = msecPerSweep*1000/SCOPEWORDS;

     // set up buffloc for next time
     uint16_t *tmpbuffloc;
     if (buffloc == buffloc1)
          tmpbuffloc = buffloc2;
     else tmpbuffloc = buffloc1;
     // and mark nextbuff as "not done"
     *(tmpbuffloc+HWORDS-1) = 0x7fff;  

	while (usNext < dmabufftime)
		{
		uint16_t point;
		int get = usNext >> 1;
		if (get < HWORDS)
			{
			bool useMVS= enableMVS 
			          && (get > 0) 
					&& (get < HWORDS-1)
					&& (msecPerSweep > 10);
			if (useMVS)
				point = MVS(*(buffloc+get-1),*(buffloc+get),*(buffloc+get+1));
			else point = *(buffloc+get);
			}
		else point = point_s;
		usNext += usperscopept;
		if (!trigbufffull)
			{
			if (triggered)
				{
				*(scopeBufLoc+scopeBufNext) = point;
				scopeBufNext++;
				trigbufffull = (scopeBufNext >= SCOPEWORDS);
				if (isLEDOn && (scopeBufNext >= SCOPEWORDS/2))
					{
					isLEDOn = false;
                         PORT->Group[LEDBuiltinPort].OUTCLR.reg = LEDBuiltinMask;
					}
				}
			else {
                    int mytriglevel = triglevel-25;
				switch (trigfeature)
                         {
                         case PLUSSLOPE :
                              if (point < mytriglevel)
                                   pretrig = true;
                              else triggered = pretrig && ((point-mytriglevel) > 50);
                              break;
                         case NEGSLOPE :
                              if (point > mytriglevel)
                                   pretrig = true;
                              else triggered = pretrig && ((mytriglevel-point) > 50);
                              break;
                         default : triggered = true;
			          }                                                     
			     if (triggered) 
					{
                         *scopeBufLoc = point;  // collect first point
                         scopeBufNext = 1;
					isLEDOn = true;
                         PORT->Group[LEDBuiltinPort].OUTSET.reg = LEDBuiltinMask;
					}
				}
			}
		point_s = point;
		}
	usNext -= dmabufftime;

     buffloc = tmpbuffloc;
     buffmakertime = micros()-now;
     trigbufffull_s = trigbufffull;
     }
else scopeworkerstatus |= 0x01;
}

void scope_init()
{
scopeBufLoc = &scopebuf[0];
SqPinPort = g_APinDescription[SQUAREWAVEPIN].ulPort;
int pin = g_APinDescription[SQUAREWAVEPIN].ulPin;
SqPinMask = (1ul << pin);
LEDBuiltinPort = g_APinDescription[LED_BUILTIN].ulPort;
pin = g_APinDescription[LED_BUILTIN].ulPin;
LEDBuiltinMask = (1ul << pin);
}


int MVS(int n1, int n2, int n3)
{
int mid;
if(n1 > n2 )   
     {
     if(n3 > n2)
          {
          if(n3 < n1 )
               mid = n3;
          else mid = n1;
          }
     else mid = n2;
     }
else {
     if(n2 > n3 )
          {
          if (n1 > n3)
               mid = n1;
          else mid = n3;
          }
     else mid = n2;
     }
return mid ;
}


