#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLS 64
#define MAX_ROWS 4096
#define MAX_CELL 256

typedef struct {
    char cell[MAX_CELL];
} Cell;

typedef struct {
    Cell cols[MAX_COLS];
    int num_cols;
} Row;

typedef struct {
    Row rows[MAX_ROWS];
    int num_rows;
} Table;

Table* readCSV(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Could not open file: %s\n", filename);
        return NULL;
    }

    Table* table = calloc(1, sizeof(Table));
    if (!table) {
        fclose(f);
        return NULL;
    }

    char line[MAX_COLS * MAX_CELL];

    while (fgets(line, sizeof(line), f)) {
        if (table->num_rows >= MAX_ROWS) {
            fprintf(stderr, "Too many rows, truncating at %d\n", MAX_ROWS);
            break;
        }

        // Strip trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[--len] = '\0';
        if (len > 0 && line[len - 1] == '\r') line[--len] = '\0';

        Row* row = &table->rows[table->num_rows];
        char* token = strtok(line, ",");

        while (token && row->num_cols < MAX_COLS) {
            strncpy(row->cols[row->num_cols].cell, token, MAX_CELL - 1);
            row->cols[row->num_cols].cell[MAX_CELL - 1] = '\0';
            row->num_cols++;
            token = strtok(NULL, ",");
        }

        table->num_rows++;
    }

    fclose(f);
    return table;
}

void freeTable(Table* table) {
    free(table);
}

int main(void) {
    Table* data = readCSV("products.csv");
    if (!data) return 1;

    for (int i = 0; i < data->num_rows; i++) {
        Row* row = &data->rows[i];
        for (int j = 0; j < row->num_cols; j++) {
            printf("%s", row->cols[j].cell);
            if (j < row->num_cols - 1) printf(", ");
        }
        printf("\n");
    }

    freeTable(data);
    return 0;
}