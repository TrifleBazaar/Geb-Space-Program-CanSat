#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <RH_RF95.h>

#define BMP280_ADDRESS 0x76
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
#define RF95_FREQ 868.0
#define DEBUG 0

Adafruit_BMP280 bmp; // I2C
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
  while ( !Serial ) delay(100);   // wait for native usb // only use if plugged into pc or laptop
  Serial.println(F("HELLO"));
  Serial.println(F("BMP280 test"));
#endif

  // LoRa reset and init
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!rf95.init()) {
  #ifdef DEBUG
    Serial.println(F("LoRa init failed"));
  #endif
    while (1) delay(10);
  }
  if (!rf95.setFrequency(RF95_FREQ)) {
  #ifdef DEBUG
    Serial.println(F("setFrequency failed"));
  #endif
    while (1) delay(10);
  }
  rf95.setTxPower(13, false);

  unsigned status;
  status = bmp.begin(BMP280_ADDRESS);
  if (!status) {
  #ifdef DEBUG
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
  #endif
    while (1) delay(10);
  }
  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void loop() {
  float temp = bmp.readTemperature();
  float pres = bmp.readPressure();
  float alt = bmp.readAltitude(1033.45);
#ifdef DEBUG
  Serial.print(F("Temperature = "));
  Serial.print(temp);
  Serial.println(" *C");
  Serial.print(F("Pressure = "));
  Serial.print(pres);
  Serial.println(" Pa");
  Serial.print(F("Approx altitude = "));
  Serial.print(alt);
  Serial.println(" m");
  Serial.println();
#endif

  // Send over LoRa as: temp,pressure,altitude
  String msg = String(temp) + "," + String(pres) + "," + String(alt);
  char buf[64];
  msg.toCharArray(buf, sizeof(buf));
  rf95.send((uint8_t *)buf, strlen(buf) + 1);
  rf95.waitPacketSent();

  delay(1000);
}
