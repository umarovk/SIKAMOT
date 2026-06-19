# Panduan Menjalankan RFID I2C + Relay (Hardware)

Sketch: `rfid_relay_i2c.ino`
Board: Arduino Nano
Modul RFID: NET-0061 / RC522 I2C V1.1 (chip MFRC522)

---

## 1. Komponen yang Dibutuhkan

- Arduino Nano + kabel USB
- Modul RFID RC522 I2C (6 pin: 3.3V, GND, RST, SCL, SDA, IRQ)
- Kartu / keychain RFID 13.56 MHz
- Modul Relay 1-channel 5V
- LED (simulasi DC Motor)
- Resistor 220Ω
- Breadboard + kabel jumper

---

## 2. Wiring

### RFID NET-0061 → Nano

| Pin RFID | Pin Nano |
|---|---|
| 3.3V | 3.3V |
| GND | GND |
| RST | D9 |
| SCL | A5 |
| SDA | A4 |
| IRQ | (kosong) |

> Jangan sambung VCC RFID ke 5V — modul rusak.

### Relay → Nano

| Pin Relay | Pin Nano |
|---|---|
| VCC | 5V |
| GND | GND |
| IN | D7 |

### Relay Output → LED

| Dari | Ke | Komponen |
|---|---|---|
| Nano 5V | COM | — |
| NO | Anoda LED (+) | Resistor 220Ω di antaranya |
| Katoda LED (−) | GND Nano | — |

NC dibiarkan kosong.

---

## 3. Install Library

Hanya butuh **1 library** (selain `Wire.h` yang bawaan IDE):

- **MFRC522_I2C** by arozcan
- URL: https://github.com/arozcan/MFRC522-I2C-Library

### Cara install:
1. Download file `.zip` dari GitHub (`Code` → `Download ZIP`)
2. Arduino IDE → `Sketch` → `Include Library` → `Add .ZIP Library...`
3. Pilih file ZIP yang baru di-download
4. Verifikasi: `Sketch` → `Include Library` → cari `MFRC522_I2C` di daftar

> Catatan: Library MFRC522 versi SPI (by GithubCommunity) **harus di-uninstall** kalau sebelumnya pernah dipasang, karena bentrok nama class `MFRC522`.

---

## 4. Setting Arduino IDE

1. **Board**: `Tools` → `Board` → `Arduino AVR Boards` → `Arduino Nano`
2. **Processor**: `Tools` → `Processor` → `ATmega328P (Old Bootloader)`
   *(Penting untuk Nano clone — kalau pakai bootloader baru, upload gagal `not in sync`)*
3. **Port**: `Tools` → `Port` → pilih COM tempat Nano terdeteksi (biasanya `COM3`)

---

## 5. Verifikasi Modul Terdeteksi (Optional)

Sebelum upload sketch utama, cek dulu alamat I2C modul pakai sketch scanner:

1. Buka file: `i2c_scanner/i2c_scanner.ino`
2. Klik **Upload**
3. Buka **Serial Monitor** (`Ctrl + Shift + M`), baud `9600`
4. Output yang diharapkan:
   ```
   === I2C Scanner ===
   Scanning...
   Device ditemukan di alamat 0x28
   Total perangkat: 1
   ```
5. Catat alamatnya (umumnya `0x28`)

Kalau alamat **bukan** `0x28`, edit konstanta `RFID_ADDR` di `rfid_relay_i2c.ino`:
```cpp
#define RFID_ADDR  0x3C   // sesuaikan dengan hasil scan
```

---

## 6. Compile Sketch Utama

1. Buka file: `rfid_relay_i2c.ino`
2. Klik tombol **Verify** (✓ di pojok kiri atas)
3. Tunggu sampai muncul `Done compiling.` di status bar
4. Pesan storage yang wajar:
   ```
   Sketch uses ~7000 bytes (~22%) of program storage space. Maximum is 30720 bytes.
   ```

### Kalau Error

| Pesan Error | Penyebab | Solusi |
|---|---|---|
| `MFRC522_I2C.h: No such file` | Library belum keinstall | Install ulang via ZIP |
| `MFRC522_I2C does not name a type` | Salah ketik class — harus `MFRC522` saja | Sudah benar di file ini |
| `multiple definition of MFRC522` | Ada library SPI MFRC522 ikut keinstall | Uninstall library SPI-nya |

