static uint32_t chnl = 0;  // DMA channel


void DMAC_Handler() 
{
// interrupts DMAC_CHINTENCLR_TERR DMAC_CHINTENCLR_TCMPL DMAC_CHINTENCLR_SUSP
uint8_t active_channel;

// disable irqs ?
__disable_irq();
active_channel =  DMAC->INTPEND.reg & DMAC_INTPEND_ID_Msk; // get channel number
DMAC->CHID.reg = DMAC_CHID_ID(active_channel);
dmadone = DMAC->CHINTFLAG.reg;
DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_TCMPL; // clear
DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_TERR;
DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_SUSP;
__enable_irq();
if (dmadone)
     {
     uint32_t now = micros();
     dmabufftime = now-dmabufftime_s;
     dmabufftime_s = now;
     if (dmabufftime > dmabufftimelong)
          dmabufftimelong = dmabufftime;
     if (dmabufftime < dmabufftimeshort)
          dmabufftimeshort = dmabufftime;
     buffermaker();
     if (!sqFromInt)
          {
          sqWave = !sqWave;
//          digitalWrite(SQUAREWAVEPIN,sqWave);
          if (sqWave)
               PORT->Group[SqPinPort].OUTSET.reg = SqPinMask;
          else PORT->Group[SqPinPort].OUTCLR.reg = SqPinMask;
          }
     }
}


void dma_init() 
{
// probably on by default
PM->AHBMASK.reg |= PM_AHBMASK_DMAC ;
PM->APBBMASK.reg |= PM_APBBMASK_DMAC ;
NVIC_EnableIRQ( DMAC_IRQn ) ;

DMAC->BASEADDR.reg = (uint32_t)descriptor_section;
DMAC->WRBADDR.reg = (uint32_t)wrb;
DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE | DMAC_CTRL_LVLEN(0xf);
}

void adc_dma(void *rxdata,  void *rxdata1,  size_t hwords) 
{
uint32_t temp_CHCTRLB_reg;

DMAC->CHID.reg = DMAC_CHID_ID(chnl);
DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
DMAC->CHCTRLA.reg = DMAC_CHCTRLA_SWRST;
DMAC->SWTRIGCTRL.reg &= (uint32_t)(~(1 << chnl));
temp_CHCTRLB_reg = DMAC_CHCTRLB_LVL(0) 
          | DMAC_CHCTRLB_TRIGSRC(ADC_DMAC_ID_RESRDY) 
          | DMAC_CHCTRLB_TRIGACT_BEAT;
DMAC->CHCTRLB.reg = temp_CHCTRLB_reg;
DMAC->CHINTENSET.reg = DMAC_CHINTENSET_MASK ; // enable all 3 interrupts
dmadone = 0;
descriptor.descaddr = (uint32_t) &descriptor;
//    descriptor.descaddr = 0;
descriptor.srcaddr = (uint32_t) &ADC->RESULT.reg;
descriptor.btcnt =  hwords;
descriptor.dstaddr = (uint32_t)rxdata + hwords*2;   // end address
descriptor.btctrl =  DMAC_BTCTRL_BEATSIZE_HWORD 
                    | DMAC_BTCTRL_DSTINC 
                    | DMAC_BTCTRL_BLOCKACT_INT
                    | DMAC_BTCTRL_VALID;
memcpy(&descriptor_section[chnl],&descriptor, sizeof(dmacdescriptor));
descriptor.descaddr = (uint32_t) &descriptor_section[0];
//    descriptor.descaddr = 0;
descriptor.srcaddr = (uint32_t) &ADC->RESULT.reg;
descriptor.btcnt =  hwords;
descriptor.dstaddr = (uint32_t)rxdata1 + hwords*2;   // end address
descriptor.btctrl =  DMAC_BTCTRL_BEATSIZE_HWORD 
                    | DMAC_BTCTRL_DSTINC 
                    | DMAC_BTCTRL_BLOCKACT_INT
                    | DMAC_BTCTRL_VALID;
//    memcpy(&descriptor_section[1],&descriptor, sizeof(dmacdescriptor));

// start channel
DMAC->CHID.reg = DMAC_CHID_ID(chnl);
DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;

dmabufftime_s = micros();
}

static __inline__ void ADCsync() __attribute__((always_inline, unused));
static void   ADCsync() 
{
while (ADC->STATUS.bit.SYNCBUSY == 1); //Just wait till the ADC is free
}

uint32_t gain;
uint32_t refsel;
uint32_t avgctrl;
uint32_t sampctrl;
uint32_t ctrlb;

void adc_init(int ADCPIN)
{
inputADC = ADCPIN;
gain = ADC->INPUTCTRL.bit.GAIN;
ADCsync();
refsel = ADC->REFCTRL.bit.REFSEL;
avgctrl = ADC->AVGCTRL.reg;
sampctrl = ADC->SAMPCTRL.reg;
ctrlb = ADC->CTRLB.reg;
analogRead(ADCPIN);  // do some pin init  pinPeripheral()

ADC->CTRLA.bit.ENABLE = 0x00;             // Disable ADC
ADCsync();    //  ref 32.6.13
//ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_2X_Val;      // Gain select as 1X
//ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_DIV2_Val; 
ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_1X_Val;      // Gain select as 1X
ADCsync();
ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC1_Val;
//ADCsync();
ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[ADCPIN].ulADCChannelNumber;
ADCsync();
ADC->AVGCTRL.reg = 0x00 ;       //no averaging
ADC->SAMPCTRL.reg = 0x00;  ; //sample length in 1/2 CLK_ADC cycles
//ADCsync();
ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV16 
                    | ADC_CTRLB_FREERUN 
                    | ADC_CTRLB_RESSEL_10BIT;
ADCsync();
ADC->CTRLA.bit.ENABLE = 0x01;
ADCsync();
}

void adc_uninit()
{
ADC->CTRLA.bit.ENABLE = 0;             // Disable ADC
ADCsync();
ADC->INPUTCTRL.bit.GAIN = gain;
ADCsync();
ADC->REFCTRL.bit.REFSEL =  refsel;
//ADCsync();
ADC->AVGCTRL.reg =  avgctrl;
//ADCsync();
ADC->SAMPCTRL.reg =  sampctrl;
//ADCsync();
ADC->CTRLB.reg =  ctrlb;
ADCsync();
ADC->CTRLA.bit.ENABLE = 1;             // Disable ADC
ADCsync();
}


