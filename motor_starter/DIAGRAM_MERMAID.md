# Diagram Wiring SIKAMOT (Mermaid Detail)

> Buka file ini di **GitHub web**, **VSCode (extension Markdown Preview Mermaid Support)**, atau **Obsidian** untuk render visual otomatis.

---

## 1. Overview Sistem

```mermaid
flowchart TB
    subgraph SRC["⚡ SUMBER DAYA"]
        AKI("🔋 Aki Motor 12V")
        FUSE("🔥 Fuse 30A")
        STEP("⬇️ Step-down<br/>12V → 5V")
    end

    subgraph INPUT["📥 INPUT"]
        CARD("💳 Kartu RFID<br/>13.56 MHz")
        READER("📡 RC522 I2C V1.1")
    end

    subgraph BRAIN["🧠 PEMROSESAN"]
        NANO("🤖 Arduino Nano<br/>ATmega328P")
    end

    subgraph CTRL["🔌 KONTROL"]
        REL1("🔌 Relay 2-Ch<br/>Channel 1")
        REL2("🔌 Relay 2-Ch<br/>Channel 2")
        BOSCH("⚙️ Relay Bosch 40A")
    end

    subgraph OUTPUT["🏍️ OUTPUT"]
        IGN("🔑 Ignition<br/>CDI / Koil")
        STARTER("⚡ Starter Motor")
    end

    AKI --> FUSE
    FUSE --> STEP
    FUSE -.->|"12V"| REL1
    FUSE -.->|"12V"| REL2
    FUSE -.->|"12V high-current"| BOSCH

    STEP -->|"5V"| NANO

    CARD -.->|"tap"| READER
    READER <-->|"I2C"| NANO

    NANO -->|"D7"| REL1
    NANO -->|"D8"| REL2

    REL1 -->|"NO1 → 12V"| IGN
    REL2 -->|"NO2 trigger"| BOSCH
    BOSCH -->|"87 → 12V"| STARTER

    classDef power fill:#ffe0e0,stroke:#c00,stroke-width:2px
    classDef input fill:#e0f0ff,stroke:#06c,stroke-width:2px
    classDef brain fill:#fff0e0,stroke:#f80,stroke-width:2px
    classDef ctrl fill:#e0ffe0,stroke:#080,stroke-width:2px
    classDef output fill:#ffe0f0,stroke:#c08,stroke-width:2px

    class AKI,FUSE,STEP power
    class CARD,READER input
    class NANO brain
    class REL1,REL2,BOSCH ctrl
    class IGN,STARTER output
```

---

## 2. Detail Wiring Per Pin

