#ifndef LISTFILES_H
#define LISTFILES_H

#include <LittleFS.h>

// Fonction récursive pour lister les fichiers/dossiers
void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing: %s\n", dirname);
  
  File root = fs.open(dirname);
  if (!root || !root.isDirectory()) {
    Serial.println("Failed to open directory");
    return;
  }
  
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("DIR: ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);  // Récursion
      }
    } else {
      Serial.print("FILE: ");
      Serial.print(file.name());
      Serial.print(" SIZE: ");
      Serial.print(file.size());
      Serial.println(" bytes");
    }
    file = root.openNextFile();
  }
}

// Fonction principale
void listLittleFS() {
  Serial.println("Listing LittleFS:");
  listDir(LittleFS, "/", 10);  // 2 niveaux de profondeur
  Serial.println("End of list.");
}

#endif