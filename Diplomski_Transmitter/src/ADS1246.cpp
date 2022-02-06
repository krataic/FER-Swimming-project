/*
Device modes: Power Up, Reset, Power-Down Mode, Conversion Control
              Calibration.
Powr Up: 
        All the digital circuits are held in a reset state for 2^16 system
        clocks to settle. SPI communication cannot occure until the internal
        reset is released.
Reset: 
        When /RST goes low device is immediately reset. All registers are restored
        to default values. When /RST goes high ADC comes out of reset. When
        the sys clock freq is 4,096 MHz the digital filter and the registers are held in
        a reset state for 0,6 ms. -> SPI comms can be resumed 0,6 ms after the 
        /RST goes high. When /RST goes low, the clock selection is reset to the
        internal oscillator.
Power-Down Mode: 
        Two ways to put the device into power-down mode: using SLEEP command and taking the START
        pin low.
Conversion Control:
        Pulse the START pin high to begin a conversion. The conversion completion is indicated
        by the /DRDY pin going low and with the DOUT/\DRDY pinwhen the DRDY MODE bit is in 1 in the
        IDAC0 register.When the conversion completes, the device automatically powers down. During 
        power-down, the conversion resut can be retrieved; however, START must be taken high before 
        communicating with the config registers. The device stays powerred down untill the START pin
        is returned high to begin a new conversion. When the START pin is returnd high, the decimation
        filter is held in a reset state for 32 modulator clk cycles internally to allow the analog 
        circuits to settle.
etc. etc.
Datasheet, page ~ 40

START pin must be held high to use commands to control conversions. DO NOT combine using
the START pin and using commands to control conversions.

Final Output Data = (Input - OFC[2:0]) * FSC[2:0]/4.000.000h
OFC -> offset register ; table 15
FSC -> full-scale register ; table 16
Calibration commands on the page 40

        SPI:
/CS activates SPI communication. When /CS is high, the DOUT//DRDY pin enters a high-impedance state.
MOSI -> falling edge
MISO -> rising edge
When no command is to be sent to the device when reading out data, send the NOP command on DIN.

tDTS = min 1 tclk -> SCLK must be held low for tDTS after the /DRDY low transition.
This constraint applies only when /CS is asserted and the device is in RDATAC mode.

The DOUT//DRDY pin has two modes:
                                  - data out (DOUT) mode
                                  - DOUT combined with data ready (/DRDY)
The DRDY MODE bit determines the function of this pin and can be found in the ID register.

When using the SLEEP command, /CS must be low for the duration of the power-down mode

*/

#include <Arduino.h>
#include <ADS1246.h>
#include <SPI.h>


int32_t data{};
uint8_t dataLow{};
uint8_t dataMid{};
int8_t dataHigh{};
double resultVoltage{};
volatile bool drdyState = HIGH;
uint8_t regSetup1{};
uint8_t regSetup2{};
uint8_t regSetup3{};
uint8_t regSetup4{};
uint16_t batteryCharge{};


