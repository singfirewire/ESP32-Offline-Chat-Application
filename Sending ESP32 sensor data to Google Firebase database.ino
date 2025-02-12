#include <WiFi.h>
#include <FirebaseESP32.h>
#include <time.h>
#include <SPIFFS.h>

#define WIFI_SSID "101-IOT"
#define WIFI_PASSWORD "10101010"
#define SWITCH_PIN 22

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 25200;
const int daylightOffset_sec = 0;
const char* PENDING_FILE = "/pending.txt";

void saveToSPIFFS(String data) {
 File file = SPIFFS.open(PENDING_FILE, FILE_APPEND);
 if(file) {
   file.println(data);
   file.close();
 }
}

void uploadPendingData() {
 if(!SPIFFS.exists(PENDING_FILE)) return;
 
 File file = SPIFFS.open(PENDING_FILE, FILE_READ);
 if(!file) return;
 
 while(file.available()) {
   String line = file.readStringUntil('\n');
   if(Firebase.pushString(fbdo, "/button_press", line)) {
     Serial.println("อัพโหลดข้อมูลที่ค้างไว้สำเร็จ");
   }
 }
 file.close();
 SPIFFS.remove(PENDING_FILE);
}

void setup() {
 Serial.begin(115200);
 SPIFFS.begin(true);
 pinMode(SWITCH_PIN, INPUT_PULLUP);

 WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(".");
 }
 Serial.println("\nเชื่อมต่อ WiFi สำเร็จ");
 Serial.print("IP Address: ");
 Serial.println(WiFi.localIP());

 configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

 config.database_url = "doit-esp32-devkit-v1-default-rtdb.asia-southeast1.firebasedatabase.app";
 config.signer.tokens.legacy_token = "AIzaSyCRGXrZjOH2wckjEIi_1BDWKypdTafLKyQ";

 Firebase.begin(&config, &auth);
 Firebase.reconnectWiFi(true);
 fbdo.setResponseSize(4096);
 
 uploadPendingData();
}

void loop() {
 if (digitalRead(SWITCH_PIN) == LOW) {
   struct tm timeinfo;
   if(!getLocalTime(&timeinfo)) {
     Serial.println("Failed to obtain time");
     return;
   }
   
   char timeStr[30];
   strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
   
   FirebaseJson json;
   json.add("timestamp", String(timeStr));
   json.add("ip_address", WiFi.localIP().toString());
   
   String jsonStr;
   json.toString(jsonStr);
   
   if(WiFi.status() == WL_CONNECTED) {
     if (Firebase.pushJSON(fbdo, "/button_press", json)) {
       Serial.println("บันทึกข้อมูลสำเร็จ");
     } else {
       saveToSPIFFS(jsonStr);
       Serial.println("เก็บข้อมูลไว้ในหน่วยความจำ");
     }
   } else {
     saveToSPIFFS(jsonStr);
     Serial.println("เก็บข้อมูลไว้ในหน่วยความจำ");
   }
   delay(500);
 }
 
 static unsigned long lastCheck = 0;
 if(millis() - lastCheck > 30000) {  // ตรวจสอบทุก 30 วินาที
   lastCheck = millis();
   if(WiFi.status() == WL_CONNECTED) {
     uploadPendingData();
   }
 }
}
