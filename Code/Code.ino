#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <U8x8lib.h> // Library for the OLED module
#include <SoftwareSerial.h> // Library for SoftwareSerial

// Include the TTN security keys for OTAA defined in ttn_keys.h file
#include "config.h"

// Disable PING and Beacon as is not required for Class A LoRaWAN operation
#define DISABLE_PING 1
#define DISABLE_BEACONS 1

// Function prototype declarations
void clearOLEDLine(int line);
void setLoraMessageOnOLED(char* message);

// Object for the OLED
U8X8_SSD1306_128X64_NONAME_SW_I2C display_(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

// Object for SoftwareSerial
SoftwareSerial pzemSerialObj;

uint8_t pzem_response_buffer[8]; // Reponse buffer for PZEM serial communication
static uint8_t loraDataPackets[9];
static osjob_t sendjob;
int lastDisplayUpdateTime = 0;
int lastLoraDataPublishTime = 0;
float voltage = 0.00;
float current = 0.00;
float power = 0.00;
float energy = 0.00;
//const unsigned TX_INTERVAL = 60;

// Pin defination for the LoRa module connection to the ESP32. May change based on the board you are using.
const lmic_pinmap lmic_pins = {
  .nss = 18, // CS PIN
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 14, // reset pin
  .dio = {
    26,
    33,
    32
  }
};

// Commands for PZEM004T V2 Module
uint8_t current_[7] = {0xB1,0xC0,0xA8,0x01,0x01,0x00,0x1B};
uint8_t address_[7] = {0xB4,0xC0,0xA8,0x01,0x01,0x00,0x1E};
uint8_t energy_[7]  = {0xB3,0xC0,0xA8,0x01,0x01,0x00,0x1D};
uint8_t voltage_[7] = {0xB0,0xC0,0xA8,0x01,0x01,0x00,0x1A};
uint8_t power_[7] =   {0xB2,0xC0,0xA8,0x01,0x01,0x00,0x1C};


void printHex2(unsigned v) {
  v &= 0xff;
  if (v < 16)
    Serial.print('0');
  Serial.print(v, HEX);
}

void onEvent(ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  switch (ev) {
  case EV_JOINING:
    Serial.println(F("EV_JOINING"));
    break;

  case EV_JOINED:
    Serial.println(F("EV_JOINED")); {
      u4_t netid = 0;
      devaddr_t devaddr = 0;
      u1_t nwkKey[16];
      u1_t artKey[16];
      LMIC_getSessionKeys( & netid, & devaddr, nwkKey, artKey);
      Serial.print("netid: ");
      Serial.println(netid, DEC);
      Serial.print("devaddr: ");
      Serial.println(devaddr, HEX);
      Serial.print("AppSKey: ");
      for (size_t i = 0; i < sizeof(artKey); ++i) {
        if (i != 0)
          Serial.print(" ");
        printHex2(artKey[i]);
      }
      Serial.println("");
      Serial.print("NwkSKey: ");
      for (size_t i = 0; i < sizeof(nwkKey); ++i) {
        if (i != 0)
          Serial.print(" ");
        printHex2(nwkKey[i]);
      }
      Serial.println();
      setLoraMessageOnOLED("Network Joined");
    }
    // Disable link check validation (automatically enabled
    // during join, but because slow data rates change max TX
    // size, we don't use it in this example.
    LMIC_setLinkCheckMode(0);
    break;
  case EV_JOIN_FAILED:
    Serial.println(F("EV_JOIN_FAILED"));
    break;
  case EV_REJOIN_FAILED:
    Serial.println(F("EV_REJOIN_FAILED"));
    break;
    break;
  case EV_TXCOMPLETE:
    Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
    setLoraMessageOnOLED("Data Sent");
    if (LMIC.txrxFlags & TXRX_ACK)
      Serial.println(F("Received ack"));
    if (LMIC.dataLen) {
      Serial.print(F("Received "));
      Serial.print(LMIC.dataLen);
      Serial.println(F(" bytes of payload"));
      
      setLoraMessageOnOLED("Data received");
      Serial.println(F("Data is "));

      // Change the following codes to process incoming data !!
      for (int counter = 0; counter < LMIC.dataLen; counter++) {
        Serial.print(LMIC.frame[LMIC.dataBeg + counter], HEX);
      }
      Serial.println(F(" "));

    }
    // Schedule next transmission
//    os_setTimedCallback( & sendjob, os_getTime() + sec2osticks(), do_send);
    break;
  case EV_LOST_TSYNC:
    Serial.println(F("EV_LOST_TSYNC"));
    break;
  case EV_RESET:
    Serial.println(F("EV_RESET"));
    break;
  case EV_RXCOMPLETE:
    // data received in ping slot
    Serial.println(F("EV_RXCOMPLETE"));
    break;
  case EV_LINK_DEAD:
    Serial.println(F("EV_LINK_DEAD"));
    break;
  case EV_LINK_ALIVE:
    Serial.println(F("EV_LINK_ALIVE"));
    break;
  case EV_TXSTART:
    Serial.println(F("EV_TXSTART"));
    break;
  case EV_TXCANCELED:
    Serial.println(F("EV_TXCANCELED"));
    break;
  case EV_JOIN_TXCOMPLETE:
    Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
    break;

  default:
    Serial.print(F("Unknown event: "));
    Serial.println((unsigned) ev);
    break;
  }
}

void do_send(osjob_t * j) {
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
    setLoraMessageOnOLED("Not sending");
  } else {
    // Prepare upstream data transmission at the next possible time.
    LMIC_setTxData2(1, loraDataPackets, sizeof(loraDataPackets) - 1, 0);
    Serial.println(F("Packet queued"));
    setLoraMessageOnOLED("Packet queued");
  }
  // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
  delay(5000);

//  Intitialize the serial interface for Debug
  while (!Serial)
  ;
  Serial.begin(115200);
  Serial.println(F("Starting"));

  // Initalize SoftwareSerial interface for PZEM module
  pzemSerialObj.begin(9600, SWSERIAL_8N1, softRxPin, softTxPin, false, 200, 10);

  //Intialize the OLED module
  initializeOLED();
  setLoraMessageOnOLED("Starting....");
  
  //Making sure the Region is set properly
  #ifdef CFG_in866
  Serial.println(F("Module Configured for Inidian LoRa band (865-867 MHz)"));
    setLoraMessageOnOLED("Using IN866 Band");
  #endif
  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();
  //    LMIC_setClockError(MAX_CLOCK_ERROR * 5 / 100);

// Serial.println(F("Setting up the PZEM module address to 192.168.1.1"));
//  fetchData(address_);
//  printPzemResponseBuffer();
  updateMeterData();
  do_send(&sendjob);
}