```mermaid
flowchart LR
    subgraph PWR["⚡ POWER (12V)"]
        direction TB
        AKI_P("Aki 12V +")
        AKI_N("Aki 12V −")
        FUSE("Fuse 30A")
        STEP_IN_P("Step-down VIN+")
        STEP_IN_N("Step-down VIN−")
        STEP_OUT_P("Step-down VOUT+ (5V)")
        STEP_OUT_N("Step-down VOUT−")
    end

    subgraph NANO["🤖 ARDUINO NANO"]
        direction TB
        N_VIN("VIN/5V")
        N_GND1("GND")
        N_33("3V3")
        N_GND2("GND")
        N_D7("D7")
        N_D8("D8")
        N_D9("D9")
        N_A4("A4 / SDA")
        N_A5("A5 / SCL")
    end

    subgraph RFID["📡 RFID RC522 I2C"]
        direction TB
        R_VCC("3.3V")
        R_GND("GND")
        R_RST("RST")
        R_SDA("SDA")
        R_SCL("SCL")
        R_IRQ("IRQ ❌")
    end

    subgraph RELMOD["🔌 RELAY 2-CHANNEL"]
        direction TB
        RM_VCC("VCC")
        RM_GND("GND")
        RM_IN1("IN1")
        RM_IN2("IN2")
        RM_COM1("COM1")
        RM_NO1("NO1")
        RM_NC1("NC1 ❌")
        RM_COM2("COM2")
        RM_NO2("NO2")
        RM_NC2("NC2 ❌")
    end

    subgraph BOSCH["⚙️ BOSCH 5-PIN 40A"]
        direction TB
        B_30("30 (COM)")
        B_87("87 (NO)")
        B_87a("87a ❌")
        B_85("85 (coil −)")
        B_86("86 (coil +)")
    end

    subgraph MOTOR["🏍️ MOTOR 12V"]
        direction TB
        IGN_WIRE("Kabel Ignition<br/>(ke CDI/koil/ECU)")
        STARTER_W("Kabel Starter<br/>(ke solenoid)")
        CHASSIS("Chassis Ground")
    end

    %% ===== POWER =====
    AKI_P -->|"merah AWG10"| FUSE
    FUSE -->|"merah AWG10"| STEP_IN_P
    AKI_N -->|"hitam AWG10"| STEP_IN_N
    STEP_OUT_P -->|"merah"| N_VIN
    STEP_OUT_N -->|"hitam"| N_GND1

    %% ===== RFID =====
    N_33 -->|"merah"| R_VCC
    N_GND2 -->|"hitam"| R_GND
    N_D9  -->|"kuning"| R_RST
    N_A4  -->|"biru"| R_SDA
    N_A5  -->|"hijau"| R_SCL

    %% ===== RELAY MODUL =====
    N_VIN  -->|"merah"| RM_VCC
    N_GND1 -->|"hitam"| RM_GND
    N_D7   -->|"putih"| RM_IN1
    N_D8   -->|"abu"| RM_IN2

    %% ===== OUTPUT IGNITION =====
    FUSE   -->|"merah AWG18"| RM_COM1
    RM_NO1 -->|"merah AWG18"| IGN_WIRE

    %% ===== OUTPUT STARTER (CASCADE) =====
    FUSE    -->|"merah AWG18"| RM_COM2
    RM_NO2  -->|"kuning AWG18"| B_86
    AKI_N   -->|"hitam"| B_85

    %% ===== BOSCH HIGH-CURRENT =====
    FUSE  -->|"⚡merah AWG10⚡"| B_30
    B_87  -->|"⚡merah AWG10⚡"| STARTER_W

    %% ===== GROUND COMMON =====
    AKI_N -.->|"chassis"| CHASSIS

    classDef pwr fill:#ffd6d6,stroke:#c00,color:#000
    classDef gnd fill:#d6d6d6,stroke:#000,color:#000
    classDef sig fill:#d6e8ff,stroke:#06c,color:#000
    classDef hicurrent fill:#ffe0a0,stroke:#e60,stroke-width:3px,color:#000
    classDef unused fill:#f5f5f5,stroke:#999,stroke-dasharray:5 5,color:#999

    class AKI_P,FUSE,STEP_IN_P,STEP_OUT_P,N_VIN,N_33,R_VCC,RM_VCC,RM_COM1,RM_COM2,B_30,B_86 pwr
    class AKI_N,STEP_IN_N,STEP_OUT_N,N_GND1,N_GND2,R_GND,RM_GND,B_85,CHASSIS gnd
    class N_D7,N_D8,N_D9,N_A4,N_A5,R_RST,R_SDA,R_SCL,RM_IN1,RM_IN2 sig
    class RM_NO1,RM_NO2,B_87,IGN_WIRE,STARTER_W hicurrent
    class R_IRQ,RM_NC1,RM_NC2,B_87a unused
```

---

## 3. Diagram Per Modul

### 3.1 RFID → Nano (Sinyal I2C)

