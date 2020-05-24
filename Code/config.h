#define DEBUG true
//TTN Keys
static const u1_t PROGMEM APPEUI[8] = { 0x5B, 0xF5, 0x02, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getArtEui(u1_t * buf) { memcpy_P(buf, APPEUI, 8);}

static const u1_t PROGMEM DEVEUI[8] = { 0x5C, 0x09, 0x35, 0x0E, 0xF6, 0xC5, 0xD8, 0x00 };
void os_getDevEui(u1_t * buf) { memcpy_P(buf, DEVEUI, 8);}

static const u1_t PROGMEM APPKEY[16] = { 0x31, 0x44, 0xEE, 0xC5, 0x2C, 0xCD, 0x5C, 0x05, 0x6F, 0x0F, 0x6E, 0xCD, 0x5B, 0x79, 0x49, 0x8A };
void os_getDevKey(u1_t * buf) { memcpy_P(buf, APPKEY, 16);}

//Timing Intervals
int displayUpdateInterval = 60; // In seconds
int loraDataPublishInterval = 300; //In Seconds

// SoftwareSerial Pins, used to connect the PZEM module
int softRxPin = 13;
int softTxPin = 21;