---

## 7. Upload ke Nano

1. Pastikan Nano sudah terhubung & port benar
2. Klik tombol **Upload** (→ panah kanan)
3. Tunggu sampai muncul `Done uploading.`
4. LED `TX/RX` di Nano akan berkedip cepat saat proses upload

### Kalau Error `not in sync` / `programmer not responding`

- Pastikan **Processor: ATmega328P (Old Bootloader)** sudah dipilih
- Tutup Serial Monitor (kalau lagi terbuka)
- Cabut & colok ulang USB Nano
- Trik: tekan tombol RESET di Nano persis saat tulisan `Uploading...` muncul

---

## 8. Cek UID Kartu

Setelah upload sukses, kita perlu tahu UID kartu RFID kamu supaya bisa dimasukkan ke whitelist.

### Langkah:

1. Buka **Serial Monitor** (`Ctrl + Shift + M`)
2. Set baud rate ke **`9600`** (dropdown kanan bawah)
3. Output awal yang diharapkan:
   ```
   === RFID I2C Access Control ===
   Tap kartu ke reader...
   ```
4. Dekatkan kartu / keychain ke modul RFID (jarak < 3 cm)
5. UID akan muncul, misalnya:
   ```
   UID terbaca: 8A 3F 21 6B
   AKSES DITOLAK
   ```
6. **Catat UID-nya** — ini ID unik kartu kamu

> Karena UID-nya belum ada di whitelist, status awal selalu `AKSES DITOLAK`. Itu normal — kita akan tambahkan UID-nya di langkah berikutnya.

### Kalau Tidak Ada Respon Saat Tap

- Cek pin RST sudah ke D9
- Coba dekatkan kartu lebih dekat ke koil RFID (sisi tembaga)
- Cek wiring SDA(A4) dan SCL(A5) tidak ketuker
- Pastikan kartu jenis 13.56 MHz (Mifare), bukan 125 kHz

---

## 9. Daftarkan UID ke Whitelist

1. Buka file `rfid_relay_i2c.ino` di Arduino IDE
2. Cari bagian:
   ```cpp
   const String allowedUIDs[] = {
     "DE AD BE EF",
     "A1 B2 C3 D4"
   };
   ```
3. Ganti dengan UID kartu kamu (yang dicatat di langkah 8). Contoh:
   ```cpp
   const String allowedUIDs[] = {
     "8A 3F 21 6B",    // kartu utama
     "12 34 56 78"     // keychain cadangan
   };
   ```
4. Format harus persis: **huruf besar**, **dipisah spasi**, di dalam tanda kutip
5. **Upload ulang** sketch ke Nano

---

## 10. Tes Akhir

1. Tap kartu yang **terdaftar** → Serial Monitor:
   ```
   UID terbaca: 8A 3F 21 6B
   AKSES DITERIMA — Motor/LED ON
   ```
   → LED nyala selama 5 detik → mati otomatis
2. Tap kartu yang **tidak terdaftar** → Serial Monitor:
   ```
   UID terbaca: FF FF FF FF
   AKSES DITOLAK
   ```
   → LED tetap mati

Sistem siap dipakai.

---

## Konfigurasi yang Bisa Diubah

| Konstanta | Default | Fungsi |
|---|---|---|
| `RFID_ADDR` | `0x28` | Alamat I2C modul (hasil scan) |
| `RST_PIN` | `9` | Pin reset RFID |
| `RELAY_PIN` | `7` | Pin sinyal ke relay |
| `RELAY_ON` | `LOW` | Balik ke `HIGH` jika relay active-HIGH |
| `ON_DURATION` | `5000` | Durasi LED/motor menyala (ms) |

---

## Troubleshooting Cepat

| Gejala | Solusi |
|---|---|
| Serial Monitor diam saja | Cek baud rate `9600`, cek board terdeteksi di Device Manager |
| LED nyala terus tanpa tap kartu | Balik `RELAY_ON`/`RELAY_OFF` di kode |
| UID muncul `00 00 00 00` | Kartu nggak kompatibel (mungkin 125 kHz, bukan 13.56 MHz) |
| Modul tidak terdeteksi scanner | Cek wiring 3.3V, SDA→A4, SCL→A5 |
| Tap kartu kadang respon kadang tidak | Power supply lemah, coba kabel USB lain |