```mermaid
flowchart LR
    subgraph NANO["Arduino Nano"]
        direction TB
        n33["3V3"]
        ngnd["GND"]
        nd9["D9"]
        na4["A4 (SDA)"]
        na5["A5 (SCL)"]
    end

    subgraph RFID["RC522 I2C V1.1"]
        direction TB
        rvcc["3.3V"]
        rgnd["GND"]
        rrst["RST"]
        rsda["SDA"]
        rscl["SCL"]
        rirq["IRQ ❌"]
    end

    n33 ==>|"🔴 merah"| rvcc
    ngnd ==>|"⚫ hitam"| rgnd
    nd9 ==>|"🟡 kuning"| rrst
    na4 ==>|"🔵 biru"| rsda
    na5 ==>|"🟢 hijau"| rscl

    style rvcc fill:#ffcccc
    style rgnd fill:#cccccc
    style rrst fill:#ffffcc
    style rsda fill:#cce0ff
    style rscl fill:#ccffcc
```

### 3.2 Relay 2-Channel → Nano (Sinyal Kontrol)

```mermaid
flowchart LR
    subgraph NANO["Arduino Nano"]
        direction TB
        n5v["5V"]
        ngnd["GND"]
        nd7["D7"]
        nd8["D8"]
    end

    subgraph REL["Relay 2-Channel 5V"]
        direction TB
        vcc["VCC"]
        gnd["GND"]
        in1["IN1"]
        in2["IN2"]
    end

    n5v ==>|"🔴 merah"| vcc
    ngnd ==>|"⚫ hitam"| gnd
    nd7 ==>|"⚪ putih (Kunci Kontak)"| in1
    nd8 ==>|"⚪ abu-abu (Starter Trigger)"| in2

    style vcc fill:#ffcccc
    style gnd fill:#cccccc
    style in1 fill:#fff0e0
    style in2 fill:#fff0e0
```

### 3.3 Relay Channel 1 → Ignition Motor

```mermaid
flowchart LR
    AKI(("🔋 Aki 12V +"))
    FUSE["🔥 Fuse 30A"]

    subgraph REL["Relay Modul Channel 1"]
        com1["COM1"]
        no1["NO1"]
        nc1["NC1 ❌"]
    end

    CDI["🔑 Kabel Ignition Motor<br/>(ke CDI / Koil / ECU)"]

    AKI ==>|"🔴 merah"| FUSE
    FUSE ==>|"🔴 merah (input)"| com1
    no1 ==>|"🔴 merah (output ke kunci kontak)"| CDI

    style com1 fill:#ffcccc
    style no1 fill:#ffe0a0,stroke-width:3px
    style nc1 fill:#f5f5f5,stroke-dasharray:5 5
```

### 3.4 Relay Channel 2 + Bosch → Starter (Cascade)

```mermaid
flowchart LR
    AKI(("🔋 Aki 12V +"))
    FUSE["🔥 Fuse 30A"]
    GND(("⚫ GND aki"))

    subgraph REL["Relay Modul Channel 2"]
        com2["COM2"]
        no2["NO2"]
    end

    subgraph BOSCH["⚙️ Bosch 40A 5-pin"]
        b30["Pin 30 (COM)"]
        b87["Pin 87 (NO)"]
        b85["Pin 85 (coil −)"]
        b86["Pin 86 (coil +)"]
    end

    STARTER["⚡ Solenoid Starter"]
    DIODE["🔻 Diode 1N4007"]

    %% Trigger path (sinyal low-current)
    AKI ==>|"🔴 merah"| FUSE
    FUSE ==>|"🔴 merah"| com2
    no2 ==>|"🟡 kuning"| b86
    GND ==>|"⚫ hitam"| b85

    %% High-current path
    FUSE ==>|"🔴 AWG 10 ⚡"| b30
    b87 ==>|"🔴 AWG 10 ⚡"| STARTER

    %% Flyback diode
    b85 -.->|"🔻 katoda ke 86"| DIODE
    DIODE -.-> b86

    style com2 fill:#ffcccc
    style no2 fill:#fff0e0
    style b30 fill:#ffe0a0,stroke-width:3px
    style b87 fill:#ffe0a0,stroke-width:3px
    style b85 fill:#cccccc
    style b86 fill:#fff0e0
    style DIODE fill:#e0e0ff,stroke-dasharray:3 3
```

