//from : https://github.com/manitou48/ZERO/blob/master/adcdma.ino
// adcdma
//  analog A1
//   could use DAC to provide input voltage   A0
//   http://www.atmel.com/Images/Atmel-42258-ASF-Manual-SAM-D21_AP-Note_AT07627.pdf pg 73

#define HWORDS 1000
uint16_t adcbuf1[HWORDS];     
uint16_t adcbuf2[HWORDS];     

typedef struct {
    uint16_t btctrl;
    uint16_t btcnt;
    uint32_t srcaddr;
    uint32_t dstaddr;
    uint32_t descaddr;
} dmacdescriptor ;
volatile dmacdescriptor wrb[12] __attribute__ ((aligned (16)));
dmacdescriptor descriptor_section[12] __attribute__ ((aligned (16)));
dmacdescriptor descriptor __attribute__ ((aligned (16)));
//DmacDescriptor descriptorx __attribute__ ((aligned (16)));

volatile uint32_t dmadone;
volatile uint32_t dmabufftime,dmabufftime_s;
volatile uint32_t dmabufftimelong,dmabufftimelongreported;
volatile uint32_t dmabufftimeshort = 5000;
volatile uint32_t dmabufftimeshortreported = 5000;


int LEDOnCnt;
int inputADC=A1;


