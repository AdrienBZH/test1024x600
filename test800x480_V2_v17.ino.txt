//v17 analyse la memoire psram
// Simple PSRAM check + optional image load test
// Pour activer le test de chargement des images au boot, mettre LOAD_IMAGES_AT_BOOT à 1
#define LOAD_IMAGES_AT_BOOT 0

#include <Arduino.h>
#include "user_sd.h"
#include "lvgl_port.h"
#include "rgb_lcd_port.h"
#include "src/ui.h"
#include "list_file_sd.h"

#if defined(ESP32)
  #include "esp_heap_caps.h"
#endif

static void print_psram_stats(const char *label) {
  Serial.printf("\n--- MEM STATS: %s ---\n", label);

#if defined(BOARD_HAS_PSRAM) && defined(ESP32)
  // Try Arduino convenience API if available (ESP.getPsramSize)
  #ifdef ESP
    size_t psram_total_arduino = 0;
    // ESP.getPsramSize() exists on Arduino cores built with PSRAM support
    psram_total_arduino = ESP.getPsramSize();
    if (psram_total_arduino) {
      Serial.printf("ESP API: PSRAM total=%u bytes\n", (unsigned)psram_total_arduino);
    } else {
      Serial.println("ESP API: PSRAM total: (0)");
    }
  #else
    Serial.println("ESP API: not available");
  #endif
#else
  Serial.println("BOARD_HAS_PSRAM not defined at build time -> PSRAM not enabled in core");
#endif

#if defined(ESP32)
  size_t internal_total = heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  size_t internal_free  = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  size_t psram_total2   = heap_caps_get_total_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  size_t psram_free2    = heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

  Serial.printf("heap_caps: INTERNAL total=%u free=%u\n", (unsigned)internal_total, (unsigned)internal_free);
  Serial.printf("heap_caps: PSRAM total=%u free=%u\n", (unsigned)psram_total2, (unsigned)psram_free2);
#endif

#if defined(ESP)
  Serial.printf("ESP.getFreeHeap(): %u bytes\n", (unsigned)ESP.getFreeHeap());
#endif

  Serial.println("--------------------------\n");
}

void setup() {
  Serial.begin(115200);
  delay(200);

  print_psram_stats("boot (before init)");

  static esp_lcd_panel_handle_t panel_handle = NULL;
  static esp_lcd_touch_handle_t tp_handle = NULL;
  tp_handle = touch_gt911_init();
  panel_handle = waveshare_esp32_s3_rgb_lcd_init();

  ESP_ERROR_CHECK(lvgl_port_init(panel_handle, tp_handle));
  print_psram_stats("after lvgl_port_init");

  // Initialize SD card and scan /assets using the list_file_sd module
  if (sd_list_init("/assets") != ESP_OK) {
      Serial.println("SD init/scan failed");
      print_psram_stats("after sd_list_init (failed)");
      // If SD card init fails, keep running but don't try to load images
  } else {
      print_psram_stats("after sd_list_init (ok)");
  }

#if LOAD_IMAGES_AT_BOOT
  Serial.println("Loading images (LOAD_IMAGES_AT_BOOT=1) ...");
  // Ces fonctions existent dans ui_img_*.c générés par SquareLine
  ui_img_config_led_std_png_load();
  ui_img_logo_loading_70_png_load();
  delay(200); // laisser le temps aux allocations et IO
  print_psram_stats("after images loaded");
#endif

  if (lvgl_port_lock(-1)) {
    delay(50);
    ui_init();
    lvgl_port_unlock();
  }

  wavesahre_rgb_lcd_bl_on();

  print_psram_stats("setup end");
}

void loop() {
  // Simple periodic PSRAM print to monitor changes (commenter si trop verbeux)
  static unsigned long last = 0;
  if (millis() - last > 5000) {
    last = millis();
    print_psram_stats("periodic");
  }
  delay(5);
}