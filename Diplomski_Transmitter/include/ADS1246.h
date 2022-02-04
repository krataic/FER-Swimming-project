#define WAKEUP      0x00
#define SLEEP       0x02
#define SYNC        0x04
#define RESET       0x06
#define NOOP        0xff
#define RDATA       0x12
#define RDATAC      0x14
#define SDATAC      0x16
#define SYSOCAL     0x60 
#define SYSGCAL     0x61
#define SELFOCAL    0x62

static const uint8_t _drdy = 25;
static const uint8_t start = 26;
static const uint8_t _rst = 27;
static const int8_t mosi = 23;
static const int8_t miso = 19;
static const int8_t sck = 18;
static const int8_t _cs = 5;
static const uint8_t analogSwitch = 16;
static const uint8_t greenLED = 4;
static const uint8_t blueLED = 0;
static const uint8_t redLED = 2;
static const uint8_t vBattery = 14;



void pinDeclare (void);
double checkBatteryCharge(void);
void hardReset(void);
void softReset(void);
void drdyInterrupt(void);
void vADSInit(void);
void vADSConfig(void);
void vADSConfig10(void);
void vADSCheckRegisters(void);
float readADS1246(void);
float read10ADS1246(void);