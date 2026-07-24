// ============================================================================
// EPUB HTML sumsize
// ----------------------------------------------------------------------------
// This program opens an EPUB file (which is internally a ZIP archive), iterates
// through all the files it contains, and sums the uncompressed size of all
// relevant text files:
//
//     *.html
//     *.htm
//     *.xhtml
//
// The program ALWAYS outputs a single number to stdout:
//
//   - If the number is zero or positive → total byte count of all HTML files.
//   - If the number is negative → error code.
//
// This allows KOReader to easily read the output via io.popen().
//
// ----------------------------------------------------------------------------
// Error codes (stdout):
//
//   -1  → No .epub extension or no argument provided.
//   -2  → File does not exist on the filesystem.
//   -3  → Cannot open the file as ZIP (mz_zip_reader_init_file failed).
//   -4  → Unexpected generic error.
//   -5  → Corrupted EPUB: ZIP opens but the central directory cannot be read.
//   -6  → Valid EPUB but contains no HTML/HTM/XHTML files.
//
// ----------------------------------------------------------------------------
// Minimal dependencies:
//   <stdio.h>
//   <string.h>
//   <stdlib.h>
//   "miniz.h"
//
// No additional libraries are required.
// ============================================================================




#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "miniz.h"


// Comprueba si un string termina con un sufijo (case-insensitive)
int ends_with(const char *str, const char *suffix) {
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr) return 0;
    return strcasecmp(str + lenstr - lensuffix, suffix) == 0;
}

int main(int argc, char **argv) {

    // ============================
    // 1. Argument check
    // ============================
    if (argc < 2) {
        printf("-1\n");  // No file -> same as file is not EPUB
        return 0;
    }

    const char *epub_path = argv[1];

    // ============================
    // 2. Check for .epub filetype
    // ============================
    if (!ends_with(epub_path, ".epub")) {
        printf("-1\n");  // No es EPUB
        return 0;
    }

    // ============================
    // 3. Check file existence
    // ============================
    FILE *test = fopen(epub_path, "rb");
    if (!test) {
        printf("-2\n");  // Archivo no encontrado
        return 0;
    }
    fclose(test);

    // ============================
    // 4. Try to open file
    // ============================
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));

    if (!mz_zip_reader_init_file(&zip, epub_path, 0)) {
        printf("-3\n");  // No se puede abrir como ZIP
        return 0;
    }

    // ============================
    // 5. Get total files
    // ============================
    mz_uint num_files = mz_zip_reader_get_num_files(&zip);
    if (num_files == 0) {
        mz_zip_reader_end(&zip);
        printf("-5\n");  // EPUB corrupto o sin directorio central
        return 0;
    }

    // ============================
    // 6. Sum HTML sizes
    // ============================
    size_t total_bytes = 0;
    int found_html = 0;

    for (mz_uint i = 0; i < num_files; i++) {
        mz_zip_archive_file_stat stat;

        // Si falla leer el stat → ZIP corrupto
        if (!mz_zip_reader_file_stat(&zip, i, &stat)) {
            mz_zip_reader_end(&zip);
            printf("-5\n");
            return 0;
        }

        const char *name = stat.m_filename;

        if (ends_with(name, ".html") ||
            ends_with(name, ".htm") ||
            ends_with(name, ".xhtml")) {

            found_html = 1;
            total_bytes += stat.m_uncomp_size;
        }
    }

    mz_zip_reader_end(&zip);

    // ============================
    // 7. Check if there were any HTML files
    // ============================
    if (!found_html) {
        printf("-6\n");  // EPUB válido pero sin HTML
        return 0;
    }

    // ============================
    // 8. Success
    // ============================
    printf("%zu\n", total_bytes);
    return 0;
}
