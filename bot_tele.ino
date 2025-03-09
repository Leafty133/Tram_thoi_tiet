#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "DHT.h"

#define DHTPIN 25           // Chân kết nối cảm biến (GPIO4)
#define DHTTYPE DHT22      // Loại cảm biến DHT11 hoặc DHT22
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Phuc Luong";      // Tên WiFi của bạn
const char* password = "17071999";  // Mật khẩu WiFi
const String botToken = "7602743847:AAERHbGBfH1NziLd4MJn4skhceP16M6yf4k";  // Token của bot Telegram
const String chatID = "5927518024";      // ID chat Telegram của bạn

WiFiClientSecure client;  // Khởi tạo client bảo mật

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  dht.begin();
  client.setInsecure();  // Bỏ qua chứng chỉ SSL

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Đang kết nối WiFi...");
  }
  Serial.println("WiFi đã kết nối!");
}

void sendTelegram(String message) {
  HTTPClient http;
  String url = "https://api.telegram.org/bot" + botToken + "/sendMessage";
  http.begin(client, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String data = "chat_id=" + chatID + "&text=" + message;
  int httpResponseCode = http.POST(data);

  if (httpResponseCode == 200) {
    Serial.println("Gửi tin nhắn thành công!");
  } else {
    Serial.print("Lỗi gửi tin nhắn: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (!isnan(temp) && !isnan(hum)) {
    String message = "🌡️ Nhiệt độ: " + String(temp) + "°C\n💧 Độ ẩm: " + String(hum) + "%";
    sendTelegram(message);
    Serial.println("Temperature: "); Serial.println(temp); 
    Serial.println("Humadity: "); Serial.println(hum); 
  } else {
    Serial.println("Không đọc được dữ liệu từ cảm biến!");
  }
  delay(30000);  // Gửi mỗi 30 giây
}
