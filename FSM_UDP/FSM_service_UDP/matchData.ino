int matchData(int check_t_id, int check_t_val) {  //match data ESP-NOW with EEPROM

  for (int i = 0; i < 1; i++) {                  // perulangan membaca 1 alamat EEPROM (sesuai dengan contoh pesan JSON)
    for (int j = 0; j < 3; j++) {                // perulangan mencari alamat dari t_id
      int t_id_address = (4 * i) + (4 * j) + 0;  // Alamat untuk t_id
      int read_t_id = EEPROM.read(t_id_address);
      if (read_t_id == 255) {  // jika bernilai 255 atau tidak ada data, maka berhenti cari
        break;
      }

      if (check_t_id == read_t_id) {                            // jika data t_id yang dicari benar
        for (int k = 0; k < 3; k++) {                           // perulangan membaca alamat t_val
          int t_val_address = (4 * i) + (4 * j) + (4 * k) + 1;  // Alamat untuk t_val
          int read_t_val = EEPROM.read(t_val_address);
          if (read_t_val == 255) {
            break;
          }

          if (check_t_val == read_t_val) {
            int s_val_address = (4 * i) + (4 * j) + (4 * k) + 2;
            s_val_broadcast = EEPROM.read(s_val_address);
            Serial.println("terdapat data eeprom yang cocok yaitu : ");
            Serial.print("t_id : ");
            Serial.println(read_t_id);
            Serial.print("t_val : ");
            Serial.println(read_t_val);
            return s_val_broadcast;  // keluarkan nilai dari service
          }
        }
      }
    }
  }

  return s_val_broadcast;  // keluarkan nilai default jika tidak ditemukan
}
