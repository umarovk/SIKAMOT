/*
  RFID I2C + Relay — Sistem Keamanan Motor (versi HARDWARE)
  Board   : Arduino Nano
  Modul   : RC522 I2C V1.1 (chip MFRC522 dalam mode I2C)
  Library : MFRC522_I2C by arozcan (install via .ZIP)

  Wiring RFID → Nano
    3.3V → 3.3V
    GND  → GND
    RST  → D9
    SCL  → A5
    SDA  → A4
    IRQ  → (tidak dipakai)

  Wiring Relay → Nano
    VCC → 5V
    GND → GND
    IN  → D7

  Wiring Relay Output → LED
    Nano 5V → COM
    NO      → Resistor 220Ω → LED(+)
    LED(-)  → GND
*/

#include <Wire.h>
#include <MFRC522_I2C.h>

#define RFID_ADDR  0x28   // alamat hasil scan I2C
#define RST_PIN    9
#define RELAY_PIN  7

#define RELAY_ON   LOW    // balik ke HIGH kalau relay ACTIVE-HIGH
#define RELAY_OFF  HIGH

const unsigned long ON_DURATION = 5000;  // 5 detik LED/motor menyala

// UID kartu yang diizinkan — GANTI sesuai UID kartu kamu
// Lihat UID asli dari Serial Monitor saat pertama kali tap
const String allowedUIDs[] = {
  "DE AD BE EF",
  "A1 B2 C3 D4"
};
const int allowedCount = sizeof(allowedUIDs) / sizeof(allowedUIDs[0]);

MFRC522 rfid(RFID_ADDR, RST_PIN);

void setup() {
  Serial.begin(9600);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);

  Wire.begin();
  rfid.PCD_Init();

  Serial.println(F("=== RFID I2C Access Control ==="));
  Serial.println(F("Tap kartu ke reader..."));
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial())   return;

  String uid = readUID();
  Serial.print(F("UID terbaca: "));
  Serial.println(uid);

  if (isAuthorized(uid)) {
    Serial.println(F("AKSES DITERIMA — Motor/LED ON"));
    digitalWrite(RELAY_PIN, RELAY_ON);
    delay(ON_DURATION);
    digitalWrite(RELAY_PIN, RELAY_OFF);
    Serial.println(F("Motor/LED OFF"));
  } else {
    Serial.println(F("AKSES DITOLAK"));
  }

  rfid.PICC_HaltA();
}

String readUID() {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) uid += " ";
  }
  uid.toUpperCase();
  return uid;
}

bool isAuthorized(const String &uid) {
  for (int i = 0; i < allowedCount; i++) {
    if (uid == allowedUIDs[i]) return true;
  }
  return false;
}
