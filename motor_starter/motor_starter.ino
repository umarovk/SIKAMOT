/*
  SIKAMOT - Sistem Keamanan Motor dengan Auto Starter
  Board   : Arduino Nano
  Modul   : RC522 I2C V1.1 (chip MFRC522)
  Library : MFRC522_I2C by arozcan (header: MFRC522_I2C.h, class: MFRC522)

  CARA KERJA:
    - Tap kartu valid (pertama)  → kunci kontak ON + starter crank → MOTOR HIDUP
    - Tap kartu valid (kedua)    → kunci kontak OFF + starter OFF → MOTOR MATI
    - Tap kartu salah            → ditolak, state tidak berubah

  WIRING RFID → Nano
    3.3V → 3.3V
    GND  → GND
    RST  → D9
    SCL  → A5
    SDA  → A4

  WIRING 2-Channel Relay → Nano
    VCC → 5V
    GND → GND
    IN1 → D7  (Relay 1: KUNCI KONTAK)
    IN2 → D8  (Relay 2: STARTER)

  WIRING OUTPUT RELAY → MOTOR (12V)
    Relay 1 (Kunci Kontak):
      Aki 12V (+) → COM1
      NO1         → Kabel kunci kontak (ke CDI/koil/ECU)
    Relay 2 (Starter):
      Aki 12V (+) → COM2
      NO2         → Kabel starter (parallel dengan tombol starter)

    Aki 12V (−) → GND aki motor (DAN ke GND Nano — ground harus disatukan)

  PERINGATAN ARUS:
    - Modul relay biasa max 10A — CUKUP untuk kunci kontak (~2A)
    - Starter motor butuh 30-100A — TIDAK CUKUP pakai relay modul biasa!
    - Untuk starter, pakai RELAY OTOMOTIF (Bosch 5-pin 30A/40A)
      yang digerakkan oleh relay modul (cascade)
*/

#include <Wire.h>
#include <MFRC522_I2C.h>

// ===================== PIN =====================
#define RFID_ADDR     0x28
#define RST_PIN       9
#define IGNITION_PIN  7   // Relay 1 - kunci kontak
#define STARTER_PIN   8   // Relay 2 - starter motor

// ===================== KONFIGURASI =====================
#define RELAY_ON      LOW    // balik ke HIGH jika relay active-HIGH
#define RELAY_OFF     HIGH

const unsigned long STARTER_DELAY    = 500;    // jeda kunci kontak → starter (ms)
const unsigned long STARTER_DURATION = 2000;   // durasi starter crank (ms)
const unsigned long TAP_COOLDOWN     = 2000;   // jeda minimum antar tap kartu

// UID kartu yang diizinkan — GANTI sesuai UID asli kamu
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
  rfid.PCD_Init();

  Serial.println(F("=== SIKAMOT - Sistem Keamanan Motor ==="));
  Serial.println(F("Status: OFF"));
  Serial.println(F("Tap kartu RFID untuk menghidupkan/mematikan motor"));
  Serial.println(F("---"));
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial())   return;

  String uid = readUID();
  rfid.PICC_HaltA();

  // Cegah double-tap (anti bouncing antar tap kartu)
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
}

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
