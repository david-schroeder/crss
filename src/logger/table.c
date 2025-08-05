#include "table.h"

table_t *new_table(uint8_t r, uint8_t c, char *name) {
    if (r == 0 || c == 0) return NULL;
    table_t *t = malloc(sizeof(table_t));
    t->rows = r;
    t->cols = c;
    t->cells = calloc(r*c, sizeof(char *));
    t->row_has_format_line = malloc(r);
    memset(t->row_has_format_line, true, r);
    t->name = name;
    return t;
}

bool table_set_cell(table_t *table, uint8_t r, uint8_t c, char *contents) {
    if (r >= table->rows) return false;
    if (c >= table->cols) return false;
    table->cells[r * table->cols + c] = contents;
    return true;
}

bool table_no_format_line(table_t *table, uint8_t r) {
    if (r >= table->rows - 1) return false;
    table->row_has_format_line[r] = false;
    return true;
}

char *table_get_cell(table_t *table, uint8_t r, uint8_t c) {
    if (r >= table->rows) return NULL;
    if (c >= table->cols) return NULL;
    return table->cells[r * table->cols + c];
}

const char *boxchar_topleft     = "\u250C";
const char *boxchar_topright    = "\u2510";
const char *boxchar_bottomleft  = "\u2514";
const char *boxchar_bottomright = "\u2518";
const char *boxchar_t_left      = "\u251C";
const char *boxchar_t_top       = "\u252C";
const char *boxchar_t_right     = "\u2524";
const char *boxchar_t_bottom    = "\u2534";
const char *boxchar_cross       = "\u253C";
const char *boxchar_line        = "\u2500";
const char *boxchar_bar         = "\u2502";

#define max(a, b) a > b ? a : b

static size_t byte_len(char *buf) {
    size_t l = 0;
    char c;
    while ((c = *buf++)) l++;
    return l;
}

static size_t display_length(char *s) {
    size_t l = 0;
    char c;
    bool in_esc_seq = false; /* Is in escape sequence */
    while ((c = *s++)) {
        if (c == '\033') in_esc_seq = true;
        if (!in_esc_seq) l++;
        if (in_esc_seq && c == 'm') in_esc_seq = false;
    }
    return l;
}

// TODO: wrapping
char *dump_table(table_t *table, bool fmt, const char *tcfmt, const char *hrfmt, const char *cellfmt) {
    /* Find widths of all columns and of table */
    uint16_t *col_widths = malloc(table->cols * sizeof(uint16_t));
    uint16_t col_width; // Length of one column at a time
    uint16_t cell_width; // Length of one cell at a time

    char *cell = NULL;
    for (int c = 0; c < table->cols; c++) {
        col_width = 5;
        for (int r = 0; r < table->rows; r++) {
            if ((cell = table_get_cell(table, r, c)) == NULL) continue;
            cell_width = display_length(cell) + 2;
            col_width = max(cell_width, col_width);
        }
        col_widths[c] = col_width;
    }

    /* Output string */
    char *dump_str = NULL;
    size_t dump_str_sz = 0;

    /* Helper/Builder strings */
    #define ROW_VCHARS 1024
    char next_row[ROW_VCHARS];
    char header_row[ROW_VCHARS];
    char format_row[ROW_VCHARS];
    char footer_row[ROW_VCHARS];
    char next_cell[ROW_VCHARS];
    memset(&header_row, '\0', ROW_VCHARS);
    memset(&format_row, '\0', ROW_VCHARS);
    memset(&footer_row, '\0', ROW_VCHARS);
    header_row[0] = ' ';
    format_row[0] = ' ';
    footer_row[0] = ' ';
    if (fmt) {
        strcat(header_row, tcfmt);
        strcat(format_row, tcfmt);
        strcat(footer_row, tcfmt);
    }
    
    /* Table format rows */
    for (int c = 0; c < table->cols; c++) {
        if (c == 0) {
            strcat(header_row, boxchar_topleft);
            strcat(format_row, boxchar_t_left);
            strcat(footer_row, boxchar_bottomleft);
        } else {
            strcat(header_row, boxchar_t_top);
            strcat(format_row, boxchar_cross);
            strcat(footer_row, boxchar_t_bottom);
        }
        for (int i = 0; i < col_widths[c]; i++) {
            strcat(header_row, boxchar_line);
            strcat(format_row, boxchar_line);
            strcat(footer_row, boxchar_line);
        }
    }
    strcat(header_row, boxchar_topright);
    strcat(format_row, boxchar_t_right);
    strcat(footer_row, boxchar_bottomright);

    /* Add header row to output once */
    dump_str_sz += byte_len(header_row) + 2; // \n + trailing zero
    dump_str = calloc(dump_str_sz, sizeof(char));

    strcat(dump_str, (char *)header_row);
    strcat(dump_str, "\n");

    /* Add cells to output */
    for (int r = 0; r < table->rows; r++) {
        memset(&next_row, '\0', ROW_VCHARS);
        next_row[0] = ' ';
        for (int c = 0; c < table->cols; c++) {
            /* Cell delimiter */
            if (fmt) strcat(next_row, tcfmt);
            strcat(next_row, boxchar_bar);
            if (fmt) strcat(next_row, r == 0 ? hrfmt : cellfmt);

            /* Cell contents */
            char *cell_contents = table_get_cell(table, r, c);
            cell_width = cell_contents ? display_length(cell_contents) : 0;
            if (cell_contents) sprintf(next_cell, " %s ", cell_contents);
            else sprintf(next_cell, "  ");
            /* Cell Padding */
            for (int i = 0; i < col_widths[c]-cell_width-2; i++) {
                strcat(next_cell, " ");
            }
            /* Adding cell to row */
            strcat(next_row, next_cell);
        }
        /* Cell delimiter at end of row */
        if (fmt) strcat(next_row, tcfmt);
        strcat(next_row, boxchar_bar);
        strcat(next_row, "\033[0m\n");
        
        /* Adding row to output */
        dump_str_sz += byte_len(next_row) + byte_len(format_row) + 1;
        dump_str = realloc(dump_str, dump_str_sz);
        strcat(dump_str, next_row);

        /* Format row */
        if (table->row_has_format_line[r] || r == table->rows - 1) {
            strcat(dump_str, r == table->rows - 1 ? footer_row : format_row);
            strcat(dump_str, "\n");
        }
    }

    free(col_widths);

    return dump_str;
}

void free_table(table_t *table) {
    free(table->cells);
    free(table->row_has_format_line);
    free(table);
}

int table_main() {
    table_t *t = new_table(3, 3, "Example table");
    table_set_cell(t, 0, 0, "header 1");
    table_set_cell(t, 0, 1, "header 2");
    table_set_cell(t, 0, 2, "raspberry chocolate cake");
    table_set_cell(t, 1, 0, "hmmm");
    table_set_cell(t, 1, 1, "X");
    table_set_cell(t, 1, 2, "sigma");
    table_set_cell(t, 2, 0, "entry");
    table_set_cell(t, 2, 2, "WAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    table_no_format_line(t, 1);
    char *dumped = dump_table(t, true, "\033[0m\033[2m", "\033[36;1m", "\033[0m");
    printf("%s\n", dumped);
    free(dumped);
    free_table(t);
    return 0;
}