---

## 4. Diagram Power Distribution

```mermaid
flowchart TB
    AKI(("🔋 AKI 12V"))
    FUSE["🔥 FUSE 30A"]

    subgraph DIST["📍 Bus 12V (setelah fuse)"]
        BUS12["+12V"]
    end

    STEP["⬇️ Step-down<br/>12V → 5V"]

    subgraph LOW["📍 Bus 5V & 3.3V (Nano)"]
        BUS5["+5V (Nano)"]
        BUS33["+3.3V (Nano)"]
        BUSGND["GND Common"]
    end

    AKI ==>|"+"| FUSE
    FUSE ==> BUS12

    BUS12 -->|"input step-down"| STEP
    STEP --> BUS5
    BUS5 -.->|"regulator Nano"| BUS33

    BUS12 -.->|"Relay COM1 (kunci kontak)"| K1["⚡ Kunci Kontak"]
    BUS12 -.->|"Relay COM2 (trigger)"| K2["⚡ Bosch Trigger"]
    BUS12 -.->|"Bosch pin 30 (starter)"| K3["⚡ Starter Output"]

    BUS5 -.->|"VCC modul relay"| K4["Relay coil"]
    BUS5 -.->|"VCC Nano"| K5["MCU"]
    BUS33 -.->|"VCC RFID"| K6["RC522"]

    AKI ==>|"−"| BUSGND
    BUSGND -.-> K7["Chassis Motor"]
    BUSGND -.-> K8["Nano GND"]
    BUSGND -.-> K9["Relay GND"]
    BUSGND -.-> K10["Bosch pin 85"]

    classDef hv fill:#ffaaaa,stroke:#c00,stroke-width:3px
    classDef lv fill:#aaccff,stroke:#06c
    classDef gnd fill:#aaaaaa,stroke:#000

    class AKI,FUSE,BUS12,K1,K2,K3 hv
    class STEP,BUS5,BUS33,K4,K5,K6 lv
    class BUSGND,K7,K8,K9,K10 gnd
```

---

## 5. Diagram State Machine (Logika Toggle)

```mermaid
stateDiagram-v2
    [*] --> OFF: Power ON

    OFF --> ValidatingON: Tap kartu
    ValidatingON --> OFF: UID tidak valid
    ValidatingON --> IgnitionON: UID valid + cooldown OK

    IgnitionON --> Cranking: tunggu 500ms
    Cranking --> MotorRunning: starter 2 detik selesai

    MotorRunning --> ValidatingOFF: Tap kartu
    ValidatingOFF --> MotorRunning: UID tidak valid
    ValidatingOFF --> OFF: UID valid + cooldown OK

    note right of OFF
        Relay1 OFF
        Relay2 OFF
        motorOn = false
    end note

    note right of IgnitionON
        Relay1 ON
        (kunci kontak hidup)
    end note

    note right of Cranking
        Relay1 ON
        Relay2 ON (2 detik)
    end note

    note right of MotorRunning
        Relay1 ON
        Relay2 OFF
        motorOn = true
    end note
```

---

## 6. Sequence Diagram (Alur Tap Kartu Valid)

