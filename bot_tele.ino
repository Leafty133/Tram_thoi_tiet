#include <WiFi.h>
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <Adafruit_I2CDevice.h>

#define SCREEN_WIDTH 128 // Khởi tạo OLED SSD1306
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0X3C
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN 25          // Khởi tạo đối tượng DHT22
#define DHTTYPE DHT22      
DHT dht(DHTPIN, DHTTYPE);

#define TRIG_PIN 27
#define ECHO_PIN 26

const long intervalTempHum = 1000;
const long intervalDistance = 100;
unsigned long timestampTH = 0;
unsigned long timestampDis = 0;

float temp = 0;
float hum = 0;
float dis = 0;

const char* ssid = "Phuc Luong";      
const char* password = "17071999";  
const String botToken = "7602743847:AAERHbGBfH1NziLd4MJn4skhceP16M6yf4k";  
const String chatID = "5927518024";      

WiFiClientSecure client;
long lastUpdateId = 0;  // Lưu ID tin nhắn cuối cùng

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Wire.begin(32, 33); // Đặt chân 32, 33 là chân SDA, SCL của OLED
  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  client.setInsecure(); // Xóa phương thức bảo mật của giao thức HTTP

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Đang kết nối WiFi...");
  }
  Serial.println("WiFi đã kết nối!");

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("Không tìm thấy OLED!"));
        while (true);
    }

    display.clearDisplay();  // Khởi tạo OLED
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
}

//Hàm lấy khoảng cách
float getDistance(int trig_pin, int echo_pin){
  digitalWrite(trig_pin, LOW);
  delayMicroseconds(2);
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);

  long duration = pulseIn(echo_pin, HIGH);
  float Distance = duration * 0.034/2;

  return Distance;
}  
// Lấy tin nhắn từ Telegram
void checkTelegram() {
  HTTPClient http;
  String url = "https://api.telegram.org/bot" + botToken + "/getUpdates?offset=" + String(lastUpdateId + 1);
  http.begin(client, url);

  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    String payload = http.getString();

    DynamicJsonDocument doc(4096);
    deserializeJson(doc, payload);
    
    if (doc["result"].size() > 0) {  
      for (JsonObject msg : doc["result"].as<JsonArray>()) {
        long updateId = msg["update_id"];
        String message = msg["message"]["text"];
        Serial.println("📥 Tin nhắn nhận được: " + message);

        if (message.equals("weather")) {
          float temp = dht.readTemperature();
          float hum = dht.readHumidity();
          String response = "🌡️ Nhiệt độ: " + String(temp) + "°C\n💧 Độ ẩm: " + String(hum) + "%";
          sendTelegram(response);
        }

        lastUpdateId = updateId; // Cập nhật ID để tránh nhận lại tin nhắn cũ
      }
    }
  } else {
    Serial.print("❌ Lỗi khi lấy tin nhắn: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

// Gửi tin nhắn lên Telegram
void sendTelegram(String message) {
  HTTPClient http;
  String url = "https://api.telegram.org/bot" + botToken + "/sendMessage";
  http.begin(client, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String data = "chat_id=" + chatID + "&text=" + message;
  int httpResponseCode = http.POST(data);

  if (httpResponseCode == 200) {
    Serial.println("✅ Gửi tin nhắn thành công!");
  } else {
    Serial.print("❌ Lỗi gửi tin nhắn: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}
void Serial_Monitor(){
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  if(!isnan(temp) && !isnan(hum)){
    Serial.print("Temperature: "); Serial.println(temp);
    Serial.print("Humadity: "); Serial.println(hum); 
  }
  else{
    Serial.println("Loi khong nhan duoc du lieu");
  }
}

void OLED_Display(float temp, float hum,float dis){

  display.clearDisplay();

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.setTextSize(1);
  display.setCursor(64, 4);
  display.print(temp);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(1);
  display.print("C");

  // hiển thị độ ẩm
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print("Hum: ");
  display.setTextSize(1);
  display.setCursor(64, 24);
  display.print(hum);
  display.print(" %"); 
  
  // Hien thi khoang cach
  display.setTextSize(2);
  display.setCursor(0, 40);
  display.print("Dis.: ");
  display.setTextSize(1);
  display.setCursor(64, 44);
  display.print(dis);
  display.print(" cm");

  display.display();
}

void loop() {
  unsigned long currentTimeStamp = millis();
  Serial_Monitor();
  checkTelegram();  

  if(currentTimeStamp - timestampTH >= intervalTempHum){
    temp = dht.readTemperature();
    hum = dht.readHumidity();
    timestampTH = currentTimeStamp;
  }

  if(currentTimeStamp - timestampDis >= intervalDistance){
    dis = getDistance(TRIG_PIN, ECHO_PIN);
    timestampDis = currentTimeStamp;
  }

  OLED_Display(temp, hum, dis);
}
