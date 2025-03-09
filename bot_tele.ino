#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <Adafruit_I2CDevice.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0X3C
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN 25           
#define DHTTYPE DHT22      
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Phuc Luong";      
const char* password = "17071999";  
const String botToken = "7602743847:AAERHbGBfH1NziLd4MJn4skhceP16M6yf4k";  
const String chatID = "5927518024";      

WiFiClientSecure client;
long lastUpdateId = 0;  // Lưu ID tin nhắn cuối cùng

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Wire.begin(32, 33);
  dht.begin();
  client.setInsecure();

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Đang kết nối WiFi...");
  }
  Serial.println("WiFi đã kết nối!");

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("Không tìm thấy OLED!"));
        while (true);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
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
    Serial.print("Tempẻature: "); Serial.println(temp);
    Serial.print("Humadity: "); Serial.println(hum); 
  }
  else{
    Serial.println("Loi khong nhan duoc du lieu");
  }
}

void OLED_Display(){
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature: ");
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print(temp);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");

  // hiển thị độ ẩm
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Humidity: ");
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print(hum);
  display.print(" %"); 
  
  display.display();

}
void loop() {
  Serial_Monitor();
  checkTelegram();  
  OLED_Display();
  delay(5000);  
}
