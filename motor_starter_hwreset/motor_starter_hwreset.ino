/*
  SIKAMOT - Sistem Keamanan Motor + Auto Starter (Hardware Reset)
  Board   : Arduino Nano
  Modul   : RC522 I2C V1.1 (chip MFRC522)
  Library : MFRC522_I2C by arozcan

  VARIAN HARDWARE RESET:
    Versi ini pakai hardware reset MFRC522 lewat pin RST (D9) sebagai
    backup kalau soft reset (PCD_Init saja) tidak cukup membangunkan
    modul setelah blocking delay panjang dari turnOnMotor().

    Hardware reset = pulse pin RST LOW → HIGH untuk reset chip MFRC522
    secara fisik, lebih agresif dari soft reset, dan biasanya berhasil
    bahkan saat I2C bus state sudah kacau total.

  KAPAN PAKAI VERSI INI?
    - Pakai motor_starter.ino dulu (soft reset PCD_Init)
    - Kalau modul TETAP hang setelah tap pertama (tidak baca tap kedua),
      ganti ke versi ini

  CARA KERJA: (sama dengan motor_starter.ino)
    - Tap kartu valid (pertama)  → kunci kontak ON + starter crank → MOTOR HIDUP
    - Tap kartu valid (kedua)    → kunci kontak OFF + starter OFF → MOTOR MATI
    - Tap kartu salah            → ditolak, state tidak berubah

  WIRING: TIDAK BERUBAH dari motor_starter.ino
    RFID: 3.3V, GND, RST=D9, SCL=A5, SDA=A4
    Relay Modul: VCC=5V, GND, IN1=D7, IN2=D8
    Cascade Bosch → 2 kabel kunci kontak + 2 kabel tombol starter
*/

#include <Wire.h>
#include <MFRC522_I2C.h>

// ===================== PIN =====================
#define RFID_ADDR     0x28
#define RST_PIN       9     // pin reset RFID (DIPAKAI UNTUK HARDWARE RESET)
#define IGNITION_PIN  7
#define STARTER_PIN   8

// ===================== KONFIGURASI =====================
#define RELAY_ON      LOW
#define RELAY_OFF     HIGH

const unsigned long STARTER_DELAY    = 500;
const unsigned long STARTER_DURATION = 2000;
const unsigned long TAP_COOLDOWN     = 2000;

// Durasi pulse hardware reset (ms)
const unsigned long HWRESET_LOW_MS   = 50;
const unsigned long HWRESET_HIGH_MS  = 50;

const String allowedUIDs[] = {
  "DE AD BE EF",
  "A1 B2 C3 D4",
  "61 FB C1 01",
  "0D 86 F6 03"
};
const int allowedCount = sizeof(allowedUIDs) / sizeof(allowedUIDs[0]);

// ===================== STATE =====================
MFRC522 rfid(RFID_ADDR, RST_PIN);
bool motorOn = false;
unsigned long lastTapTime = 0;

void setup() {
  Serial.begin(9600);

  pinMode(IGNITION_PIN, OUTPUT);
  pinMode(STARTER_PIN, OUTPUT);
  digitalWrite(IGNITION_PIN, RELAY_OFF);
  digitalWrite(STARTER_PIN, RELAY_OFF);

  Wire.begin();
  hardResetRFID();   // hardware reset di awal untuk pastikan modul fresh

  Serial.println(F("=== SIKAMOT - Sistem Keamanan Motor (HW Reset) ==="));
  Serial.println(F("Status: OFF"));
  Serial.println(F("Tap kartu RFID untuk menghidupkan/mematikan motor"));
  Serial.println(F("---"));
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial())   return;

  String uid = readUID();
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  if (millis() - lastTapTime < TAP_COOLDOWN) {
    Serial.println(F("[!] Tunggu sebentar sebelum tap lagi..."));
    return;
  }

  Serial.print(F("UID: "));
  Serial.println(uid);

  if (!isAuthorized(uid)) {
    Serial.println(F("[X] AKSES DITOLAK - Kartu tidak terdaftar"));
    Serial.println(F("---"));
    return;
  }

  lastTapTime = millis();

  if (motorOn) {
    turnOffMotor();
  } else {
    turnOnMotor();
  }

  // Hardware reset RFID setelah delay panjang turnOn/turnOff
  hardResetRFID();
}

// ===================== HARDWARE RESET MFRC522 =====================
// Pulse pin RST LOW → HIGH untuk reset chip secara fisik,
// lalu re-init lewat library.
void hardResetRFID() {
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, LOW);
  delay(HWRESET_LOW_MS);
  digitalWrite(RST_PIN, HIGH);
  delay(HWRESET_HIGH_MS);
  rfid.PCD_Init();
}

// ===================== MOTOR CONTROL =====================
void turnOnMotor() {
  Serial.println(F("[OK] AKSES DITERIMA"));
  Serial.println(F(">>> KUNCI KONTAK ON"));
  digitalWrite(IGNITION_PIN, RELAY_ON);

  delay(STARTER_DELAY);

  Serial.println(F(">>> STARTER CRANK..."));
  digitalWrite(STARTER_PIN, RELAY_ON);
  delay(STARTER_DURATION);
  digitalWrite(STARTER_PIN, RELAY_OFF);

  Serial.println(F(">>> MOTOR HIDUP"));
  Serial.println(F("Status: ON"));
  Serial.println(F("Tap kartu untuk mematikan"));
  Serial.println(F("---"));

  motorOn = true;
}

void turnOffMotor() {
  Serial.println(F("[OK] AKSES DITERIMA"));
  Serial.println(F(">>> MOTOR OFF"));
  digitalWrite(IGNITION_PIN, RELAY_OFF);
  digitalWrite(STARTER_PIN, RELAY_OFF);

  Serial.println(F("Status: OFF"));
  Serial.println(F("Tap kartu untuk menghidupkan"));
  Serial.println(F("---"));

  motorOn = false;
}

// ===================== RFID HELPER =====================
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
