#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <Keypad.h>

// =====================================================
// PIN ESP32
// =====================================================

// ---------- RFID ----------
#define RFID_SS       5
#define RFID_RST      22

#define RFID_SCK      18
#define RFID_MISO     19
#define RFID_MOSI     23

// ---------- SERVO ----------
#define SERVO_PIN     13

// ---------- LED ----------
#define LED_MERAH     25
#define LED_HIJAU     26


// =====================================================
// PIN KEYPAD 4x4
// =====================================================

byte rowPins[4] = {
  27, 14, 16, 17
};

byte colPins[4] = {
  32, 33, 21, 4
};


// =====================================================
// LAYOUT KEYPAD
// =====================================================

char keys[4][4] = {

  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}

};


// =====================================================
// MEMBUAT OBJEK KEYPAD
// =====================================================

Keypad keypad = Keypad(
  makeKeymap(keys),
  rowPins,
  colPins,
  4,
  4
);


// =====================================================
// RFID
// =====================================================

MFRC522 rfid(
  RFID_SS,
  RFID_RST
);


// =====================================================
// SERVO
// =====================================================

Servo palang;


// =====================================================
// PIN / PASSWORD KEYPAD
// =====================================================
//
// >>> GANTI PIN DI SINI <<<
//
// Contoh:
// "2580"
// "123456"
// "9876"
//
// PIN diakhiri dengan tombol #
//
// =====================================================

const char PIN_AKSES[] = "2580";


// =====================================================
// INPUT PIN
// =====================================================

String inputPIN = "";


// =====================================================
// STRUKTUR DATA UID KARTU
// =====================================================

struct Kartu {

  byte uid[10];

  byte panjang;
};


// =====================================================
// DAFTAR UID KARTU
// =====================================================
//
// >>> GANTI UID KARTU DI SINI <<<
//
// UID 4 BYTE contoh:
//
// { {0x93, 0xA4, 0x7B, 0x21}, 4 }
//
// UID 7 BYTE contoh:
//
// { {0x04, 0x91, 0x8A, 0x32, 0x76, 0x11, 0x80}, 7 }
//
// =====================================================

const Kartu daftarKartu[] = {

  // Kartu 1
  { {0x93, 0xA4, 0x7B, 0x21}, 4 },

  // Kartu 2
  { {0x12, 0x34, 0x56, 0x78}, 4 },

  // Kartu 3
  { {0xAB, 0xCD, 0xEF, 0x01}, 4 }

};


// =====================================================
// JUMLAH KARTU
// =====================================================

const byte JUMLAH_KARTU =
  sizeof(daftarKartu) / sizeof(daftarKartu[0]);


// =====================================================
// CEK UID RFID
// =====================================================

bool kartuValid() {

  for (byte kartu = 0; kartu < JUMLAH_KARTU; kartu++) {

    // Cek panjang UID
    if (rfid.uid.size != daftarKartu[kartu].panjang) {
      continue;
    }

    bool cocok = true;

    // Bandingkan UID byte per byte
    for (byte i = 0; i < rfid.uid.size; i++) {

      if (
        rfid.uid.uidByte[i] !=
        daftarKartu[kartu].uid[i]
      ) {

        cocok = false;
        break;
      }
    }

    // Jika UID cocok
    if (cocok) {
      return true;
    }
  }

  // Tidak ditemukan UID yang cocok
  return false;
}


// =====================================================
// MENAMPILKAN UID RFID
// =====================================================

void tampilkanUID() {

  Serial.print("UID Kartu: ");

  for (byte i = 0; i < rfid.uid.size; i++) {

    if (rfid.uid.uidByte[i] < 0x10) {
      Serial.print("0");
    }

    Serial.print(
      rfid.uid.uidByte[i],
      HEX
    );

    if (i < rfid.uid.size - 1) {
      Serial.print(" ");
    }
  }

  Serial.println();
}


// =====================================================
// MEMBUKA PALANG
// =====================================================

void bukaPalang(const char* metode) {

  Serial.println();
  Serial.println("==============================");
  Serial.println("      AKSES DITERIMA");
  Serial.println("==============================");

  Serial.print("Metode: ");
  Serial.println(metode);

  Serial.println("Palang membuka...");

  // LED hijau ON
  digitalWrite(LED_MERAH, LOW);
  digitalWrite(LED_HIJAU, HIGH);

  // Servo membuka
  palang.write(90);

  // Tunggu 3 detik
  delay(3000);

  Serial.println("Palang menutup...");

  // Servo kembali
  palang.write(0);

  // Tunggu servo
  delay(500);

  // LED hijau OFF
  digitalWrite(LED_HIJAU, LOW);

  // LED merah ON
  digitalWrite(LED_MERAH, HIGH);

  Serial.println("Palang tertutup.");
  Serial.println();
}


// =====================================================
// AKSES DITOLAK
// =====================================================

