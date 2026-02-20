
#include <WiFi.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <time.h>


// Konfigurasi WiFi
const char* ssid = "my wifi";  // Ganti dengan SSID WiFi Anda
const char* password = "1234abcd";  // Ganti dengan password WiFi


// Konfigurasi Google Apps Script (ganti dengan URL deploy Anda)
const char* scriptURL = "PUT URL HERE";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 3600;   // WIB (UTC+7/8, sesuaikan)
const int daylightOffset_sec = 0;

// Pin ESP32
#define SS_PIN 5      // MFRC522 SDA
#define RST_PIN 4     // MFRC522 RST
#define BUZZER_PIN 14 // Buzzer (logika terbalik: LOW nyala)
#define SERVO_PIN 15  // Servo
#define LED_GREEN 12  // LED Hijau
#define LED_RED 13    // LED Merah
#define LCD_SDA 21    // LCD SDA
#define LCD_SCL 22    // LCD SCL


// Inisialisasi komponen
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Alamat LCD I2C, sesuaikan jika perlu
WebServer server(80);


// Struktur data UID dan role
struct User {
String uid;
String name;
String role;  // "siswa", "guru_mapel", "wali_kelas", "admin"
};


User users[] = {
{"B5572E46", "Zhello", "siswa"},
{"A5D6EE46", "Meydinnah", "siswa"},
{"A57A9B46", "Gloria", "siswa"},
{"B59F8C46", "Denold", "siswa"},
{"A5636146", "Pak Rido", "guru_mapel"},
{"A5A7E046", "Pak Imanuel", "guru_mapel"},
{"B5774546", "Pak Nara", "guru_mapel"},
{"714E6E6", "Ibu Rita", "guru_mapel"}
// Tambah UID wali kelas dan admin nanti via web
};
int numUsers = sizeof(users) / sizeof(users[0]);


// Logs (array sederhana, max 100 entries)
struct LogEntry {
String uid;
String name;
String role;
String status;  // "granted" atau "denied"
String timestamp;
};
LogEntry logs[100];
int logIndex = 0;


// Fungsi untuk cek UID
int checkUID(String uid) {
for (int i = 0; i < numUsers; i++) {
if (users[i].uid == uid) return i;
}
return -1;
}


// Fungsi kirim data ke Google Sheets
void sendToSheets(String uid, String name, String role, String status) {
if (WiFi.status() == WL_CONNECTED) {
HTTPClient http;
http.begin(scriptURL);
http.addHeader("Content-Type", "application/json");
String payload = "{";
payload += "\"uid\":\"" + uid + "\",";
payload += "\"name\":\"" + name + "\",";
payload += "\"role\":\"" + role + "\",";
payload += "\"status\":\"" + status + "\"";
payload += "}";
int httpResponseCode = http.POST(payload);
if (httpResponseCode > 0) {
Serial.println("Data dikirim ke Sheets: " + payload);
} else {
Serial.println("Error kirim ke Sheets: " + String(httpResponseCode));
}
http.end();
}
}


// Fungsi untuk update LCD (baru)
void updateLCD(String line1, String line2 = "") {
lcd.clear();
lcd.setCursor(0, 0);
lcd.print(line1);
if (line2 != "") {
lcd.setCursor(0, 1);
lcd.print(line2);
}
}

// Fungsi untuk ubah waktu (Time)
String getTimeNow() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "NTP Error";
  }
  char buffer[25];
  strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", &timeinfo);
  return String(buffer);
}

// Fungsi aksi hardware
void handleAccess(int userIndex, String uid) {
  if (userIndex != -1) {
    // ===== GRANTED =====
    updateLCD("Welcome", users[userIndex].name);

    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
    delay(120);
    digitalWrite(BUZZER_PIN, HIGH);

    delay(300);

    servo.write(90);   // buka
    delay(1500);
    servo.write(0);    // tutup

    digitalWrite(LED_GREEN, LOW);

    Serial.println("Access Granted: " + users[userIndex].name + " (" + uid + ")");
    sendToSheets(uid, users[userIndex].name, users[userIndex].role, "granted");

    logs[logIndex] = {
    uid,
    users[userIndex].name,
    users[userIndex].role,
    "granted",
    getTimeNow()
    };
    
    saveLogToSPIFFS(
   uid + "," +
   users[userIndex].name + "," +
   users[userIndex].role + ",granted," +
   getTimeNow()
   );

   logIndex = (logIndex + 1) % 100;

} else {
// ===== DENIED =====
updateLCD("Access Denied", "UID Unknown");

for (int i = 0; i < 3; i++) {
   digitalWrite(BUZZER_PIN, LOW);
   delay(120);
   digitalWrite(BUZZER_PIN, HIGH);
   delay(100);
   }

   digitalWrite(LED_RED, HIGH);
   delay(1000);
   digitalWrite(LED_RED, LOW);

   Serial.println("Access Denied: UID " + uid);
   sendToSheets(uid, "Unknown", "unknown", "denied");

// SIMPAN KE ARRAY
   logs[logIndex] = {
     uid,
     "Unknown",
     "unknown",
     "denied",
     getTimeNow()
    };

// SIMPAN KE SPIFFS
   saveLogToSPIFFS(
    uid + ",Unknown,unknown,denied," + getTimeNow()
   );

// NAIKKAN INDEX
   logIndex = (logIndex + 1) % 100;
  }
}
void saveLogToSPIFFS(String data) {
  File file = SPIFFS.open("/logs.txt", FILE_APPEND);
  if (!file) {
    Serial.println("Gagal simpan SPIFFS");
    return;
  }
  file.println(data);
  file.close();
}

