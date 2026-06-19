# motor_starter_hwreset

Varian dari `motor_starter.ino` dengan **hardware reset MFRC522** lewat pin RST (D9).

## Kapan Pakai Versi Ini?

Pakai **dengan urutan ini**:

1. ✅ **Pertama coba `motor_starter.ino`** (soft reset pakai `PCD_Init()` saja)
2. ⚠️ Kalau modul **tetap hang** setelah tap pertama → tap kedua tidak terbaca
3. ✅ **Pindah ke `motor_starter_hwreset.ino`** ini

## Apa Bedanya?

| Aspek | motor_starter.ino | motor_starter_hwreset.ino |
|---|---|---|
| Reset method | Soft (PCD_Init via I2C) | **Hardware** (pulse pin RST) |
| Reliability | Cukup untuk kebanyakan kasus | Lebih agresif, recovery dari hang berat |
| Durasi reset | ~10-50 ms | **~100 ms** (50ms LOW + 50ms HIGH) |
| Logic motor | Sama | Sama |
| Wiring | Sama | Sama (RST sudah di D9) |

## Cara Kerja Hardware Reset

```cpp
void hardResetRFID() {
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, LOW);    // pin RST = LOW → chip reset
  delay(50);                      // tahan 50ms
  digitalWrite(RST_PIN, HIGH);    // pin RST = HIGH → chip aktif lagi
  delay(50);                      // wait stabil
  rfid.PCD_Init();                // re-init via library
}
```

Ini setara **pencabutan power MFRC522** lalu **dipasang ulang** — chip benar-benar
dari awal lagi, jadi state lama (HALT, crypto, dll) **hilang total**.

## Wiring

**TIDAK perlu wiring tambahan** — pin RST sudah ke D9 dari awal.
Wiring sama persis dengan `motor_starter/WIRING.md`.

## Test Setelah Upload

Buka Serial Monitor (9600), tes 5x tap berturut-turut:

```
Tap 1: motor HIDUP    ← seharusnya OK
Tap 2: motor MATI     ← yang sebelumnya gagal, sekarang harus OK
Tap 3: motor HIDUP    ← OK
Tap 4: motor MATI     ← OK
Tap 5: motor HIDUP    ← OK
```

Kalau **5/5 sukses** → fix berhasil, pakai versi ini untuk production.

## Catatan

- Versi ini sedikit **lebih lambat** (~100ms extra per tap untuk hardware reset)
- Tidak ada perbedaan power consumption signifikan
- Logic motor + UID whitelist identik dengan versi original
