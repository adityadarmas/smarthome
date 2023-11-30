#include <EEPROM.h>

int data[] = {10, 20, 30, 40, 50}; // Data yang akan disimpan
int dataCount = sizeof(data) / sizeof(data[0]); // Jumlah data
int address = 0; // Alamat awal EEPROM

void setup() {
  Serial.begin(9600);
}

void loop() {
  // Simpan data dalam EEPROM
  for (int i = 0; i < dataCount; i++) {
    EEPROM.put(address, data[i]);
    address += sizeof(data[i]);
  }

  // Baca dan cetak data yang disimpan
  address = 0; // Kembalikan alamat ke awal
  for (int i = 0; i < dataCount; i++) {
    int value;
    EEPROM.get(address, value);
    Serial.print("Alamat ");
    Serial.print(address);
    Serial.print(": ");
    Serial.println(value);
    address += sizeof(value);
  }

  while (1) {
    // Loop tak terbatas
  }
}
