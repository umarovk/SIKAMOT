/*
  SIKAMOT - RFID Module Detector (SPI Version)
  Board   : Arduino Nano
  Modul   : MFRC522 8-pin SPI
  Library : MFRC522 by GithubCommunity

  TUJUAN:
    Diagnostic sketch untuk:
    1. Cek apakah modul RFID terbaca via SPI
    2. Identifikasi versi chip MFRC522 (1.0, 2.0, atau clone)
    3. Tes baca register internal
    4. Tes deteksi kartu

  WIRING: sama dengan motor_starter.ino
    SDA  → D10
    SCK  → D13
    MOSI → D11
    MISO → D12
    GND  → GND
    RST  → D9
    3.3V → 3.3V

  CARA PAKAI:
    1. Upload sketch ini ke Nano
    2. Buka Serial Monitor (baud 9600)
    3. Lihat output diagnostic
    4. Tap kartu untuk tes baca UID
*/

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN   10
#define RST_PIN  9

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  while (!Serial);   // tunggu Serial siap

  Serial.println(F("\n==========================================="));
  Serial.println(F("   SIKAMOT - RFID Module Detector (SPI)"));
  Serial.println(F("==========================================="));
  Serial.println();

  Serial.println(F("[1/4] Inisialisasi SPI..."));
  SPI.begin();
  Serial.println(F("      OK"));
  Serial.println();

  Serial.println(F("[2/4] Inisialisasi MFRC522..."));
  rfid.PCD_Init();
  delay(50);
  Serial.println(F("      OK"));
  Serial.println();

  Serial.println(F("[3/4] Cek koneksi & versi chip..."));
  identifyVersion();
  Serial.println();

  Serial.println(F("[4/4] Cek register internal..."));
  dumpKeyRegisters();
  Serial.println();

  Serial.println(F("==========================================="));
  Serial.println(F("Diagnostic selesai. Sekarang tap kartu RFID"));
  Serial.println(F("untuk tes baca UID..."));
  Serial.println(F("==========================================="));
  Serial.println();
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  Serial.println(F(">>> KARTU TERDETEKSI <<<"));

  Serial.print(F("UID  : "));
  printUID();

  Serial.print(F("Size : "));
  Serial.print(rfid.uid.size);
  Serial.println(F(" bytes"));

  Serial.print(F("SAK  : 0x"));
  if (rfid.uid.sak < 0x10) Serial.print("0");
  Serial.println(rfid.uid.sak, HEX);

  MFRC522::PICC_Type picc = rfid.PICC_GetType(rfid.uid.sak);
  Serial.print(F("Type : "));
  Serial.println(rfid.PICC_GetTypeName(picc));

  Serial.println(F("---"));

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// ============================================================
// Identifikasi versi MFRC522
// ============================================================
void identifyVersion() {
  byte v = rfid.PCD_ReadRegister(MFRC522::VersionReg);

  Serial.print(F("      VersionReg (0x37) = 0x"));
  if (v < 0x10) Serial.print("0");
  Serial.println(v, HEX);

  Serial.print(F("      => "));
  switch (v) {
    case 0x91:
      Serial.println(F("MFRC522 v1.0 (asli NXP)"));
      break;
    case 0x92:
      Serial.println(F("MFRC522 v2.0 (asli NXP)"));
      break;
    case 0x88:
      Serial.println(F("Clone FM17522 (Fudan, compatible)"));
      break;
    case 0xB2:
      Serial.println(F("Clone FM17522E (Fudan, compatible)"));
      break;
    case 0x12:
      Serial.println(F("Counterfeit chip - kemungkinan tidak reliable"));
      break;
    case 0x00:
    case 0xFF:
      Serial.println(F("⚠ KOMUNIKASI GAGAL!"));
      Serial.println(F("      Kemungkinan penyebab:"));
      Serial.println(F("      - Wiring SPI salah (SDA/SCK/MOSI/MISO)"));
      Serial.println(F("      - Power 3.3V tidak tersambung"));
      Serial.println(F("      - RST pin salah"));
      Serial.println(F("      - Modul rusak"));
      break;
    default:
      Serial.print(F("Unknown chip variant (0x"));
      Serial.print(v, HEX);
      Serial.println(F(")"));
      break;
  }
}

// ============================================================
// Dump register penting untuk diagnostic
// ============================================================
void dumpKeyRegisters() {
  Serial.println(F("      Register     Hex   Status"));
  Serial.println(F("      --------------------------"));

  printReg("CommandReg ", MFRC522::CommandReg);
  printReg("ComIEnReg  ", MFRC522::ComIEnReg);
  printReg("DivIEnReg  ", MFRC522::DivIEnReg);
  printReg("ComIrqReg  ", MFRC522::ComIrqReg);
  printReg("Status1Reg ", MFRC522::Status1Reg);
  printReg("Status2Reg ", MFRC522::Status2Reg);
  printReg("ModeReg    ", MFRC522::ModeReg);
  printReg("TxControlReg", MFRC522::TxControlReg);

  // Antenna gain
  byte gain = rfid.PCD_GetAntennaGain();
  Serial.print(F("      Antenna gain = 0x"));
  Serial.print(gain, HEX);
  Serial.print(F(" ("));
  printGainDb(gain);
  Serial.println(F(")"));
}

void printReg(const char* name, MFRC522::PCD_Register reg) {
  byte v = rfid.PCD_ReadRegister(reg);
  Serial.print(F("      "));
  Serial.print(name);
  Serial.print(F("  0x"));
  if (v < 0x10) Serial.print("0");
  Serial.println(v, HEX);
}

void printGainDb(byte gain) {
  // gain di register Bits 6:4
  byte g = (gain >> 4) & 0x07;
  const char* labels[] = {
    "18 dB", "23 dB", "18 dB", "23 dB",
    "33 dB", "38 dB", "43 dB", "48 dB (MAX)"
  };
  Serial.print(labels[g]);
}

// ============================================================
// Print UID kartu
// ============================================================
void printUID() {
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) Serial.print(" ");
  }
  Serial.println();
}
