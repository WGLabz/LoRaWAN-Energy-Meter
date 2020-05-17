#include <SoftwareSerial.h>
// Used SoftwareSerial library for ESP32 https://github.com/jdollar/espsoftwareserial
SoftwareSerial swSer;

uint8_t current_[7] = {0xB1,0xC0,0xA8,0x01,0x01,0x00,0x1B};
uint8_t address_[7] = {0xB4,0xC0,0xA8,0x01,0x01,0x00,0x1E};
uint8_t energy_[7]  = {0xB3,0xC0,0xA8,0x01,0x01,0x00,0x1D};
uint8_t voltage_[7] = {0xB0,0xC0,0xA8,0x01,0x01,0x00,0x1A};
uint8_t power_[7] =   {0xB2,0xC0,0xA8,0x01,0x01,0x00,0x1C};


void setup() {
  Serial.begin(115200);
  Serial.println("Hello");
  swSer.begin(9600, SWSERIAL_8N1, 13, 21, false, 200, 10);
}

void loop() {
  Serial.print("Voltage: ");
  Serial.print(readVoltage());
  Serial.println("Volt");
  Serial.print("Current: ");
  Serial.print(readCurrent());
  Serial.println("Amps");
  Serial.print("Power: ");
  Serial.print(readPower());
  Serial.println("Watt");
  Serial.print("Energy: ");
  Serial.print(readEnergy());
  Serial.println("Wh");
  delay(10000);
}

uint8_t pzem_response_buffer[7];

// PZEM Module COmmunication related functions
float readVoltage(){
  if(fetchData(voltage_)){
    return (pzem_response_buffer[1] << 8) + pzem_response_buffer[2] +(pzem_response_buffer[3] / 10.0);
  }
  else
    return -1.00;  
}
float readCurrent(){
  if(fetchData(current_)){
    return (pzem_response_buffer[1] << 8) + pzem_response_buffer[2]+ (pzem_response_buffer[3] / 1000.0);
  }
  else
    return -1.00;  
}
float readPower(){
  if(fetchData(power_)){
    return (pzem_response_buffer[1] << 8) + pzem_response_buffer[2];
  }
  else
    return -1.00;  
}
float readEnergy(){
  if(fetchData(energy_)){
    return ((uint32_t)pzem_response_buffer[1] << 16) + ((uint16_t)pzem_response_buffer[2] << 8) + pzem_response_buffer[3];
  }
  else
    return -1.00;  
}
bool fetchData(uint8_t *command){
  while(swSer.available() > 0){ //Empty in buffer if it holds any data
    swSer.read();
  }
  for(int count=0;count < 7; count++)
    swSer.write(command[count]);

  int startTime = millis();
  int receivedBytes = 0; 

  while((receivedBytes < 7 ) && ((millis()- startTime) < 10000)){ // Waits till it recives 7 bytes or a timout of 10 second happens
      if(swSer.available() > 0){
        pzem_response_buffer[receivedBytes++] = (uint8_t)swSer.read();
       }
       yield();
    }
  return receivedBytes == 7;
}

void printPzemResponseBuffer(){
  Serial.print("Buffer Data: ");
  for(int x =0 ;x < 7; x++){
    Serial.print(int(pzem_response_buffer[x]));
    Serial.print(" ");
  }
   Serial.println("");
  }
