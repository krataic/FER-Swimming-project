#include <stdio.h>
#include "WiFi.h"
#include <esp_now.h>
#include <Arduino.h>

float myData{};
float myData2{};
float myData3{};
float myData4{};
uint8_t incomingData{};
float scaledData{};
uint32_t i{};
uint32_t offset{};
// E8:68:E7:1A:B4:F8
// E8:68:E7:1A:B4:F8


//uint8_t receiverAddress[] = {0xE8,0x68,0xE7,0x1A,0xB4,0xF8}; // adresa ESP-a u bazenu
//uint8_t receiverAddress[] = {0x24,0x0A,0xC4,0xF9,0x28,0xDC}; // moj 2. esp32
//uint8_t receiverAddress[] = {0x24,0x0A,0xC4,0xEF,0x7D,0x28}; // moj 1. esp32
uint8_t receiverAddress[] = {0xE8, 0x68, 0xE7, 0x2C, 0x5D, 0xBC}; //E8:68:E7:2C:5D:BC, wroom32u adresa

void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.println(myData);
  /*myData *= 100;
  myData2 = myData;
  myData3 = (myData2 * 5.0633); // prikaz sile u Newtonima
  i++;
  if (i < 500) {
    Serial.println(myData3);
  }
  if (i == 500) {
    offset = myData3;
  }
  if (i >= 501) {
    //myData3 -= offset;
    Serial.println(myData3);
  }*/
  /*
  if (i < 200){
    i++;
    Serial.println(0);
  }
  else if (i >= 200 && i <= 500) {
    i++;
    offset += myData3;
    if (i == 499)
      offset = offset / 300;
      Serial.println(0);
      i++;
  }
  else if (i > 500) {
    myData3 = myData3 - offset;
    Serial.println(myData3);
  }*/
  /*memcpy(&myData, incomingData, sizeof(myData));
  myData *= 100;
  myData2 = myData - 105;
  myData3 = (myData2 * 5.0633) + 100; // prikaz sile u Newtonima
  Serial.println(myData3);*/
}
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void vReceiverSetup(void){
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
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
}
 
void setup(){
  vReceiverSetup();
  //Serial.setTimeout(1);
  //delay(2000);
  //esp_now_send(receiverAddress, (uint8_t*)&incomingData, sizeof(incomingData)); // šalji podatak za početak mjerenja
}

void loop(){
  while(!Serial.available());
  incomingData = Serial.readString().toInt();
  //incomingData = Serial.read();
  if (incomingData == 5) { // pokreni snimanje
    //Serial.println("Pokrecem snimanje");
    esp_err_t state = esp_now_send(receiverAddress, (uint8_t*)&incomingData, sizeof(incomingData));
  }
  if (incomingData == 10) { // prekini stream, idi u sleep
    //Serial.println("Zaustavljam snimanje");
    i = 0;
    offset = 0;
    esp_err_t state = esp_now_send(receiverAddress, (uint8_t*)&incomingData, sizeof(incomingData));
  }
  if (incomingData == 15) {
    esp_err_t state = esp_now_send(receiverAddress, (uint8_t*)&incomingData, sizeof(incomingData));
  }
}