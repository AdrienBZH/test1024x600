#include "ui.h"
#include "ui_img_manager.h"

#if defined(ESP32)
  #include "esp_heap_caps.h"
#endif

uint8_t* _ui_load_binary(char* fname, const uint32_t size)
{
    lv_fs_file_t f;
    lv_fs_res_t res = lv_fs_open(&f, fname, LV_FS_MODE_RD);
    if (res != LV_FS_RES_OK) return NULL;

    uint32_t read_num = 0;
    uint8_t* buf = NULL;

#if defined(ESP32)
    // Try to allocate a DMA-capable buffer in PSRAM first.
    // Request MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA | MALLOC_CAP_8BIT so the buffer
    // is suitable for DMA transfers to the LCD and aligned properly.
    buf = (uint8_t*)heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
    if (!buf) {
        // Fallback: try PSRAM without DMA flag
        buf = (uint8_t*)heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    }
    if (!buf) {
        // Final fallback: default heap (may be internal)
        buf = (uint8_t*)heap_caps_malloc(size, MALLOC_CAP_DEFAULT);
    }
#else
    buf = (uint8_t*)lv_mem_alloc(size);
#endif

    if (!buf) {
        lv_fs_close(&f);
        return NULL;
    }

    res = lv_fs_read(&f, buf, size, &read_num);
    if (res != LV_FS_RES_OK || read_num != size)
    {
#if defined(ESP32)
        heap_caps_free(buf);
#else
        lv_mem_free(buf);
#endif
        lv_fs_close(&f);
        return NULL;
    }

    lv_fs_close(&f);
    return buf;
}

void _ui_free_binary(const void *ptr)
{
    if (!ptr) return;
#if defined(ESP32)
    heap_caps_free((void*)ptr);
#else
    lv_mem_free((void*)ptr);
#endif
}