/*
  SIKAMOT - Simulasi Starter Motor dengan LED
  Board   : Arduino Nano
  Modul   : RC522 I2C V1.1 (chip MFRC522)
  Library : MFRC522_I2C by arozcan

  Tujuan:
    Sebelum pasang ke motor asli, simulasikan dulu cascade
    relay modul → Bosch → output, dengan LED sebagai pengganti
    kunci kontak dan tombol starter.

  CARA KERJA SIMULASI (sama dengan motor_starter.ino):
    - Tap kartu valid (pertama) →
        LED KUNING (ignition) NYALA → tunggu 500ms →
        LED MERAH (starter) NYALA 2 detik → mati →
        LED KUNING tetap nyala (motor "hidup")
    - Tap kartu valid (kedua) →
        Semua LED MATI ("motor mati")
    - Tap kartu salah →
        Ditolak, state tidak berubah

  WIRING — TIDAK BERUBAH dari motor_starter.ino:
    - RFID 6 pin → Nano (3.3V, GND, RST=D9, SCL=A5, SDA=A4)
    - Relay Modul → Nano (VCC=5V, GND, IN1=D7, IN2=D8)
    - Relay Modul output → Bosch coil (NO1→Bosch#1 pin86, NO2→Bosch#2 pin86)
    - Bosch pin 85 → GND aki

  WIRING TAMBAHAN UNTUK SIMULASI LED:
    Supply 12V (dari aki motor atau adapter 12V):
      +12V → Bosch #1 pin 30
      +12V → Bosch #2 pin 30

    LED Kuning (Ignition):
      Bosch #1 pin 87 → Resistor 1kΩ → LED Kuning (+) → LED (−) → GND

    LED Merah (Starter):
      Bosch #2 pin 87 → Resistor 1kΩ → LED Merah (+) → LED (−) → GND

  CATATAN:
    - Resistor 1kΩ untuk LED di 12V (R = (12-2)/0.02 ≈ 500Ω, 1kΩ aman)
    - Kalau pakai supply 5V, ganti resistor jadi 220Ω
    - Coil Bosch BUTUH 12V untuk aktif — tidak bisa pakai 5V
*/

#include <Wire.h>
#include <MFRC522_I2C.h>

// ===================== PIN =====================
#define RFID_ADDR     0x28
#define RST_PIN       9
#define IGNITION_PIN  7   // Relay 1 - trigger Bosch #1 (LED kuning)
#define STARTER_PIN   8   // Relay 2 - trigger Bosch #2 (LED merah)

// ===================== KONFIGURASI =====================
#define RELAY_ON      LOW
#define RELAY_OFF     HIGH

const unsigned long STARTER_DELAY    = 500;
const unsigned long STARTER_DURATION = 2000;
const unsigned long TAP_COOLDOWN     = 2000;

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

  Serial.println(F("=== SIKAMOT - SIMULASI STARTER (LED) ==="));
  Serial.println(F("Status: OFF (semua LED mati)"));
  Serial.println(F("Tap kartu untuk simulasi hidup/mati motor"));
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

  // Re-init RFID setelah delay panjang supaya bisa baca tap berikutnya
  rfid.PCD_Init();
}

void turnOnMotor() {
  Serial.println(F("[OK] AKSES DITERIMA"));
  Serial.println(F(">>> LED KUNING (Kunci Kontak) ON"));
  digitalWrite(IGNITION_PIN, RELAY_ON);

  delay(STARTER_DELAY);

  Serial.println(F(">>> LED MERAH (Starter) ON 2 detik..."));
  digitalWrite(STARTER_PIN, RELAY_ON);
  delay(STARTER_DURATION);
  digitalWrite(STARTER_PIN, RELAY_OFF);

  Serial.println(F(">>> LED MERAH OFF, LED KUNING tetap ON"));
  Serial.println(F("Status: ON (motor 'hidup')"));
  Serial.println(F("Tap kartu untuk mematikan"));
  Serial.println(F("---"));

  motorOn = true;
}

void turnOffMotor() {
  Serial.println(F("[OK] AKSES DITERIMA"));
  Serial.println(F(">>> SEMUA LED OFF (motor 'mati')"));
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
