#ifndef TABLE_H
#define TABLE_H

#include "utils.h"
#include <uchar.h>

/*
Utility for working with tables, especially for console output.
*/

typedef struct{
    uint8_t rows;
    uint8_t cols;
    char *name;
    bool *row_has_format_line; // Element i being true implies a format line after row i
    char **cells;
} table_t;

/* Create a new table */
table_t *new_table(uint8_t r, uint8_t c, char *name);

/*
Mark a row as having no format line after it.

@param table Table to modify
@param r Row to modify
@return True on success, i.e. when `r`is valid in `table`
*/
bool table_no_format_line(table_t *table, uint8_t r);

/*
Set the contents of a table's cell

@param table Table to modify
@param r Row to modify
@param c Column to modify
@param contents String to write to cell; is not copied
@return Whether the cell could be updated,
i.e. whether the cell requested is valid in the table
*/
bool table_set_cell(table_t *table, uint8_t r, uint8_t c, char *contents);

/*
Get the contents of a table's cell
@param table Table to read
@param r Row to read
@param c Column to read
@return Contents of cell or NULL if cell was invalid.
Note that NULL may be returned if the cell contained NULL.
*/
char *table_get_cell(table_t *table, uint8_t r, uint8_t c);

/*
Get a table's string representation

@param table Table to convert to string
@param fmt Whether to use escape sequences for text formatting. When false, `tcfmt`, `hrfmt` and `cellfmt` have no effect.
@param tcfmt Table Character Format - Escape sequence to apply to each character constituting the table lines
@param hrfmt Header Row Format - Escape sequence to apply to cell contents of the first row
@param cellfmt Cell Format - Escape sequence to apply to all cells except the first row
@return Allocated string containing printable table
*/
char *dump_table(table_t *table, bool fmt, const char *tcfmt, const char *hrfmt, const char *cellfmt);

/* Free an existing table */
void free_table(table_t *table);

#endif // TABLE_H
