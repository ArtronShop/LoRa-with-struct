#include <SPI.h>
#include <LoRa.h>

#define MY_ADDR      0xCF854753
#define DEVICE_TOKEN  "gpanUHr504v4Is3GjZ4DKmHsL8skL3Fg"

#define LORA_SS   5
#define LORA_RST  17
#define LORA_DIO0 4

uint32_t nodeList[] = {
  0x7947AED8,
  0x41CADF0E
};

struct {
  uint32_t to_addr;
  uint32_t from_addr;
  char token[32];
  uint16_t payload_size;
  uint8_t crc;
} __attribute__ ((packed)) dataHeader;

uint8_t genCRC(uint8_t *data, uint16_t len) {
  uint8_t sum = 0;
  for (uint16_t inx=0; inx < len; inx++) {
    sum += data[inx];
  }
  sum = ~sum;
  return sum;
}

void clearBuffer() {
  while (LoRa.available()) LoRa.read();
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Receiver Callback");

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // register the receive callback
  LoRa.onReceive(onReceive);

  // put the radio into receive mode
  LoRa.receive();
}

void loop() {
  delay(100);
}

void onReceive(int packetSize) {
  Serial.println();

  Serial.print("Received packet size ");
  Serial.print(packetSize);
  Serial.print(" with RSSI ");
  Serial.println(LoRa.packetRssi());

  if (packetSize < sizeof(dataHeader)) {
    Serial.println("Packet size error");
    return;
  }

  for (int i = 0; i < sizeof(dataHeader); i++) {
    ((uint8_t*)(&dataHeader))[i] = (uint8_t)LoRa.read();
  }

  Serial.print("Data Received from 0x");
  Serial.print(dataHeader.from_addr, HEX);
  Serial.println();

  if (dataHeader.to_addr != MY_ADDR) {
    Serial.println("Data not send to me, reject data !");
  }

  bool foundInList = false;
  for (int i = 0; i < (sizeof(nodeList) / sizeof(uint32_t)); i++) {
    if (nodeList[i] == dataHeader.from_addr) {
      foundInList = true;
      break;
    }
  }

  if (!foundInList) {
    Serial.println("Not found in nodelist, reject data !");
    return;
  }


  if (memcmp(dataHeader.token, DEVICE_TOKEN, 32) != 0) {
    Serial.println("Token invalid, reject data !");
    return;
  }

  if ((packetSize - sizeof(dataHeader)) != dataHeader.payload_size) {
    Serial.println("Payload size invalid, reject data !");
    return;
  }

  String data = "";
  while (LoRa.available()) {
    data += (char)LoRa.read();
  }

  if (dataHeader.crc != genCRC((uint8_t*)data.c_str(), data.length())) {
    Serial.println("CRC invalid, reject data !");
    return;
  }

  Serial.print("Data: ");
  Serial.println(data);
}
