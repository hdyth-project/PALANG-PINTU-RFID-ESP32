#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

#define RFID_SS       5
#define RFID_RST      22
#define RFID_SCK      18
#define RFID_MISO     19
#define RFID_MOSI     23
#define SERVO_PIN     13
#define LED_MERAH     25
#define LED_HIJAU     26

// RFID
MFRC522 rfid(RFID_SS, RFID_RST);
Servo palang;

struct Kartu {
  byte uid[10];
  byte panjang;
};

const Kartu daftarKartu[] = {

  // Kartu 1 dengan UID 93:A4:7B:21
  { {0x93, 0xA4, 0x7B, 0x21}, 4 },

  //Kartu 2 dengan UID 12:23:56:78
//  { {0x12, 0x23, 0x56, 0x781}, 4 },

  
//  { {0x93, 0xA4, 0x7B, 0x21}, 4 },
};
const byte JUMLAH_KARTU =
  sizeof(daftarKartu) / sizeof(daftarKartu[0]);

bool kartuValid() {
  // Periksa semua kartu yang terdaftar
  for (byte kartu = 0; kartu < JUMLAH_KARTU; kartu++) {

    // Cek panjang UID
    if (rfid.uid.size != daftarKartu[kartu].panjang) {
      continue;
    }

    bool cocok = true;

    // Bandingkan setiap byte UID
    for (byte i = 0; i < rfid.uid.size; i++) {

      if (rfid.uid.uidByte[i] != daftarKartu[kartu].uid[i]) {
        cocok = false;
        break;
      }
    }

    // Jika semua byte cocok
    if (cocok) {
      return true;
    }
  }

  // Tidak ada UID yang cocok
  return false;
}

// MENAMPILKAN UID KARTU
void tampilkanUID() {

  Serial.print("UID Kartu: ");

  for (byte i = 0; i < rfid.uid.size; i++) {

    // Tambahkan 0 jika angka hanya 1 digit
    if (rfid.uid.uidByte[i] < 0x10) {
      Serial.print("0");
    }

    Serial.print(rfid.uid.uidByte[i], HEX);

    if (i < rfid.uid.size - 1) {
      Serial.print(" ");
    }
  }

  Serial.println();
}


// =====================================================
// MEMBUKA PALANG
// =====================================================

void bukaPalang() {

  Serial.println("KARTU VALID!");
  Serial.println("Palang membuka...");

  // Matikan merah
  digitalWrite(LED_MERAH, LOW);

  // Nyalakan hijau
  digitalWrite(LED_HIJAU, HIGH);

  // Servo membuka
  palang.write(90);

  // Palang terbuka selama 3 detik
  delay(3000);

  Serial.println("Palang menutup...");

  // Servo kembali ke posisi awal
  palang.write(0);

  // Tunggu servo sampai kembali
  delay(500);

  // Matikan hijau
  digitalWrite(LED_HIJAU, LOW);

  // Nyalakan merah
  digitalWrite(LED_MERAH, HIGH);

  Serial.println("Palang tertutup.");
  Serial.println("Silakan scan kartu...");
  Serial.println();
}


// =====================================================
// SETUP
// =====================================================

void setup() {

  // Serial Monitor
  Serial.begin(115200);

  // =================================================
  // SPI ESP32
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

  pinMode(LED_MERAH, OUTPUT);
  pinMode(LED_HIJAU, OUTPUT);


  // =================================================
  // KONDISI AWAL
  // =================================================

  palang.write(0);

  digitalWrite(LED_MERAH, HIGH);
  digitalWrite(LED_HIJAU, LOW);


  // =================================================
  // INFORMASI SERIAL MONITOR
  // =================================================

  Serial.println();
  Serial.println("================================");
  Serial.println("     SISTEM PALANG RFID ESP32");
  Serial.println("================================");

  Serial.print("Jumlah kartu terdaftar: ");
  Serial.println(JUMLAH_KARTU);

  Serial.println();

  Serial.println("Sistem siap.");
  Serial.println("Palang: TERTUTUP");
  Serial.println("LED merah: ON");
  Serial.println("Silakan scan kartu RFID...");
  Serial.println();
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  // =================================================
  // CEK APAKAH ADA KARTU BARU
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
  // CEK KARTU
  // =================================================

  if (kartuValid()) {

    // Kartu terdaftar
    bukaPalang();

  }

  else {

    // =================================================
    // KARTU TIDAK TERDAFTAR
    // =================================================

    Serial.println("KARTU TIDAK TERDAFTAR!");
    Serial.println("Palang tetap tertutup.");
    Serial.println();

    // Pastikan servo tetap tertutup
    palang.write(0);

    // LED hijau OFF
    digitalWrite(LED_HIJAU, LOW);

    // LED merah ON
    digitalWrite(LED_MERAH, HIGH);

    delay(1000);

    Serial.println("Silakan scan kartu...");
    Serial.println();
  }


  // =================================================
  // SELESAIKAN KOMUNIKASI DENGAN KARTU
  // =================================================

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(300);
}
