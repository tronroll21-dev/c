#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shellapi.h>  /* CommandLineToArgvW */

#define MAX_FIELDS    64
#define MAX_FIELD_LEN 512   /* in UTF-16 code units */

/* ------------------------------------------------------------------ */
/* UTF-16 LE field storage                                              */
/* ------------------------------------------------------------------ */
typedef struct {
    uint16_t units[MAX_FIELD_LEN];  /* raw UTF-16 LE code units       */
    int      len;                   /* number of code units (not bytes)*/
} Field;

/* ------------------------------------------------------------------ */
/* Read one UTF-16 LE code unit (2 bytes, little-endian) from f.       */
/* Returns 0 on success, -1 on EOF/error.                              */
/* ------------------------------------------------------------------ */
static int read_u16(FILE *f, uint16_t *out)
{
    uint8_t b[2];
    if (fread(b, 1, 2, f) != 2) return -1;
    *out = (uint16_t)(b[0] | (b[1] << 8));
    return 0;
}

/* ------------------------------------------------------------------ */
/* Split one UTF-16 LE line into tab-separated fields.                 */
/* Call with the FILE positioned just after the previous newline       */
/* (or at the BOM for the first call).                                 */
/*                                                                     */
/* Returns number of fields parsed, or -1 on EOF with nothing read.   */
/* ------------------------------------------------------------------ */
int parse_tsv_row(FILE *f, Field fields[], int max_fields)
{
    int  fi  = 0;   /* current field index   */
    int  len = 0;   /* current field length  */
    int  got = 0;   /* have we read anything?*/
    int  first = 1; /* for BOM skip          */

    while (1) {
        uint16_t cu;
        if (read_u16(f, &cu) != 0) {
            /* EOF: flush the last field if we read something */
            if (got) {
                fields[fi].units[len] = 0;
                fields[fi].len = len;
                return fi + 1;
            }
            return -1;  /* nothing read at all */
        }

        got = 1;

        /* Skip BOM (0xFEFF) at the very start of the file */
        if (first && cu == 0xFEFF) { first = 0; continue; }
        first = 0;

        if (cu == 0x0009) {         /* TAB — end of field */
            fields[fi].units[len] = 0;
            fields[fi].len = len;
            fi++;
            len = 0;
            if (fi >= max_fields) return fi;
        } else if (cu == 0x000D) {  /* CR — skip (handle CRLF) */
            continue;
        } else if (cu == 0x000A) {  /* LF — end of row */
            fields[fi].units[len] = 0;
            fields[fi].len = len;
            return fi + 1;
        } else {                    /* normal character */
            if (len < MAX_FIELD_LEN - 1)
                fields[fi].units[len++] = cu;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Print a UTF-16 LE field to stdout as UTF-8.                         */
/*                                                                     */
/* For portability this manual encoder is used instead of iconv or     */
/* Windows APIs, so the program compiles and runs on Linux too.        */
/* ------------------------------------------------------------------ */
static void print_field_utf8(const Field *f)
{
    for (int i = 0; i < f->len; i++) {
        uint32_t cp = f->units[i];

        /* Surrogate pair? Decode to full code point. */
        if (cp >= 0xD800 && cp <= 0xDBFF) {
            if (i + 1 < f->len) {
                uint32_t low = f->units[++i];
                cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
            }
        }

        /* Encode as UTF-8 */
        if (cp <= 0x7F) {
            putchar((int)cp);
        } else if (cp <= 0x7FF) {
            putchar(0xC0 | (cp >> 6));
            putchar(0x80 | (cp & 0x3F));
        } else if (cp <= 0xFFFF) {
            putchar(0xE0 | (cp >> 12));
            putchar(0x80 | ((cp >> 6) & 0x3F));
            putchar(0x80 | (cp & 0x3F));
        } else {
            putchar(0xF0 | (cp >> 18));
            putchar(0x80 | ((cp >> 12) & 0x3F));
            putchar(0x80 | ((cp >> 6)  & 0x3F));
            putchar(0x80 | (cp & 0x3F));
        }
    }
}

/* ------------------------------------------------------------------ */
/* main                                                                 */
/* ------------------------------------------------------------------ */
int main(void)
{
        
    int     argc;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (argc < 2) {
        fwprintf(stderr, L"Usage: parse_tsv <search_string>\n");
        return 1;
    }
    
    const wchar_t *path = (argc > 2) ? argv[2] : L"test.tsv";
    FILE *f = _wfopen(path, L"rb");
    if (!f) { perror("fopen"); return 1; }
        
    /* argv[1] is already UTF-16 — compare directly against field 1 */
    LPWSTR search = argv[1];
    int    search_len = wcslen(search);

    LocalFree(argv);  /* required by CommandLineToArgvW */

    Field fields[MAX_FIELDS];
    int   row = 0;

    while (1) {
        int n = parse_tsv_row(f, fields, MAX_FIELDS);
        if (n < 0) break;   /* EOF */

        // printf("=== Row %d (%d field%s) ===\n", row, n, n == 1 ? "" : "s");
        // for (int i = 0; i < n; i++) {
        //     printf("  [%d] (len=%d) \"", i, fields[i].len);
        //     print_field_utf8(&fields[i]);
        //     printf("\"\n");
        // }

        /* Compare field[1] against the search string */
        if (fields[1].len == search_len &&
            memcmp(fields[1].units, search, search_len * sizeof(uint16_t)) == 0)
        {
            /* match — print the row */

            printf("row %d: %ls\n", row, search);
            for (int i = 0; i < n; i++) {
                printf("  [%d] (len=%d) \"", i, fields[i].len);
                print_field_utf8(&fields[i]);
                printf("\"\n");
            }
        }
         row++;
    }

    fclose(f);
    return 0;
}