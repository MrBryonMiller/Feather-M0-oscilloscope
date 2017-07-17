#define SCOPEWORDS 430
uint16_t scopebuf[SCOPEWORDS];
uint16_t *scopeBufLoc;
int scopeBufNext;

uint16_t *buffloc,*buffloc1,*buffloc2;
bool enableMVS;
EPortType SqPinPort,LEDBuiltinPort;
uint32_t SqPinMask,LEDBuiltinMask;

volatile  uint32_t buffmakertime,buffmakertimereported;

volatile bool triggered,trigbufffull;
volatile uint16_t scopeworkerstatus;
volatile bool scopeunload;
int msecPerSweep=500;
int triglevel=500;

float fltTrigLevel=2.5;
String disTrigLevel="----";

typedef enum {FREERUN, PLUSSLOPE, NEGSLOPE} trigtype;
trigtype trigfeature = FREERUN;