void pinDeclare (void) {
  pinMode(_drdy, INPUT);
  pinMode(start, OUTPUT);
  pinMode(_rst, OUTPUT);
  pinMode(_cs, OUTPUT);
  pinMode(analogSwitch, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(vBattery, INPUT);
  pinMode(powerSwitch, OUTPUT);
  digitalWrite(powerSwitch, LOW);
}

double checkBatteryCharge() {
  batteryCharge = analogRead(vBattery);
  batteryCharge = (double)batteryCharge * 1.1791;
  return (double)batteryCharge;
}

void hardReset(void) {
  digitalWrite(_rst, LOW);
  delay(2); // just in case
  digitalWrite(_rst, HIGH);
  delay(16); // for analog components to settle
}

void softReset(void) {
  SPI.transfer(RESET);
  delayMicroseconds(600); // conversion starts min. 0,6 ms after RESET command
}

void drdyInterrupt() {
  drdyState = LOW;
}

void vADSInit(void) {
  hardReset();
  digitalWrite(start, HIGH); // has to be high for ADS to work
  digitalWrite(_cs, LOW); 
  softReset();
}

void vADSConfig(void) {

  /*Write the respective register configuration with the WREG command(40h,03h,01h,00h,03h and 42h);
  As an optional sanity check,read back all 
  configuration registers with the RREG command(four bytes from 20h,03h);*/
  SPI.transfer(0x40); // WREG command
  SPI.transfer(0x03); // 03 = no. of bytes written-1 = 4 bytes
  SPI.transfer(0x01); // burn-out detect current source off
  SPI.transfer(0x00); // // V-bias not enabled
  SPI.transfer(0x00); // 0x80 = ext. osc, 0x00 = int. osc
  //SPI.transfer(0x00); // 0||PGA[2:0]||DR[3:0], 0000 = 5 SPS
  //SPI.transfer(0x23); //PGA = 4, DR = 40 SPS
  //SPI.transfer(0x25); //PGA = 4, DR = 160 SPS
  SPI.transfer(0x35); //PGA = 8, DR = 160 SPS
}

void vADSConfig10 (void) {
  SPI.transfer(0x40); // WREG command
  SPI.transfer(0x03); // 03 = no. of bytes written-1 = 4 bytes
  SPI.transfer(0x01); // burn-out detect current source off
  SPI.transfer(0x00); // // V-bias not enabled
  SPI.transfer(0x00); // 0x80 = ext. osc, 0x00 = int. osc
  SPI.transfer(0x38); //PGA = 8, DR = 1 kSPS 
  //SPI.transfer(0x35); //PGA = 8, DR = 160 SPS 
}

void vADSCheckRegisters(void) {
  SPI.transfer(0x20); // RREG command, 0x00 is the first register to read from 
  SPI.transfer(0x03); // 4 bytes to read

  regSetup1 = SPI.transfer(NOOP);
  regSetup2 = SPI.transfer(NOOP);
  regSetup3 = SPI.transfer(NOOP);
  regSetup4 = SPI.transfer(NOOP);

  Serial.println(regSetup1);
  Serial.println(regSetup2);
  Serial.println(regSetup3);
  Serial.println(regSetup4);
}

float readADS1246(void) {
  while(drdyState){
    continue;
  }
  noInterrupts();
  drdyState = HIGH;
  interrupts();
  //digitalWrite(SS, LOW);
  // delay for a minimum tcssc, tcssc = 10 ns
  SPI.transfer(RDATA);
  dataHigh = SPI.transfer(NOOP);
  dataMid = SPI.transfer(NOOP);
  dataLow = SPI.transfer(NOOP); // 24 sclks sent
  data = dataHigh << 16 | dataMid << 8 | dataLow;
  delayMicroseconds(2); // delay for min tsccs, tsccs = 7tclk = 1,71 us
  resultVoltage = (5*(double)data)/16777215;
  Serial.print(data);
  Serial.print(" : ");
  Serial.println(resultVoltage);
  return resultVoltage;

  /* napravit preinaku da mi vraća double vrijednost!!!!*/
}

float read10ADS1246(void) {
  float a{};
  for (int i=0;i<10;i++){
    while(drdyState){
      continue;
    }
    noInterrupts();
    drdyState = HIGH;
    interrupts();
    //digitalWrite(SS, LOW);
    // delay for a minimum tcssc, tcssc = 10 ns
    SPI.transfer(RDATA);
    dataHigh = SPI.transfer(NOOP);
    dataMid = SPI.transfer(NOOP);
    dataLow = SPI.transfer(NOOP); // 24 sclks sent
    data = dataHigh << 16 | dataMid << 8 | dataLow;
    delayMicroseconds(2); // delay for min tsccs, tsccs = 7tclk = 1,71 us
    resultVoltage = (5*(double)data)/16777215;
    a += resultVoltage;
  }
  return (a/10);
}