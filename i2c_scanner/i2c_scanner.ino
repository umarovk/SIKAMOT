/*
  I2C Scanner — cek alamat perangkat I2C yang terhubung ke Nano
  Wiring:
    Modul SDA → A4 Nano
    Modul SCL → A5 Nano
    Modul VCC → 3.3V (untuk RC522 I2C) atau 5V (untuk modul lain)
    Modul GND → GND
*/

#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(9600);
  while (!Serial);
  Serial.println(F("\n=== I2C Scanner ==="));
}

void loop() {
  byte error, address;
  int count = 0;

  Serial.println(F("Scanning..."));

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print(F("Device ditemukan di alamat 0x"));
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      count++;
    } else if (error == 4) {
      Serial.print(F("Unknown error di 0x"));
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  if (count == 0) {
    Serial.println(F("Tidak ada perangkat I2C terdeteksi!"));
    Serial.println(F("Cek wiring SDA(A4) dan SCL(A5), power, dan ground."));
  } else {
    Serial.print(F("Total perangkat: "));
    Serial.println(count);
  }

  Serial.println(F("---"));
  delay(3000);
}
