int matchData(int check_t_id, int check_t_val) {    //match data ESP-NOW with EEPROM
  for (int address = 0; address = 20; address++) {  //perulangan membaca 20 alamat eeprom
    for (int i = 1; i = 5; i++) {                   //perulangan mencari alamat dari t_id
      int t_id_address = (4 * i) - 4;               // Alamat untuk t_id
      int read_t_id = EEPROM.read(t_id_address);
      if (read_t_id == 255) {  // jika bernilai 255 atau tidak ada data, maka berhenti cari
        break;
      }
      if (check_t_id == read_t_id) {        //jika data t_id yang dicari benar
        for (int j = 0; j = 5; j++) {       //perulangan membaca alamat t_val
          int t_val_address = (4 * j) - 3;  // Alamat untuk t_val
          int read_t_val = EEPROM.read(t_val_address);
          if (read_t_val == 255) {
            break;
          }
          if (check_t_val == read_t_val) {
            int s_val_address = (4 * j) - 2;
            s_val_broadcast = EEPROM.read(s_val_address);
            return s_val_broadcast;  // keluarkan nilai dari service
          }
        }
      }
    }
  }
}
