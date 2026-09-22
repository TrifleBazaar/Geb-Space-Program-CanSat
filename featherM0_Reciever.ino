#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
#define RF95_FREQ 868.0
#define DEBUG 0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
  Serial.begin(9600);
#ifdef DEBUG
  while ( !Serial ) delay(100);
#endif

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!rf95.init()) {
    Serial.println(F("LoRa init failed"));
    while (1) delay(10);
  }
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println(F("setFrequency failed"));
    while (1) delay(10);
  }
  rf95.setTxPower(13, false);
}

void loop() {
  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      String line = String((char *)buf);
      int c1 = line.indexOf(',');
      int c2 = line.indexOf(',', c1 + 1);
      float temp = line.substring(0, c1).toFloat();
      float pres = line.substring(c1 + 1, c2).toFloat();
      float alt = line.substring(c2 + 1).toFloat();

      Serial.print("Temperature = "); Serial.print(temp); Serial.println(" *C");
      Serial.print("Pressure = "); Serial.print(pres); Serial.println(" Pa");
      Serial.print("Approx altitude = "); Serial.print(alt); Serial.println(" m");
      Serial.println();
    }
  }
}
