#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

// Library for the OLED module
#include <U8x8lib.h>

// Include the TTN security keys for OTAA defined in ttn_keys.h file
#include "ttn_keys.h"

// Disable PING and Beacon as is not required for Class A LoRaWAN operation
#define DISABLE_PING 1
#define DISABLE_BEACONS 1

// Function prototype declarations
void clearOLEDLine(int line);
void setLoraMessageOnOLED(char* message);

// Object for the OLED
U8X8_SSD1306_128X64_NONAME_SW_I2C display_(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

static uint8_t mydata[] = "Hello, from WGLabz!";
static osjob_t sendjob;

const unsigned TX_INTERVAL = 60;

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
//    os_setTimedCallback( & sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
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
    LMIC_setTxData2(1, mydata, sizeof(mydata) - 1, 0);
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
  do_send( & sendjob);


}

void loop() {
  os_runloop_once();
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
