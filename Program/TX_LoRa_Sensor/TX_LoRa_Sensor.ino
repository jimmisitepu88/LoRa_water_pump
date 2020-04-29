#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10        /* Time ESP32 will go to sleep (in seconds) */

const byte led = 2;
const byte pinSonar = 14;

#include <SPI.h>              // include libraries
#include <LoRa.h>

const long frequency = 923E6;  // LoRa Frequency

const int csPin = 21;          // LoRa radio chip select
const int resetPin = 22;        // LoRa radio reset
const int irqPin = 17;          // change for your board; must be a hardware interrupt pin

unsigned long curTime, oldTime;
unsigned long RcurTime, RoldTime;
unsigned long LcurTime, LoldTime;

String bufMessage;
String strSensorOn, strSensorOff;

String id, buf_id;char ssid[13];
String keyIdChip = "C402FDBF713C";
uint64_t chipid;
bool ledState;
bool hold;

void setup() {  
  Serial.begin(115200);                   // initialize serial
  while (!Serial);
  pinMode(pinSonar, INPUT_PULLUP);
  pinMode(led, OUTPUT);
  
  while(!check_id(keyIdChip)){
    Serial.println("id salah");
    delay(2000);
  }

  strSensorOn = id + "," + "alarm#"; // format kirim data
  strSensorOff = id + "," + "off#"; 
  
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed");
    while (true);                       // if failed, do nothing
  }
  Serial.println("LoRa Success");
  LoRa.onReceive(onReceive);
  LoRa_txMode();
  
  send_LoRa();
  
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Going to sleep now");
  Serial.flush(); 
  esp_deep_sleep_start(); 
}
void loop() {
  
}

void LoRa_rxMode(){
  LoRa.enableInvertIQ();                // active invert I and Q signals
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.disableInvertIQ();               // normal mode
}

void LoRa_sendMessage(String message) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.print(message);                  // add payload
  LoRa.endPacket();                     // finish packet and send it
  LoRa_rxMode();                        // set rx mode
}

void onReceive(int packetSize) {
  String message = "";

  while (LoRa.available()) {
    message += (char)LoRa.read();
  }
  bufMessage = message;
  Serial.print("Node Receive: ");
  Serial.println(message);
}

bool tunggu(unsigned long waktu){
  bool sts;
  curTime = millis();
  oldTime = curTime;
  while ( curTime - oldTime < waktu){
    curTime = millis();
    if(bufMessage == id){
      sts = 1;
      Serial.println("break");
      break;
    }else{
      sts = 0;
    }
  }
  return sts;
}

bool check_id(String id_chip){
  bool checked;
  chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  uint16_t chip = (uint16_t)(chipid >> 32);
  snprintf(ssid, 13, "%04X%08X", chip, (uint32_t)chipid);
  for ( int i=0; i < 12; i++){
    buf_id += String(ssid[i]);
  }
  id = buf_id; buf_id = "";
  Serial.println(id);
  if(id == id_chip){
    checked = 1;
  }else{
    checked = 0;
  }
  return checked;
}

void send_LoRa(){
    if(digitalRead(pinSonar)== 0){
        Serial.println(F("kirim on"));
        LoRa_sendMessage(strSensorOn);
        LoRa_rxMode();
        while(!tunggu(2000)){
          Serial.println("send");
          LoRa_sendMessage(strSensorOn);
        }
        Serial.println("terkirim");bufMessage = "";
        Serial.print("' with RSSI ");
        Serial.println(LoRa.packetRssi());
        LoRa_txMode();      
    }else{
        Serial.println(F("kirim off"));
        LoRa_sendMessage(strSensorOff);
        LoRa_rxMode();
        while(!tunggu(2000)){
          Serial.println("send");
          LoRa_sendMessage(strSensorOff);
      } 
        Serial.println("terkirim"); bufMessage = "";
        Serial.print("' with RSSI ");
        Serial.println(LoRa.packetRssi());
        LoRa_txMode();      
    }
  }
