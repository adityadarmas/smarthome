#include <WiFi.h>
#include <WiFiUdp.h>

const char *ssid = "UnknownDevice";
const char *password = "12345678";
const int udpPort = 12345;  // Ganti dengan port yang Anda inginkan

WiFiUDP udp;

void setup() {
  Serial.begin(115200);

  // Menghubungkan ke WiFi sebagai Station
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  // Mengirim data ke alamat IP broadcast pada port UDP
  udp.beginPacket(IPAddress(255, 255, 255, 255), udpPort);
  udp.print("Hello, ESP32!");
  udp.endPacket();

  // Menunggu sebentar sebelum mengirim data lagi
  delay(5000);
}
