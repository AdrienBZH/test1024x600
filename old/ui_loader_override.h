#ifndef _UI_LOADER_OVERRIDE_H
#define _UI_LOADER_OVERRIDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Prototype de notre loader PSRAM (sera appelé par la macro) */
uint8_t * ui_load_image_psram(char * fname, const uint32_t compsize, const uint32_t size);

/* Redirige la macro SLS vers notre loader PSRAM (non-intrusif, pas de modification des fichiers SLS) */
#define UI_LOAD_IMAGE(fname, compsize, size) ui_load_image_psram((char*)(fname), (compsize), (size))

#ifdef __cplusplus
}
#endif

#endif /* _UI_LOADER_OVERRIDE_H */