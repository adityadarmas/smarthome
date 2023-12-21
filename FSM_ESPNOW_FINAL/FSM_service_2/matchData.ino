int matchData(int check_t_id, int check_t_val) {  

  for (int i = 0; i < 1; i++) {   //perulangan alamat eeprom              
    for (int j = 0; j < 3; j++) {                
      int t_id_address = (4 * i) + (4 * j) + 0;  
      int read_t_id = EEPROM.read(t_id_address);
      if (read_t_id == 255) {  
        break;
      }

      if (check_t_id == read_t_id) { //perulangan t_id                           
        for (int k = 0; k < 3; k++) {                           
          int t_val_address = (4 * i) + (4 * j) + (4 * k) + 1; 
          int read_t_val = EEPROM.read(t_val_address);
          if (read_t_val == 255) {
            break;
          }

          if (check_t_val == read_t_val) { //perulangan t_val
            int s_val_address = (4 * i) + (4 * j) + (4 * k) + 2;
            s_val_broadcast = EEPROM.read(s_val_address);
            Serial.println("terdapat data eeprom yang cocok yaitu : ");
            Serial.print("t_id : ");
            Serial.println(read_t_id);
            Serial.print("t_val : ");
            Serial.println(read_t_val);
            return s_val_broadcast;  
          }
        }
      }
    }
  }

  return s_val_broadcast;  
}
