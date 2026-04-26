/* File: src/tests/test_controller_stubs.c
 * Stubs for controller tests that require game state
 *
 * These stubs provide minimal implementations of functions required
 * by controller.c and menu files, but don't require full game initialization.
 */

#include "z-term.h"
#include "z-util.h"
#include "h-type.h"
#include "angband.h"

/* Stub for Term_keypress - just returns success */
errr Term_keypress(int k) {
    (void)k; /* Suppress unused parameter warning */
    return 0; /* Success */
}

/* Stub for Term_putstr - does nothing */
errr Term_putstr(int x, int y, int n, byte a, cptr s) {
    (void)x; (void)y; (void)n; (void)a; (void)s;
    return 0;
}

/* Stub for Term_erase - does nothing */
errr Term_erase(int x, int y, int n) {
    (void)x; (void)y; (void)n;
    return 0;
}

/* Stub for path_build - simple string concatenation */
errr path_build(char *buf, int max, cptr path, cptr file) {
    if (!buf || !path || !file || max <= 0) return 1;

    size_t path_len = strlen(path);
    size_t file_len = strlen(file);

    if (path_len + file_len + 2 > max) {
        /* Truncate if needed */
        file_len = max - path_len - 2;
        if (file_len < 0) file_len = 0;
    }

    strncpy(buf, path, max - 1);
    buf[max - 1] = '\0';

    /* Add separator if path doesn't end with one */
    if (path_len > 0 && buf[path_len - 1] != '/' && buf[path_len - 1] != '\\') {
        if (path_len < max - 1) {
            buf[path_len] = '/';
            buf[path_len + 1] = '\0';
            path_len++;
        }
    }

    /* Append filename */
    if (path_len < max - 1) {
        strncat(buf, file, max - path_len - 1);
    }

    return 0;
}

/* Stub for my_fopen - wraps fopen */
FILE *my_fopen(cptr file, cptr mode) {
    return fopen(file, mode);
}

/* Stub for my_fclose - wraps fclose */
errr my_fclose(FILE *fff) {
    return fclose(fff);
}

/* Stub for my_fgets - wraps fgets */
errr my_fgets(FILE *fff, char *buf, huge len) {
    if (!fff || !buf || len <= 0) return 1;
    if (fgets(buf, (int)len, fff) == NULL) return 1;
    return 0;
}

/* Stub for ANGBAND_DIR_USER - provide a default value */
cptr ANGBAND_DIR_USER = NULL; /* Will be NULL for tests, which is fine */

/* Renderer tests link renderer.c without full game state. Keep these NULL so
 * renderer_is_wall() uses its test-map path and renderer_sync_from_player()
 * safely returns early.
 */
byte (*cave_feat)[DUNGEON_WID] = NULL;
s16b (*cave_o_idx)[DUNGEON_WID] = NULL;
s16b (*cave_m_idx)[DUNGEON_WID] = NULL;
player_type *p_ptr = NULL;
player_other *op_ptr = NULL;

s16b message_num(void) {
    return 0;
}

cptr message_str(s16b age) {
    (void)age;
    return "";
}

