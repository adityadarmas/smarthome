void callback(char* topic, byte* payload, unsigned int length) {  // Callback ini akan dipanggil ketika pesan MQTT diterima
  Serial.print("pesan dari topik : ");
  Serial.println(topic);

  if (strcmp(topic, mqtt_topic_rule) == 0) {  //menerima data "rule"

    // Membaca payload pesan
    String message_rule;
    for (int i = 0; i < length; i++) {
      message_rule += (char)payload[i];
    }

    //menampilkan data JSON yang diterima
    Serial.println(message_rule);
    Serial.println(" ");

    // Parsing pesan JSON
    StaticJsonDocument<200> jsonDoc;  // Ukuran buffer JSON
    DeserializationError error = deserializeJson(jsonDoc, message_rule);
    if (error) {
      Serial.print("Gagal parsing JSON: ");
      Serial.println(error.c_str());
      return;
    }

    // Mendapatkan nilai dari JSON
    int rule_id = jsonDoc["rule_id"];
    JsonObject trigger = jsonDoc["trigger"];
    JsonObject service = jsonDoc["service"];

    RuleData rule_data;
    rule_data.rule_id = rule_id;

    // Iterate over trigger and print values
    int index_trigger = 0;
    for (JsonPair kv : trigger) {

      String t_id_str = kv.key().c_str();
      int t_id = t_id_str.toInt();
      rule_data.trigger.t_id[index_trigger] = t_id;

      int t_val = kv.value().as<int>();
      rule_data.trigger.t_val[index_trigger] = t_val;

      index_trigger++;
    }

    // Iterate over service and print values
    int index_service = 0;
    for (JsonPair kv : service) {

      String s_id_str = kv.key().c_str();
      int s_id = s_id_str.toInt();
      rule_data.service.s_id[index_service] = s_id;

      int s_val = kv.value().as<int>();
      rule_data.service.s_val[index_service] = s_val;

      index_service++;
    }

    if (rule_data.service.s_id[0] == uid) {
      //simpan data
      int jumdat = rule_data.rule_id;
      // Serial.println(jumdat);
      int EOFeeprom = jumdat - 1;
      // Menyimpan data t_id, t_val, dan s_val ke EEPROM
      for (int i = 0; i < jumdat; i++) {
        int t_id_address = (4 * jumdat) - 4;   // Alamat untuk t_id
        int t_val_address = (4 * jumdat) - 3;  // Alamat untuk t_val
        int s_val_address = (4 * jumdat) - 2;  // Alamat untuk s_val
        int diff_address = (4 * jumdat) - 1;   // Alamat untuk nilai pemisah
        // Simpan nilai t_id, t_val, s_val ke EEPROM
        EEPROM.write(t_id_address, rule_data.trigger.t_id[0]);
        EEPROM.write(t_val_address, rule_data.trigger.t_val[0]);
        EEPROM.write(s_val_address, rule_data.service.s_val[0]);
        EEPROM.write(diff_address, 254);  // Nilai pemisah
        EEPROM.commit();
      }
      readData();
    }
  } else if (strcmp(topic, mqtt_topic_service) == 0) {  //menerima data "service"
    // Membaca payload pesan
    String message_service;
    for (int i = 0; i < length; i++) {
      message_service += (char)payload[i];
    }

    //menampilkan data rule dalam bentuk json
    Serial.println(message_service);

    // Parsing pesan JSON
    StaticJsonDocument<200> jsonDoc;  // Ukuran buffer JSON
    DeserializationError error = deserializeJson(jsonDoc, message_service);
    if (error) {
      Serial.print("Gagal parsing JSON: ");
      Serial.println(error.c_str());
      return;
    }

    // Mendapatkan nilai dari JSON
    s_id_recv[0] = jsonDoc["device_id"];
    s_val_recv[0] = jsonDoc["device_value"];
    if (s_id_recv[0] == uid) {
      s_id[0] = s_id_recv[0];
      s_val[0] = s_val_recv[0];
      // Tampilkan data yang diterima
      Serial.println("Data diterima:");
      Serial.print("s_id: ");
      Serial.println(s_id[0]);
      Serial.print("s_val: ");
      Serial.println(s_val[0]);
    }
    
    //publish kode ke topik service_ack
    StaticJsonDocument<200> jsonDocTx;  // Ukuran buffer JSON
    jsonDocTx["device_id"] = s_id[0];      //ganti t_id
    jsonDocTx["device_value"] = s_val[0];  //ganti t_val
    // Serialize JSON ke string
    String jsonStringTx;
    serializeJson(jsonDocTx, jsonStringTx);
    // Kirim data JSON ke broker MQTT
    client.publish(mqtt_topic_ack, jsonStringTx.c_str());
  }
}

void readData() {  //read data eeprom
  for (int r = 0; r < 10; r++) {
    int value = EEPROM.read(r);
    Serial.print("Value at address ");
    Serial.print(r);
    Serial.print(": ");
    Serial.println(value);
  }
}
