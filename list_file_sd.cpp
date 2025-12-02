#include "list_file_sd.h"
#include "user_sd.h"
#include <SD_MMC.h>

char *FilePatch[MAX_FILES];
uint64_t FileSizeArr[MAX_FILES];
uint8_t file_num = 0;

static const char *mount_point = MOUNT_POINT;

static void free_file_list_internal() {
  for (uint8_t i = 0; i < MAX_FILES; ++i) {
    if (FilePatch[i]) {
      free(FilePatch[i]);
      FilePatch[i] = nullptr;
      FileSizeArr[i] = 0;
    }
  }
  file_num = 0;
}

void free_file_list() {
  free_file_list_internal();
}

uint8_t get_file_count() {
  return file_num;
}

const char *get_file_path(uint8_t index) {
  if (index >= file_num) return nullptr;
  return FilePatch[index];
}

uint64_t get_file_size(uint8_t index) {
  if (index >= file_num) return 0;
  return FileSizeArr[index];
}

// Helper: combine two path components safely (no duplicate slashes)
static String join_paths(const String &a, const String &b) {
  if (a.endsWith("/")) {
    if (b.startsWith("/")) return a + b.substring(1);
    return a + b;
  } else {
    if (b.startsWith("/")) return a + b;
    return a + "/" + b;
  }
}

// Recursive directory scanner that prints a tree and stores file paths/sizes.
// depth for indentation, idx is the current index into FilePatch/FileSizeArr.
static void scan_dir_recursive(const String &path, int depth, uint8_t &idx) {
  File dir = SD_MMC.open(path.c_str());
  if (!dir) {
    Serial.print("Failed to open directory: ");
    Serial.println(path);
    return;
  }

  if (!dir.isDirectory()) {
    Serial.print("Provided path is not a directory: ");
    Serial.println(path);
    dir.close();
    return;
  }

  // Print the directory line
  for (int i = 0; i < depth; ++i) Serial.print("  ");
  Serial.print("+ ");
  Serial.println(path);

  File file = dir.openNextFile();
  while (file) {
    String name = String(file.name()); // may be "/assets/xxx" or "xxx" depending on FS
    // Normalize name to not include leading mount point fragments; treat as relative name
    // If name contains the current path at start, try to get basename
    String relName = name;
    // If name starts with '/', strip leading '/'
    if (relName.startsWith("/")) relName = relName.substring(1);

    // Determine entry path relative to mount_point
    String entryPath;
    // If the file.name() already contains the path (e.g. "assets/foo"), we build entryPath as that
    // But to be consistent, we build entryPath by joining the 'path' and the basename of relName
    int lastSlash = relName.lastIndexOf('/');
    String baseName = (lastSlash >= 0) ? relName.substring(lastSlash + 1) : relName;
    entryPath = join_paths(path, baseName); // e.g. "/assets/sub" + "file.bin" => "/assets/sub/file.bin"

    if (file.isDirectory()) {
      // Print dir entry
      for (int i = 0; i < depth + 1; ++i) Serial.print("  ");
      Serial.print("DIR  : ");
      Serial.println(baseName);

      // Recurse into this directory. Need the path to open: use entryPath
      scan_dir_recursive(entryPath, depth + 1, idx);
    } else {
      // File entry: print with size
      uint64_t sz = file.size();
      for (int i = 0; i < depth + 1; ++i) Serial.print("  ");
      Serial.print("FILE : ");
      Serial.print(baseName);
      Serial.print("  ");
      Serial.print(sz);
      Serial.println(" bytes");

      // Store full path with mount_point prefix if there's room
      if (idx < MAX_FILES) {
        // Build stored path: mount_point + entryPath
        String storePath = String(mount_point) + entryPath;
        // allocate
        size_t len = storePath.length() + 1;
        FilePatch[idx] = (char *)malloc(len);
        if (FilePatch[idx]) {
          memcpy(FilePatch[idx], storePath.c_str(), len);
          FileSizeArr[idx] = sz;
          idx++;
        } else {
          Serial.println("Memory allocation failed for File path!");
        }
      } else {
        Serial.println("File list capacity reached, skipping further files.");
      }
    }

    file.close();
    file = dir.openNextFile();
  }

  dir.close();
}

// Public: list files (recursive) starting at base_path
void list_files_sd(const char *base_path) {
  free_file_list_internal();
  Serial.print("Scanning recursively from: ");
  Serial.println(base_path);

  uint8_t idx = 0;
  String startPath = String(base_path);
  // Ensure startPath has no trailing slash except root "/"
  if (startPath.length() > 1 && startPath.endsWith("/")) {
    startPath = startPath.substring(0, startPath.length() - 1);
  }

  scan_dir_recursive(startPath, 0, idx);
  file_num = idx;

  Serial.print("sd card ok, nombre de fichier : ");
  Serial.println(file_num);
}

// Initialize SD and run recursive listing
esp_err_t sd_list_init(const char *base_path) {
  esp_err_t r = sd_mmc_init();
  if (r != ESP_OK) {
    Serial.println("sd_mmc_init() failed in sd_list_init");
    return ESP_FAIL;
  }
  list_files_sd(base_path);
  return ESP_OK;
}