// Remplacer la fonction ui_load_image_psram dans ton fichier ui_img_loader_psram.cpp par la version ci-dessous.
// Cette version charge uniquement les images whitelistées (ex: le splash) au boot.
// Les autres images retournent NULL (seront chargées plus tard par ui_img_psram_replace_image).

uint8_t * ui_load_image_psram(char * fname, const uint32_t compsize, const uint32_t size)
{
    if (!fname || size == 0) return NULL;
    Serial.printf("UI_LOAD_IMAGE_PS: fname='%s' compsize=%u size=%u\n", fname, (unsigned)compsize, (unsigned)size);

    // Nom canonique attendu du splash (adapter si besoin)
    const char *splash_names[] = {
        "ui_img_logo_loading_1024x600_70_png.bin",
        "ui_img_logo_loading_70_png.bin", // fallback court
        NULL
    };

    // Check si fname contient l'un des noms whitelistés
    bool want_load = false;
    for (int i = 0; splash_names[i]; ++i) {
        if (strstr(fname, splash_names[i]) != NULL) {
            want_load = true;
            break;
        }
    }

    if (!want_load) {
        // Skip automatic load for non-whitelisted images (we'll load them later manually)
        Serial.printf("UI_LOAD_IMAGE_PS: skipping auto-load for '%s'\n", fname);
        return NULL;
    }

    // --- Si on est ici, on charge volontairement (splash) ---
    uint32_t real_compsize = compsize;
    if (real_compsize == 0) {
        // detect size from LittleFS
        char path[256];
        // convert "S:assets/..." -> "/assets/..."
        const char *p = fname;
        if (p[0] && p[1] == ':') p += 2;
        if (p[0] == '/') snprintf(path, sizeof(path), "%s", p);
        else snprintf(path, sizeof(path), "/%s", p);

        File f = LittleFS.open(path, "r");
        if (!f) {
            Serial.printf("UI_LOAD_IMAGE_PS: cannot open '%s' to detect size\n", path);
            return NULL;
        }
        real_compsize = f.size();
        f.close();
        Serial.printf("UI_LOAD_IMAGE_PS: detected compsize=%u\n", (unsigned)real_compsize);
    }

    // if stored raw (no compression)
    if (real_compsize == size) {
        uint8_t *buf = read_file_into_psram(fname, size);
        if (!buf) {
            Serial.println("UI_LOAD_IMAGE_PS: read_file_into_psram failed (raw)");
            return NULL;
        }
        return buf;
    }

    // otherwise read compressed and decompress
    uint8_t *zip = read_file_into_psram(fname, real_compsize);
    if (!zip) {
        Serial.println("UI_LOAD_IMAGE_PS: read_file_into_psram failed (zip)");
        return NULL;
    }

    uint8_t *out = (uint8_t*)alloc_preferred(size);
    if (!out) {
        Serial.printf("UI_LOAD_IMAGE_PS: out alloc failed size=%u\n", (unsigned)size);
        free(zip);
        return NULL;
    }

    Serial.println("UI_LOAD_IMAGE_PS: starting fastlz_decompress...");
    int outsize = fastlz_decompress(zip, real_compsize, out, size);
    if (outsize <= 0) {
        Serial.printf("UI_LOAD_IMAGE_PS: fastlz_decompress failed ret=%d\n", outsize);
        free(zip);
        free(out);
        return NULL;
    }
    if ((uint32_t)outsize != size) {
        Serial.printf("UI_LOAD_IMAGE_PS: warning outsize=%d != expected %u\n", outsize, (unsigned)size);
    }

    free(zip);
    Serial.printf("UI_LOAD_IMAGE_PS: decompressed OK -> %p\n", out);
    return out;
}