void loadLogFromSPIFFS() {
  File file = SPIFFS.open("/logs.txt");
  if (!file) return;

  logIndex = 0;

  while (file.available() && logIndex < 100) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line == "") continue;

    int p1 = line.indexOf(',');
    int p2 = line.indexOf(',', p1 + 1);
    int p3 = line.indexOf(',', p2 + 1);
    int p4 = line.indexOf(',', p3 + 1);

    logs[logIndex].uid = line.substring(0, p1);
    logs[logIndex].name = line.substring(p1 + 1, p2);
    logs[logIndex].role = line.substring(p2 + 1, p3);
    logs[logIndex].status = line.substring(p3 + 1, p4);
    logs[logIndex].timestamp = line.substring(p4 + 1);

    logIndex++;
  }

  file.close();
}

void handleClearData() {
  SPIFFS.remove("/logs.txt");   // hapus file log
  logIndex = 0;                 // kosongkan array RAM
  server.send(200, "text/plain", "Data cleared");
}

void setup() {
Serial.begin(115200);
SPI.begin();
mfrc522.PCD_Init();
servo.attach(SERVO_PIN);
servo.write(0);  // Pintu tertutup awal
pinMode(BUZZER_PIN, OUTPUT);
digitalWrite(BUZZER_PIN, HIGH);  // Buzzer mati awal
pinMode(LED_GREEN, OUTPUT);
pinMode(LED_RED, OUTPUT);
lcd.init();
lcd.backlight();
updateLCD("System Booting...", "Please Wait");  // Pesan baru


// Connect WiFi
WiFi.begin(ssid, password);
int attempts = 0;
while (WiFi.status() != WL_CONNECTED && attempts < 10) {
delay(1000);
attempts++;
updateLCD("Connecting WiFi...", "Attempt " + String(attempts));  // Pesan baru
Serial.println("Connecting to WiFi...");
}
if (WiFi.status() == WL_CONNECTED) {
updateLCD("WiFi Connected", "IP: " + WiFi.localIP().toString().substring(0, 16));  // Pesan baru
Serial.println("WiFi Connected");
Serial.println("IP Address: " + WiFi.localIP().toString());

configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
} else {
updateLCD("WiFi Failed", "Check Config");  // Pesan baru
Serial.println("WiFi Connection Failed");
}
delay(2000);  // Tunggu sebentar
updateLCD("System Ready", "Tap RFID Card");  // Pesan baru


// Inisialisasi SPIFFS untuk web
if (!SPIFFS.begin(true)) {
Serial.println("SPIFFS Mount Failed");
return;

loadLogFromSPIFFS();

}


// Setup web server (gabungkan dengan dashboard_web.ino di sini)
server.on("/", handleRoot);
server.on("/adduser", handleAddUser);
server.on("/edituser", handleEditUser);
server.on("/deleteuser", handleDeleteUser);
server.on("/logs", handleRoot);
server.on("/cleardata", handleClearData);
server.on("/clear_siswa", handleClearSiswa);
server.on("/clear_guru", handleClearGuru);
// ... (lihat bagian dashboard_web.ino)


server.begin();
Serial.println("Web Server Started");
}


void loop() {
server.handleClient();  // Handle web requests


// Baca RFID
if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
updateLCD("Reading Card...", "");  // Pesan baru
String uid = "";
for (byte i = 0; i < mfrc522.uid.size; i++) {
uid += String(mfrc522.uid.uidByte[i], HEX);
}
uid.toUpperCase();
Serial.println("Tap RFID: UID " + uid);
int userIndex = checkUID(uid);
handleAccess(userIndex, uid);
mfrc522.PICC_HaltA();
}
}