void aksesDitolak(const char* metode) {

  Serial.println();
  Serial.println("==============================");
  Serial.println("       AKSES DITOLAK");
  Serial.println("==============================");

  Serial.print("Metode: ");
  Serial.println(metode);

  Serial.println("Palang tetap tertutup.");

  // Pastikan servo tertutup
  palang.write(0);

  // LED hijau OFF
  digitalWrite(LED_HIJAU, LOW);

  // LED merah ON
  digitalWrite(LED_MERAH, HIGH);

  delay(1000);

  Serial.println();
}


// =====================================================
// PROSES KEYPAD
// =====================================================

void prosesKeypad() {

  char tombol = keypad.getKey();

  // Tidak ada tombol ditekan
  if (!tombol) {
    return;
  }


  // =================================================
  // TOMBOL *
  // HAPUS INPUT
  // =================================================

  if (tombol == '*') {

    inputPIN = "";

    Serial.println();
    Serial.println("Input PIN dihapus.");
    Serial.println("Masukkan PIN:");

    return;
  }


  // =================================================
  // TOMBOL #
  // KONFIRMASI PIN
  // =================================================

  if (tombol == '#') {

    Serial.println();

    Serial.print("PIN dimasukkan: ");

    // Jangan tampilkan PIN asli
    for (unsigned int i = 0; i < inputPIN.length(); i++) {
      Serial.print("*");
    }

    Serial.println();


    // -----------------------------------------------
    // CEK PIN
    // -----------------------------------------------

    if (inputPIN == PIN_AKSES) {

      Serial.println("PIN BENAR!");

      // Kosongkan input
      inputPIN = "";

      // Buka palang
      bukaPalang("KEYPAD");

    }

    else {

      Serial.println("PIN SALAH!");

      // Kosongkan input
      inputPIN = "";

      // Tolak akses
      aksesDitolak("KEYPAD");
    }

    return;
  }


  // =================================================
  // TOMBOL ANGKA
  // =================================================

  if (
    tombol >= '0' &&
    tombol <= '9'
  ) {

    // Batasi panjang PIN
    if (inputPIN.length() < 10) {

      inputPIN += tombol;

      Serial.print("*");
    }
  }
}


// =====================================================
// SETUP
// =====================================================

void setup() {

  // =================================================
  // SERIAL
  // =================================================

  Serial.begin(115200);

  delay(500);


  // =================================================
  // SPI RFID
  // =================================================

  SPI.begin(
    RFID_SCK,
    RFID_MISO,
    RFID_MOSI,
    RFID_SS
  );


  // =================================================
  // RFID
  // =================================================

  rfid.PCD_Init();

  delay(100);


  // =================================================
  // SERVO
  // =================================================

  palang.setPeriodHertz(50);

  palang.attach(
    SERVO_PIN,
    500,
    2400
  );


  // =================================================
  // LED
  // =================================================

  pinMode(
    LED_MERAH,
    OUTPUT
  );

  pinMode(
    LED_HIJAU,
    OUTPUT
  );


  // =================================================
  // KONDISI AWAL
  // =================================================

  palang.write(0);

  digitalWrite(
    LED_MERAH,
    HIGH
  );

  digitalWrite(
    LED_HIJAU,
    LOW
  );


  // =================================================
  // INFORMASI SERIAL MONITOR
  // =================================================

  Serial.println();
  Serial.println("================================");
  Serial.println("    PALANG PINTU OTOMATIS");
  Serial.println("       ESP32 + RFID + KEYPAD");
  Serial.println("================================");

  Serial.println();

  Serial.print(
    "Jumlah kartu terdaftar: "
  );

  Serial.println(
    JUMLAH_KARTU
  );

  Serial.println();

  Serial.println("Sistem siap.");
  Serial.println();
  Serial.println("Metode akses:");
  Serial.println("1. Scan kartu RFID");
  Serial.println("2. Masukkan PIN keypad");
  Serial.println();

  Serial.println("Silakan scan kartu");
  Serial.println("atau masukkan PIN...");
  Serial.println();
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  // =================================================
  // PROSES KEYPAD
  // =================================================

  prosesKeypad();


  // =================================================
  // CEK KARTU RFID
  // =================================================

  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }


  // =================================================
  // BACA KARTU
  // =================================================

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }


  // =================================================
  // TAMPILKAN UID
  // =================================================

  tampilkanUID();


  // =================================================
  // CEK UID
  // =================================================

  if (kartuValid()) {

    // -----------------------------------------------
    // RFID VALID
    // -----------------------------------------------

    Serial.println("Kartu terdaftar.");

    bukaPalang("RFID");

  }

  else {

    // -----------------------------------------------
    // RFID TIDAK TERDAFTAR
    // -----------------------------------------------

    Serial.println(
      "Kartu tidak terdaftar."
    );

    aksesDitolak("RFID");
  }


  // =================================================
  // SELESAIKAN KOMUNIKASI RFID
  // =================================================

  rfid.PICC_HaltA();

  rfid.PCD_StopCrypto1();

  delay(300);
}
