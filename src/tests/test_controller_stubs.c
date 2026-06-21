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
byte (*cave_info)[256] = NULL;
s16b (*cave_o_idx)[DUNGEON_WID] = NULL;
s16b (*cave_m_idx)[DUNGEON_WID] = NULL;
object_type *o_list = NULL;
object_kind *k_info = NULL;
monster_type *m_list = NULL;
monster_race *r_info = NULL;
maxima *z_info = NULL;
s16b m_max = 1;
s16b m_cnt = 0;
s16b o_max = 1;
s16b o_cnt = 0;
player_type *p_ptr = NULL;
player_other *op_ptr = NULL;

s16b message_num(void) {
    return 0;
}

cptr message_str(s16b age) {
    (void)age;
    return "";
}

/* Stubs for controller_item_ui.c */
bool character_generated = FALSE;
bool game_in_progress = FALSE;
bool item_tester_full = FALSE;
static object_type inventory_stub[INVEN_TOTAL];
object_type *inventory = inventory_stub;
byte tval_to_attr[128];

void screen_save(void) {}
void screen_load(void) {}
void show_inven(void) {}
void show_equip(void) {}
bool item_tester_okay(const object_type *o_ptr) {
    (void)o_ptr;
    return TRUE;
}
void object_desc(char *buf, const object_type *o_ptr, int pref, int mode) {
    (void)o_ptr;
    (void)pref;
    (void)mode;
    if (buf) strcpy(buf, "item");
}
char index_to_label(int i) {
    return (char)('a' + (i % 26));
}
void prt(cptr s, int row, int col) {
    (void)s;
    (void)row;
    (void)col;
}
void put_str(cptr s, int row, int col) {
    (void)s;
    (void)row;
    (void)col;
}
void c_put_str(byte attr, cptr s, int row, int col) {
    (void)attr;
    (void)s;
    (void)row;
    (void)col;
}
errr Term_fresh(void) {
    return 0;
}
void bell(cptr str) {
    (void)str;
}
void msg_print(cptr str) {
    (void)str;
}
cptr mention_use(int i) {
    (void)i;
    return "slot";
}
s16b wield_slot(const object_type *o_ptr) {
    (void)o_ptr;
    return INVEN_WIELD;
}

bool store_is_shopping(void) {
    return FALSE;
}
int store_get_stock_count(void) {
    return 0;
}
int store_get_page_top(void) {
    return 0;
}
char store_get_item_label(int item) {
    return (char)('a' + (item % 26));
}
object_type *store_get_stock_item(int item) {
    (void)item;
    return NULL;
}
bool store_item_can_sell(const object_type *o_ptr) {
    (void)o_ptr;
    return FALSE;
}

