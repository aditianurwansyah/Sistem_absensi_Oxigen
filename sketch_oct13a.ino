#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

// Definisi pin RFID untuk Wemos D1 (ESP8266)
#define SS_PIN D4  // SDA / SS
#define RST_PIN D3 // RST

MFRC522 mfrc522(SS_PIN, RST_PIN);

// Konfigurasi WiFi Anda
const char* ssid = "NAMA_WIFI_ANDA";
const char* password = "PASSWORD_WIFI_ANDA";

// Masukkan URL Web App Google Apps Script yang sudah di-deploy sebelumnya
const char* scriptURL = "MASUKKAN_URL_WEB_APP_ANDA_DISINI";

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Inisialisasi RFID
  SPI.begin();
  mfrc522.PCD_Init();
  
  // Koneksi WiFi
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Cek apakah ada kartu baru yang mendekat
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Baca serial kartu
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Ubah UID kartu menjadi format String Hexadecimal
  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidString += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    uidString += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidString.toUpperCase();
  
  Serial.print("UID Kartu Terdeteksi: ");
  Serial.println(uidString);

  // Kirim data ke Google Sheets jika terhubung ke internet
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure(); // Mengabaikan verifikasi sertifikat SSL Google (wajib untuk ESP8266)

    HTTPClient http;
    String url = String(scriptURL) + "?uid=" + uidString;

    Serial.print("Mengirim data ke Google Sheets...");
    
    if (http.begin(client, url)) {
      int httpCode = http.GET();
      if (httpCode > 0) {
        Serial.printf(" [Berhasil] HTTP Code: %d\n", httpCode);
        String payload = http.getString();
        Serial.println("Respon Server: " + payload);
      } else {
        Serial.printf(" [Gagal] Error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      Serial.println(" [Gagal] Tidak dapat terhubung ke URL.");
    }
  } else {
    Serial.println("WiFi terputus!");
  }

  // Jeda 2 detik agar kartu tidak terbaca dua kali secara beruntun
  delay(2000);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}