```mermaid
sequenceDiagram
    actor User
    participant Card as 💳 Kartu
    participant RFID as 📡 RC522
    participant Nano as 🤖 Nano
    participant R1 as 🔌 Relay1<br/>(Kunci Kontak)
    participant R2 as 🔌 Relay2<br/>(Starter)
    participant Bosch as ⚙️ Bosch
    participant Motor as 🏍️ Motor

    Note over Nano: State: OFF

    User->>Card: Tap ke reader
    Card->>RFID: Transmisi UID
    RFID->>Nano: PICC_IsNewCardPresent()
    RFID->>Nano: PICC_ReadCardSerial()
    Nano->>Nano: readUID() → "61 FB C1 01"
    Nano->>Nano: isAuthorized() ✓

    Nano->>R1: digitalWrite(D7, LOW)
    R1->>Motor: 12V → CDI/Koil ON
    Note over Nano: delay 500ms

    Nano->>R2: digitalWrite(D8, LOW)
    R2->>Bosch: trigger pin 86
    Bosch->>Motor: 12V high-current → Starter
    Note over Nano: delay 2000ms (crank)

    Nano->>R2: digitalWrite(D8, HIGH)
    R2->>Bosch: trigger OFF
    Bosch->>Motor: Starter stop

    Note over Nano: State: ON<br/>motorOn = true

    User->>Card: Tap lagi
    Card->>RFID: Transmisi UID
    RFID->>Nano: Read UID
    Nano->>Nano: isAuthorized() ✓
    Nano->>R1: digitalWrite(D7, HIGH)
    R1->>Motor: 12V cut → Engine mati

    Note over Nano: State: OFF<br/>motorOn = false
```

---

## 7. Mapping Pin Lengkap (Tabel + Diagram)

```mermaid
flowchart TB
    subgraph NANO_PINS["🤖 Arduino Nano — Pin Mapping"]
        direction LR
        subgraph POWER["⚡ Power Pins"]
            P1["VIN/5V<br/>← step-down OUT+"]
            P2["GND<br/>→ ground bus"]
            P3["3V3<br/>→ RFID VCC"]
        end

        subgraph DIGITAL["🔢 Digital Pins"]
            D7["D7<br/>→ Relay IN1<br/>(Kunci Kontak)"]
            D8["D8<br/>→ Relay IN2<br/>(Starter Trigger)"]
            D9["D9<br/>→ RFID RST"]
        end

        subgraph ANALOG["📊 Analog Pins (I2C)"]
            A4["A4 / SDA<br/>→ RFID SDA"]
            A5["A5 / SCL<br/>→ RFID SCL"]
        end

        subgraph UNUSED["❌ Tidak Dipakai"]
            UN["D0, D1, D2, D3,<br/>D4, D5, D6, D10-D13,<br/>A0-A3, A6, A7"]
        end
    end

    style P1 fill:#ffcccc
    style P2 fill:#cccccc
    style P3 fill:#ffd6e0
    style D7 fill:#ffe0a0
    style D8 fill:#ffe0a0
    style D9 fill:#ffffcc
    style A4 fill:#cce0ff
    style A5 fill:#ccffcc
    style UN fill:#f5f5f5,stroke-dasharray:5 5,color:#999
```

---

## Legenda Warna Standar

| Warna | Fungsi |
|---|---|
| 🔴 Merah | Tegangan positif (+5V, +12V, +3.3V) |
| ⚫ Hitam | Ground (GND) / negatif aki |
| 🟡 Kuning | Sinyal trigger / RST |
| 🔵 Biru | Data I2C SDA |
| 🟢 Hijau | Clock I2C SCL |
| ⚪ Putih / Abu | Sinyal kontrol relay (IN1, IN2) |
| 🟠 Oranye | High-current (AWG 10) |

---

## Cara Render Visual

| Tool | Cara |
|---|---|
| **GitHub** | Push repo → buka file `.md` di web, auto-render |
| **VSCode** | Install extension `Markdown Preview Mermaid Support` → `Ctrl+Shift+V` |
| **Obsidian** | Buka file → mode preview |
| **Online viewer** | Copy isi blok mermaid → paste di [mermaid.live](https://mermaid.live) |
| **Export PNG** | mermaid.live → tombol `Actions` → `Download PNG/SVG` |

Untuk dapat **file gambar** (PNG/SVG yang bisa di-print), pakai mermaid.live → copy diagram → export.
