#include <SPI.h>
#include <LoRa.h>

#define MY_ADDR       0x41CADF0E
#define DEVICE_TOKEN  "gpanUHr504v4Is3GjZ4DKmHsL8skL3Fg"

struct {
  uint32_t to_addr;
  uint32_t from_addr;
  char token[32];
  uint16_t payload_size;
  uint8_t crc;
} dataHeader;

uint8_t genCRC(uint8_t* data, uint16_t len) {
  uint8_t sum = 0;
  for (uint16_t i=0;i<len;i++) {
    sum += data[i];
  }
  sum = ~sum;
  return sum;
}

int counter = 0;

void setup() {
  Serial.begin(9600);
  
  Serial.println("LoRa Sender");

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  dataHeader.to_addr = 0xcf854753; // Gateway Address
  dataHeader.from_addr = MY_ADDR;
  memcpy(dataHeader.token, DEVICE_TOKEN, 32);
}

void loop() {
  Serial.print("Sending packet: ");

  String data = "Hello, LoRa from Sender 2 at " + String(millis());
  Serial.println(data);

  dataHeader.payload_size = data.length();
  dataHeader.crc = genCRC(data.c_str(), data.length());

  // send packet
  LoRa.beginPacket();
  LoRa.write((uint8_t*)(&dataHeader), sizeof(dataHeader));
  LoRa.print(data);
  LoRa.endPacket();

  delay(500);
}
