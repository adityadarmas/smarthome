void data_receive(const uint8_t* mac, const uint8_t* incomingData, int len) {
  memcpy(&message, incomingData, sizeof(message));
  Serial.print("Bytes received: ");
  Serial.println(len);
  //  int counter;
  //  if (len == 4) {  //jika yang dikirimkan adalah waktu
  //    Serial.print("reset time");
  //    Serial.println(counter);
  //  } else {
  Serial.print("dev_id: ");
  Serial.println(message.device_id);
  Serial.print("dev_val: ");
  Serial.println(message.device_val);
  Serial.println();
  //  }
}
