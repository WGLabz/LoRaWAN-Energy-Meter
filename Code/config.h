//TTN Keys
static const u1_t PROGMEM APPEUI[8] = { 0x2F, 0x2D, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getArtEui(u1_t * buf) { memcpy_P(buf, APPEUI, 8);}

static const u1_t PROGMEM DEVEUI[8] = { 0xDA, 0xAF, 0x64, 0x43, 0xD9, 0xE0, 0x0B, 0x00 };
void os_getDevEui(u1_t * buf) { memcpy_P(buf, DEVEUI, 8);}

static const u1_t PROGMEM APPKEY[16] = { 0x7D, 0xEE, 0x6A, 0x88, 0xA0, 0xB4, 0xE9, 0xD0, 0x9E, 0xC8, 0xED, 0x21, 0xBA, 0x5F, 0x96, 0x3B };
void os_getDevKey(u1_t * buf) { memcpy_P(buf, APPKEY, 16);}

//Timing Intervals
int displayUpdateInterval = 60; // In seconds
int loraDataPublishInterval = 60; //In Seconds

// SoftwareSerial Pins, used to connect the PZEM module
int softRxPin = 13;
int softTxPin = 21;
