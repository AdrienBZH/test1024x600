#ifndef LIST_FILE_SD_H
#define LIST_FILE_SD_H

#include <Arduino.h>
#include <esp_err.h>

// Maximum number of files tracked
#ifndef MAX_FILES
#define MAX_FILES 50
#endif

// Exported storage (compatible avec l'ancien code si besoin)
// FilePatch[i] contient le chemin complet monté (ex: "/sdcard/assets/foo.bin")
extern char *FilePatch[MAX_FILES];
extern uint64_t FileSizeArr[MAX_FILES];
extern uint8_t file_num;

// Lance le scan du dossier 'base_path' sur le filesystem monté (récursif).
// Exemple: list_files_sd("/assets");
void list_files_sd(const char *base_path);

// Initialise la SD (sd_mmc_init) puis lance le scan (récursif).
// Retourne ESP_OK si SD montée et scan fait, sinon ESP_FAIL.
esp_err_t sd_list_init(const char *base_path);

// Accesseurs utilitaires
uint8_t get_file_count();
const char *get_file_path(uint8_t index);
uint64_t get_file_size(uint8_t index);

// Libère les buffers alloués (optionnel si tu veux nettoyer)
void free_file_list();

#endif // LIST_FILE_SD_H