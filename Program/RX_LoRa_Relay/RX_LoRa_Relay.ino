#include <SPI.h>              // include libraries
#include <LoRa.h>
const byte led = 2;
const long frequency = 923E6;  // LoRa Frequency

const int csPin = 21;          // LoRa radio chip select
const int resetPin = 22;        // LoRa radio reset
const int irqPin = 17; 
const int relay = 25;
String getAlarm, oldAlarm;

String data;
String id, idPenerima;
String keyIdPenerima = "C402FDBF713C"; // ganti id pengirim
int len_id, len_data;
unsigned long LcurTime, LoldTime;
unsigned long RcurTime, RoldTime;

bool ledState;
int count;

void setup() {
  Serial.begin(115200);                   // initialize serial
  while (!Serial);
  pinMode(relay, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(resetPin, INPUT_PULLUP);
  
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  Serial.println("LoRa init succeeded.");
  LoRa.onReceive(onReceive);
  LoRa_rxMode();
}

void loop() {
  LcurTime = millis();
  if(LcurTime - LoldTime > 1000){
    led_indikator();
    digitalWrite(led, ledState);
    if(getAlarm == oldAlarm){
      count++;
      if(count > 15 ){
        ESP.restart();
      }
      Serial.print("check data: ");
      Serial.println(count);
    }
    LoldTime = LcurTime;
  }

  if(getAlarm != oldAlarm){
    count = 0;
    LoRa_txMode();
    len_id = getAlarm.indexOf(',');
    len_data = getAlarm.indexOf('#');
    idPenerima = getAlarm.substring(0,len_id);
    data = getAlarm.substring(len_id+1,len_data);
    
    Serial.print("id: ");
    Serial.println(idPenerima);
    Serial.print("data: ");
    Serial.println(data);

    Serial.print("kirim id");
    LoRa_sendMessage(idPenerima);
    Serial.println();

    if(idPenerima == keyIdPenerima){
      if(data == "alarm"){
        digitalWrite(relay, HIGH);
      }
      if(data == "off"){
        digitalWrite(relay, LOW);
      }
    }
    
    getAlarm = "";
    oldAlarm = getAlarm;
    delay(1000);
    LoRa_rxMode();
   }
}

void LoRa_rxMode(){
  LoRa.disableInvertIQ();               // normal mode
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.enableInvertIQ();                // active invert I and Q signals
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
  getAlarm = message;
  Serial.print("Gateway Receive: ");
  Serial.println(message);
}

void led_indikator(){
  if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
}
