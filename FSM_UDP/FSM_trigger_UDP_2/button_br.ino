void button_def() {
  // read the state of the switch/button:
  currentState = digitalRead(BUTTON_PIN);
  if (lastState == HIGH && currentState == LOW) {
    Serial.println("tombol br ditekan");
    message.device_id = 1;
    message.device_val = 1;
    sendDataUDP(message);//kirim data message
  } else if (lastState == LOW && currentState == HIGH) {
    Serial.println("tombol br dilepas");
    message.device_id = 1;
    message.device_val = 0;
  }
  lastState = currentState;
}

void sendDataUDP(broadcast_message data) {
  // Buat buffer untuk menyimpan data yang akan dikirim
  byte buffer[sizeof(broadcast_message)];

  // Salin data dari struct ke buffer
  memcpy(buffer, &data, sizeof(broadcast_message));

  // Kirim data melalui UDP
  udp.beginPacket(IPAddress(255, 255, 255, 255), udpPort);
  udp.write(buffer, sizeof(broadcast_message));
  udp.endPacket();

  Serial.println("Data sent via UDP");
}