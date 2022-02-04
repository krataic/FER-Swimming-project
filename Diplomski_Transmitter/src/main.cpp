#include <Arduino.h>
#include <FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <ADS1246.h>
#include <SPI.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#define Threshold 40

volatile bool batteryReadRequest{false};
uint8_t myData{};
volatile bool startStream{true};
//uint8_t receiverAddress[] = {0x24,0x0A,0xC4,0xF9,0x28,0xDC}; // moj 2. esp32
uint8_t receiverAddress[] = {0x24,0x0A,0xC4,0xEF,0x7D,0x28}; // moj 1. esp32
float result{};

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.println(myData);
  if (myData == 5) {
    startStream = HIGH;
  }
  if (myData == 10) {
    startStream = LOW;
    esp_now_deinit();
    
    esp_sleep_enable_touchpad_wakeup();
    esp_deep_sleep_start();
  }
  if (myData == 15) {
    batteryReadRequest = 1;
  }
}

void initESP(void) {
  pinDeclare();
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_wifi_set_max_tx_power(78) != ESP_OK) {
    Serial.println("Error setting max tx power");
    return;
  }
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    //return;
  }
  SPI.begin(sck, miso, mosi, _cs);
  SPI.beginTransaction(SPISettings(1200000,MSBFIRST,SPI_MODE1));
  attachInterrupt(_drdy, drdyInterrupt, FALLING);
  
  // in here comes the delay for tcssc
  vADSInit();
  //vADSConfig();
  vADSConfig10();
  vADSCheckRegisters();
  delayMicroseconds(1); // delay for tsccs
  SPI.transfer(SDATAC);
  //SPI.transfer(RDATAC);
  SPI.transfer(SYNC);
  digitalWrite(greenLED,LOW);
  digitalWrite(redLED,HIGH);
  digitalWrite(blueLED,HIGH);
  digitalWrite(analogSwitch, LOW);
}

void callback(){
  uint8_t poruka{120};
  initESP();
  Serial.println("Testpoint1");
  esp_now_send(receiverAddress, (uint8_t*)&poruka, sizeof(poruka));
}

void setup() {
  initESP();
  touchAttachInterrupt(T3, callback, Threshold); // GPIO15 is touch sensitive pin
}

void loop() {
  if (batteryReadRequest) {
    result = (float)checkBatteryCharge();
    esp_err_t state = esp_now_send(receiverAddress, (uint8_t*)&result, sizeof(result));
    if (state == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
    Serial.println(result);
    batteryReadRequest = 0;
  }
  if (startStream) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW);
    digitalWrite(blueLED, LOW);
    result = read10ADS1246();
    esp_err_t state = esp_now_send(receiverAddress, (uint8_t*)&result, sizeof(result));
    if (state == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
    Serial.println(result);
  }
  if (!startStream){
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, HIGH);
    digitalWrite(blueLED, LOW);
    result = 0;
    /*esp_err_t state = esp_now_send(receiverAddress, (uint8_t*)&result, sizeof(result));
    if (state == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }*/
  }
  
}
