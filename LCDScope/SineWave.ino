#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 64

float S1 = 20.0;// 9.5;
float S2;
volatile bool sqWave;
//int togtime;

void SineWave() 
{
float I1 = -S2*sineTime/1000.0;//-S1*0.001;
S1 = S1+I1;
float I2 = S1*sineTime/1000.0;
S2 = S2+I2;
int Analog = int (S1*240)/10;
Analog += 500;
if (Analog < 5)
	Analog = 5;
if (Analog > 1023)
	Analog = 1023;
if (printSineMode)
	Serial.println(Analog);
analogWrite(A0,Analog);
}

void TC3_Handler() 
{
TcCount16* TC = (TcCount16*) TC3;
// If this interrupt is due to the compare register matching the timer count
// we toggle the LED.
if (TC->INTFLAG.bit.MC0 == 1) 
     {
     TC->INTFLAG.bit.MC0 = 1;
     if (enableSine)
          SineWave();
     if (sqFromInt)
          {
          sqWave = !sqWave;
//          digitalWrite(SQUAREWAVEPIN,sqWave);
          if (sqWave)
               PORT->Group[SqPinPort].OUTSET.reg = SqPinMask;
          else PORT->Group[SqPinPort].OUTCLR.reg = SqPinMask;
          }
     }
}

void setTimerFrequency(int frequencyHz) 
{
int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
//Serial.println(compareValue);
TcCount16* TC = (TcCount16*) TC3;
// Make sure the count is in a proportional position to where it was
// to prevent any jitter or disconnect when changing the compare value.
TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
TC->CC[0].reg = compareValue;
//Serial.print("COUNT.reg ");
//Serial.println(TC->COUNT.reg);
//Serial.print("CC[0].reg ");
//Serial.println(TC->CC[0].reg);
while (TC->STATUS.bit.SYNCBUSY == 1);
}

void startTimer(int frequencyHz) 
{
REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3) ;
while ( GCLK->STATUS.bit.SYNCBUSY == 1 ); // wait for sync

TcCount16* TC = (TcCount16*) TC3;

TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

// Use the 16-bit timer
TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

// Use match mode so that the timer counter resets when the count matches the compare register
TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

// Set prescaler to 1024
//TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024;
TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV64;
while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

setTimerFrequency(frequencyHz);

// Enable the compare interrupt
TC->INTENSET.reg = 0;
TC->INTENSET.bit.MC0 = 1;

NVIC_EnableIRQ(TC3_IRQn);

TC->CTRLA.reg |= TC_CTRLA_ENABLE;
while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
}

