#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Ping.h> // ใช้สำหรับการ Ping
#include <ESPDateTime.h> // ใช้สำหรับดึงเวลาและวันที่

// กำหนดขนาดจอ OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// กำหนดขา I2C สำหรับจอ OLED
#define OLED_RESET    -1 // ไม่ใช้ขา Reset
#define SCREEN_ADDRESS 0x3C // ที่อยู่ I2C ของจอ OLED

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// กำหนดชื่อและรหัสผ่าน Wi-Fi
const char* ssid = "101-IOT";
const char* password = "10101010";

// กำหนด MQTT Broker สำหรับทดสอบการเชื่อมต่ออินเทอร์เน็ต
const char* mqtt_server = "broker.hivemq.com"; // MQTT Broker ฟรี
const int mqtt_port = 1883; // พอร์ต MQTT

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// กำหนดขาสำหรับปุ่มกด
const int nextButtonPin = 15; // ปุ่มหน้าถัดไป (Next Page)
const int prevButtonPin = 16; // ปุ่มหน้าก่อนหน้า (Previous Page)
int currentPage = 0; // หน้าปัจจุบัน
int lastNextButtonState = HIGH; // สถานะปุ่มหน้าถัดไปครั้งล่าสุด
int lastPrevButtonState = HIGH; // สถานะปุ่มหน้าก่อนหน้าครั้งล่าสุด

void connectToWiFi() {
  // เชื่อมต่อ Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("กำลังเชื่อมต่อ Wi-Fi...");

  // รอจนกว่า Wi-Fi จะเชื่อมต่อสำเร็จ
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // แสดงข้อความว่าเชื่อมต่อ Wi-Fi สำเร็จ
  Serial.println("");
  Serial.println("เชื่อมต่อ Wi-Fi สำเร็จ");
  Serial.println(WiFi.localIP());
}

bool checkInternetConnection() {
  // ทดสอบการ Ping เซิร์ฟเวอร์ (เช่น google.com)
  bool success = Ping.ping("google.com", 2); // Ping 2 ครั้ง
  return success;
}

bool checkMQTTConnection() {
  // พยายามเชื่อมต่อกับ MQTT Broker
  if (mqttClient.connect("ESP32TestClient")) {
    mqttClient.disconnect(); // ตัดการเชื่อมต่อหลังจากทดสอบ
    return true;
  } else {
    return false;
  }
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  switch (currentPage) {
    case 0: // สถานะการเชื่อมต่อ Wi-Fi
      display.setCursor(0, 0);
      display.println("Wi-Fi Status:");
      display.setCursor(0, 10);
      if (WiFi.status() == WL_CONNECTED) {
        display.println("Connected");
      } else {
        display.println("Disconnected");
      }
      break;

    case 1: // IP Address
      display.setCursor(0, 0);
      display.println("IP Address:");
      display.setCursor(0, 10);
      display.println(WiFi.localIP());
      break;

    case 2: // สถานะการออกอินเทอร์เน็ต
      display.setCursor(0, 0);
      display.println("Internet Status:");
      display.setCursor(0, 10);
      if (checkInternetConnection()) {
        display.println("Ping: OK");
      } else {
        display.println("Ping: FAIL");
      }
      break;

    case 3: // สถานะการเชื่อมต่อ MQTT
      display.setCursor(0, 0);
      display.println("MQTT Status:");
      display.setCursor(0, 10);
      if (checkMQTTConnection()) {
        display.println("Connected");
      } else {
        display.println("Disconnected");
      }
      break;
  }

  // แสดงเวลาและวันที่บนบรรทัดที่ 3
  display.setCursor(0, 25);
  DateTime.format(DateFormatter::SIMPLE); // กำหนดรูปแบบวันที่และเวลา
  String dateTime = DateTime.toString(); // ได้วันที่และเวลาในรูปแบบ YYYY-MM-DD HH:MM:SS
  display.println(dateTime); // แสดงวันที่และเวลา

  display.display();
}

void setup() {
  // เริ่มต้น Serial Monitor
  Serial.begin(115200);

  // เริ่มต้นจอ OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // หยุดการทำงานถ้าไม่สามารถเริ่มต้นจอ OLED ได้
  }

  // ล้างหน้าจอ
  display.clearDisplay();
  display.display();

  // ตั้งค่า GPIO สำหรับปุ่มกด
  pinMode(nextButtonPin, INPUT_PULLUP); // ใช้ Pull-up resistor
  pinMode(prevButtonPin, INPUT_PULLUP); // ใช้ Pull-up resistor

  // เชื่อมต่อ Wi-Fi ครั้งแรก
  connectToWiFi();

  // ตั้งค่า MQTT Client
  mqttClient.setServer(mqtt_server, mqtt_port);

  // เริ่มต้น ESPDateTime
  DateTime.setTimeZone("CST-7"); // ตั้งค่าโซนเวลา (GMT+7 สำหรับประเทศไทย)
  DateTime.begin();
  if (!DateTime.isTimeValid()) {
    Serial.println("Failed to get time from server.");
  } else {
    Serial.println("Time and date initialized.");
  }
}

void loop() {
  // ตรวจสอบสถานะการเชื่อมต่อ Wi-Fi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("การเชื่อมต่อ Wi-Fi หลุด, พยายามเชื่อมต่อใหม่...");
    connectToWiFi();
  }

  // อ่านค่าปุ่มกด
  int nextButtonState = digitalRead(nextButtonPin);
  int prevButtonState = digitalRead(prevButtonPin);

  // ตรวจสอบการกดปุ่มหน้าถัดไป (Next Page)
  if (nextButtonState == LOW && lastNextButtonState == HIGH) {
    currentPage = (currentPage + 1) % 4; // สลับไปหน้าถัดไป (0 → 1 → 2 → 3 → 0)
    Serial.println("กดปุ่มหน้าถัดไป: เปลี่ยนหน้าเป็น " + String(currentPage));
    delay(10); // Debounce ปุ่ม (ลดเวลาเหลือ 10 มิลลิวินาที)
  }

  // ตรวจสอบการกดปุ่มหน้าก่อนหน้า (Previous Page)
  if (prevButtonState == LOW && lastPrevButtonState == HIGH) {
    currentPage = (currentPage - 1 + 4) % 4; // สลับไปหน้าก่อนหน้า (3 → 2 → 1 → 0 → 3)
    Serial.println("กดปุ่มหน้าก่อนหน้า: เปลี่ยนหน้าเป็น " + String(currentPage));
    delay(10); // Debounce ปุ่ม (ลดเวลาเหลือ 10 มิลลิวินาที)
  }

  // บันทึกสถานะปุ่มกดครั้งล่าสุด
  lastNextButtonState = nextButtonState;
  lastPrevButtonState = prevButtonState;

  // อัปเดตจอ OLED
  updateDisplay();
}
