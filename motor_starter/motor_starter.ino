/*
  SIKAMOT - Sistem Keamanan Motor + Auto Starter (SPI Version)
  Board   : Arduino Nano
  Modul   : MFRC522 8-pin SPI v2.0 (SDA/SCK/MOSI/MISO/IRQ/GND/RST/3.3V)
  Library : MFRC522 by GithubCommunity (SPI version, install via Library Manager)

  CARA KERJA:
    - Tap kartu valid (pertama)  → kunci kontak ON + starter crank → MOTOR HIDUP
    - Tap kartu valid (kedua)    → kunci kontak OFF + starter OFF → MOTOR MATI
    - Tap kartu salah            → ditolak, state tidak berubah

  WIRING RFID 8-pin → Nano (SPI)
    SDA  → D10  (SS / Chip Select SPI)
    SCK  → D13  (Clock SPI)
    MOSI → D11  (Master Out Slave In)
    MISO → D12  (Master In Slave Out)
    IRQ  → (kosong, tidak dipakai)
    GND  → GND
    RST  → D9
    3.3V → 3.3V (JANGAN ke 5V — modul rusak)

  WIRING 2-Channel Relay → Nano
    VCC → 5V
    GND → GND
    IN1 → D7  (Relay 1: KUNCI KONTAK)
    IN2 → D8  (Relay 2: STARTER)

  WIRING OUTPUT (CASCADE: Relay Modul → Bosch → Motor)

    Relay Modul Channel 1 (D7) trigger Bosch #1:
      Aki 12V (+) → COM1 modul
      NO1 modul   → Bosch #1 pin 86 (coil +)
      Bosch #1 pin 85 → GND aki (coil −)
      Bosch #1 pin 30  ↔ pin 87  → bridge 2 KABEL KUNCI KONTAK motor

    Relay Modul Channel 2 (D8) trigger Bosch #2:
      Aki 12V (+) → COM2 modul
      NO2 modul   → Bosch #2 pin 86 (coil +)
      Bosch #2 pin 85 → GND aki (coil −)
      Bosch #2 pin 30  ↔ pin 87  → bridge 2 KABEL TOMBOL STARTER motor

    Aki 12V (−) → GND aki motor (DAN ke GND Nano — ground harus disatukan)

  CATATAN:
    - Library MFRC522 (SPI) vs MFRC522_I2C (I2C) BENTROK — install salah satu
    - Untuk varian ini, install MFRC522 by GithubCommunity, uninstall MFRC522_I2C
    - Output Bosch 30↔87 bekerja sebagai SAKLAR PENGGANTI kunci kontak/starter
    - Dioda 1N4007 paralel coil Bosch (katoda ke pin 86) untuk proteksi spike
*/

#include <SPI.h>
#include <MFRC522.h>

// ===================== PIN =====================
#define SS_PIN        10    // SDA RFID → D10 Nano (SS SPI)
#define RST_PIN       9     // RST RFID → D9 Nano
#define IGNITION_PIN  7     // Relay 1 - kunci kontak
#define STARTER_PIN   8     // Relay 2 - starter motor

// ===================== KONFIGURASI =====================
#define RELAY_ON      LOW   // balik ke HIGH jika relay active-HIGH
#define RELAY_OFF     HIGH

const unsigned long STARTER_DELAY    = 500;    // jeda kunci kontak → starter (ms)
const unsigned long STARTER_DURATION = 2000;   // durasi starter crank (ms)
const unsigned long TAP_COOLDOWN     = 2000;   // jeda minimum antar tap kartu

// UID kartu yang diizinkan (4 byte hex, huruf besar, dipisah spasi)
const String allowedUIDs[] = {
  "3C 8A 52 07"   // kartu utama (Mifare 1KB)
};
const int allowedCount = sizeof(allowedUIDs) / sizeof(allowedUIDs[0]);

// ===================== STATE =====================
MFRC522 rfid(SS_PIN, RST_PIN);
bool motorOn = false;
unsigned long lastTapTime = 0;

void setup() {
  Serial.begin(9600);

  pinMode(IGNITION_PIN, OUTPUT);
  pinMode(STARTER_PIN, OUTPUT);
  digitalWrite(IGNITION_PIN, RELAY_OFF);
  digitalWrite(STARTER_PIN, RELAY_OFF);

  SPI.begin();
  rfid.PCD_Init();

  Serial.println(F("=== SIKAMOT - Sistem Keamanan Motor (SPI) ==="));
  Serial.println(F("Status: OFF"));
  Serial.println(F("Tap kartu RFID untuk menghidupkan/mematikan motor"));
  Serial.println(F("---"));
}

void loop() {
  // Cek kartu di field — pakai REQA (kartu baru) atau WUPA (kartu halted).
  byte atqa[2];
  byte atqaSize = sizeof(atqa);
  MFRC522::StatusCode reqResult = rfid.PICC_RequestA(atqa, &atqaSize);
  if (reqResult != MFRC522::STATUS_OK) {
    atqaSize = sizeof(atqa);
    MFRC522::StatusCode wupResult = rfid.PICC_WakeupA(atqa, &atqaSize);
    if (wupResult != MFRC522::STATUS_OK) {
      // ===== DEBUG (hapus setelah issue selesai) =====
      static unsigned long lastDbg = 0;
      if (millis() - lastDbg > 3000) {
        Serial.print(F("[DBG] motorOn="));
        Serial.print(motorOn);
        Serial.print(F(" reqA=0x"));
        Serial.print(reqResult, HEX);
        Serial.print(F(" wupA=0x"));
        Serial.println(wupResult, HEX);
        lastDbg = millis();
      }
      return;
    }
    Serial.println(F("[DBG] WUPA OK — kartu halted ke-wake-up"));
  }
  if (!rfid.PICC_ReadCardSerial()) {
    Serial.println(F("[DBG] read serial FAILED"));
    return;
  }

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

  // Pastikan antenna tetap aktif setelah delay panjang turnOn/turnOff
  rfid.PCD_AntennaOn();
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
