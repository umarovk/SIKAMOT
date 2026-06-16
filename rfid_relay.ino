/*
  RFID + Relay (LED simulasi DC Motor)
  Board   : Arduino Nano
  Library : MFRC522 (by GithubCommunity) — install via Library Manager

  Wiring RFID-RC522 → Nano
    SDA  → D10
    SCK  → D13
    MOSI → D11
    MISO → D12
    RST  → D9
    GND  → GND
    3.3V → 3.3V

  Wiring Relay → Nano
    VCC → 5V
    GND → GND
    IN  → D7

  Wiring Relay Output → LED
    Nano 5V → COM
    NO      → Resistor 220Ω → LED(+)
    LED(-)  → GND
*/

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN    10
#define RST_PIN   9
#define RELAY_PIN 7

// Banyak modul relay 1-channel itu ACTIVE LOW.
// Kalau di Wokwi/relay kamu ternyata ACTIVE HIGH, tinggal balik nilainya.
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

// Berapa lama LED/motor menyala setelah kartu valid (ms)
const unsigned long ON_DURATION = 5000;

// Daftar UID kartu yang diizinkan (format hex, huruf besar, dipisah spasi)
// Ganti sesuai UID kartu kamu. Di Wokwi default biasanya "DE AD BE EF".
const String allowedUIDs[] = {
  "DE AD BE EF",
  "A1 B2 C3 D4",
  "1234",
  "01 02 03 04"
};
const int allowedCount = sizeof(allowedUIDs) / sizeof(allowedUIDs[0]);

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);

  SPI.begin();
  rfid.PCD_Init();

  Serial.println(F("=== RFID Access Control ==="));
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
  rfid.PCD_StopCrypto1();
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
