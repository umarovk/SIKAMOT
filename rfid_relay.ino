/*
  Sistem Keamanan Motor — RFID + Fingerprint + Relay
  Board   : Arduino Nano
  Library : MFRC522 (by GithubCommunity)
            Adafruit_Fingerprint (hanya untuk hardware asli)

  -------- WIRING (sama untuk Wokwi & Hardware) --------
  RFID-RC522 → Nano
    SDA  → D10
    SCK  → D13
    MOSI → D11
    MISO → D12
    RST  → D9
    GND  → GND
    3.3V → 3.3V

  Relay → Nano
    VCC → 5V
    GND → GND
    IN  → D7

  Relay Output → LED
    Nano 5V → COM
    NO      → Resistor 220Ω → LED(+)
    LED(-)  → GND

  -------- WIRING FINGERPRINT --------
  [WOKWI - simulasi pakai 2 push button]
    Button "Valid Finger"  : 1 kaki → D2,  kaki lain → GND
    Button "Invalid Finger": 1 kaki → D3,  kaki lain → GND
    (pakai INPUT_PULLUP, jadi tidak perlu resistor)

  [HARDWARE ASLI - sensor AS608 / R307]
    VCC (merah)   → 5V    (atau 3.3V untuk modul tertentu, cek datasheet)
    GND (hitam)   → GND
    TX  (kuning)  → D2    (RX SoftwareSerial Nano)
    RX  (putih)   → D3    (TX SoftwareSerial Nano)
    Untuk pakai mode hardware: hapus baris "#define SIMULATE_FINGERPRINT"
*/

#define SIMULATE_FINGERPRINT   // <- comment baris ini kalau pakai sensor asli

#include <SPI.h>
#include <MFRC522.h>

#ifndef SIMULATE_FINGERPRINT
  #include <SoftwareSerial.h>
  #include <Adafruit_Fingerprint.h>
#endif

// ===================== PIN =====================
#define SS_PIN          10
#define RST_PIN         9
#define RELAY_PIN       7
#define FP_VALID_PIN    2    // Wokwi: tombol valid  | Hardware: RX SoftwareSerial
#define FP_INVALID_PIN  3    // Wokwi: tombol salah  | Hardware: TX SoftwareSerial

// ===================== KONFIGURASI =====================
#define RELAY_ON        LOW   // relay active-LOW (paling umum)
#define RELAY_OFF       HIGH

const unsigned long FP_TIMEOUT  = 10000; // 10 detik untuk scan jari
const unsigned long ON_DURATION = 5000;  // 5 detik motor/LED nyala

// UID kartu RFID yang diizinkan (Wokwi default: "DE AD BE EF")
const String allowedUIDs[] = {
  "DE AD BE EF",
  "A1 B2 C3 D4"
};
const int allowedCount = sizeof(allowedUIDs) / sizeof(allowedUIDs[0]);

// ID sidik jari yang diizinkan (hanya dipakai mode hardware)
const int allowedFingerIDs[] = { 1, 2, 3 };
const int allowedFingerCount = sizeof(allowedFingerIDs) / sizeof(allowedFingerIDs[0]);

// ===================== OBJEK =====================
MFRC522 rfid(SS_PIN, RST_PIN);

#ifndef SIMULATE_FINGERPRINT
  SoftwareSerial fpSerial(FP_VALID_PIN, FP_INVALID_PIN); // RX, TX
  Adafruit_Fingerprint finger(&fpSerial);
#endif

// ===================== SETUP =====================
void setup() {
  Serial.begin(9600);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);

#ifdef SIMULATE_FINGERPRINT
  pinMode(FP_VALID_PIN, INPUT_PULLUP);
  pinMode(FP_INVALID_PIN, INPUT_PULLUP);
#else
  finger.begin(57600);
  if (!finger.verifyPassword()) {
    Serial.println(F("[!] Sensor fingerprint tidak terdeteksi"));
  }
#endif

  SPI.begin();
  rfid.PCD_Init();

  Serial.println(F("=== Sistem Keamanan Motor ==="));
  Serial.println(F("Langkah 1: Tap kartu RFID"));
}

// ===================== LOOP =====================
void loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial())   return;

  String uid = readUID();
  Serial.print(F("UID: "));
  Serial.println(uid);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  if (!isAuthorizedUID(uid)) {
    Serial.println(F("[X] Kartu tidak terdaftar"));
    Serial.println(F("---"));
    return;
  }

  Serial.println(F("[OK] Kartu valid"));
  Serial.println(F("Langkah 2: Letakkan jari (10 detik)..."));

  if (waitForFingerprint()) {
    Serial.println(F("[OK] Sidik jari valid — MOTOR ON"));
    digitalWrite(RELAY_PIN, RELAY_ON);
    delay(ON_DURATION);
    digitalWrite(RELAY_PIN, RELAY_OFF);
    Serial.println(F("MOTOR OFF"));
  } else {
    Serial.println(F("[X] Sidik jari salah / timeout"));
  }

  Serial.println(F("---"));
  Serial.println(F("Tap kartu lagi untuk mulai"));
}

// ===================== FINGERPRINT =====================
bool waitForFingerprint() {
  unsigned long start = millis();

#ifdef SIMULATE_FINGERPRINT
  while (millis() - start < FP_TIMEOUT) {
    if (digitalRead(FP_VALID_PIN) == LOW) {
      delay(50); // debounce
      while (digitalRead(FP_VALID_PIN) == LOW); // tunggu lepas
      return true;
    }
    if (digitalRead(FP_INVALID_PIN) == LOW) {
      delay(50);
      while (digitalRead(FP_INVALID_PIN) == LOW);
      return false;
    }
  }
  return false;

#else
  while (millis() - start < FP_TIMEOUT) {
    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK) continue;
    if (finger.image2Tz() != FINGERPRINT_OK) continue;
    if (finger.fingerSearch() != FINGERPRINT_OK) return false;

    int id = finger.fingerID;
    Serial.print(F("ID terdeteksi: "));
    Serial.println(id);
    for (int i = 0; i < allowedFingerCount; i++) {
      if (id == allowedFingerIDs[i]) return true;
    }
    return false;
  }
  return false;
#endif
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

bool isAuthorizedUID(const String &uid) {
  for (int i = 0; i < allowedCount; i++) {
    if (uid == allowedUIDs[i]) return true;
  }
  return false;
}