void loop() {
  os_runloop_once();
  if((millis() - lastDisplayUpdateTime) > (displayUpdateInterval * 1000)){
    lastDisplayUpdateTime = millis();
    updateMeterData();
    display_.drawString(8, 2,(String(voltage)+"V").c_str ());
    display_.drawString(8, 3, (String(current)+"A").c_str ());
    display_.drawString(8, 4, (String(power)+"W").c_str ());
    display_.drawString(8, 5, (String(energy)+"Kwh").c_str ());
  }
  if((millis() - lastLoraDataPublishTime) > (loraDataPublishInterval * 1000)){
    lastLoraDataPublishTime = millis();
    updateMeterData();
    do_send(&sendjob); // Send the meter data w/ LoRaWAN
  }
}
void initializeOLED(){
  display_.begin();
  display_.setFont(u8x8_font_pxplusibmcgathin_r);
  display_.drawString(0, 0, "WGLabz");

  //  Display Energy params label on OLED
  display_.drawString(0, 2, "Voltage: ");
  display_.drawString(0, 3, "Current: ");
  display_.drawString(0, 4, "Power: ");
  display_.drawString(0, 5, "Energy: ");

  display_.drawString(8, 2, "230.00V");
  display_.drawString(8, 3, "0.04A");
  display_.drawString(8, 4, "2.00W");
  display_.drawString(8, 5, "56.00Kwh");

  display_.drawString(0, 7, "LoRaWAN Status");
}
void clearOLEDLine(int line){
//  Print a blank text line to the OLED display in the passed line val.
  display_.drawString(0, line, "                    ");
}

void setLoraMessageOnOLED(char* message){
//    delay(2000);
    clearOLEDLine(7);
    display_.drawString(0, 7, message);
}

// PZEM Module COmmunication related functions

bool updateMeterData(){
    voltage = fetchData(voltage_) ? (pzem_response_buffer[1] << 8) + pzem_response_buffer[2] +(pzem_response_buffer[3] / 10.0) : -1;
    if(DEBUG)
      printPzemResponseBuffer();
    loraDataPackets[0] = pzem_response_buffer[1];
    loraDataPackets[1] = pzem_response_buffer[2];
    loraDataPackets[2] = pzem_response_buffer[3];
    current = fetchData(current_)? (pzem_response_buffer[1] << 8) + pzem_response_buffer[2]+ (pzem_response_buffer[3] / 100.0) : -1;   
    if(DEBUG)
      printPzemResponseBuffer();
    power = fetchData(power_) ? (pzem_response_buffer[1] << 8) + pzem_response_buffer[2]: -1;
    if(DEBUG)
      printPzemResponseBuffer();
    loraDataPackets[3] = pzem_response_buffer[1];
    loraDataPackets[4] = pzem_response_buffer[2];
    energy = fetchData(energy_) ? ((uint32_t)pzem_response_buffer[1] << 16) + ((uint16_t)pzem_response_buffer[2] << 8) + pzem_response_buffer[3] : -1;
    if(DEBUG)
      printPzemResponseBuffer();
    loraDataPackets[5] = pzem_response_buffer[1];
    loraDataPackets[6] = pzem_response_buffer[2];
    loraDataPackets[7] = pzem_response_buffer[3];
}
bool fetchData(uint8_t *command){
  while(pzemSerialObj.available() > 0){ //Empty in buffer if it holds any data
    pzemSerialObj.read();
  }
  for(int count=0;count < 7; count++)
    pzemSerialObj.write(command[count]);

  int startTime = millis();
  int receivedBytes = 0; 

  while((receivedBytes < 7 ) && ((millis()- startTime) < 10000)){ // Waits till it recives 7 bytes or a timout of 10 second happens
      if(pzemSerialObj.available() > 0){
        pzem_response_buffer[receivedBytes++] = (uint8_t)pzemSerialObj.read();
       }
       yield();
    }
  return receivedBytes == 7;
}

void printPzemResponseBuffer(){ //For Debugging
  Serial.print("Buffer Data: ");
  for(int x =0 ;x < 7; x++){
    Serial.print(int(pzem_response_buffer[x]));
    Serial.print(" ");
  }
   Serial.println("");
  }
