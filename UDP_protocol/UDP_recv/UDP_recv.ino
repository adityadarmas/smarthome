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

  // Mulai menerima data pada port UDP
  udp.begin(udpPort);
}

void loop() {
  // Menerima data UDP
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char packetBuffer[255];
    int len = udp.read(packetBuffer, sizeof(packetBuffer));
    if (len > 0) {
      packetBuffer[len] = 0;
      Serial.println("Received packet: " + String(packetBuffer));
    }
  }

  // Menunggu sebentar sebelum membaca data lagi
  delay(1000);
}
