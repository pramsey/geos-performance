#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <zlib.h>

#include "geos_perf.h"

/* CHUNK_SIZE is the size of the memory chunk used by the zlib routines. */
#define CHUNK_SIZE 0x4000

/* The following macro calls a zlib routine and checks the return
   value. If the return value ("status") is not OK, it prints an error
   message and exits the program. Zlib's error statuses are all less
   than zero. */

#define CALL_ZLIB(x) {                                                  \
        int status;                                                     \
        status = x;                                                     \
        if (status < 0) {                                               \
            fprintf (stderr,                                            \
                     "%s:%d: %s returned a bad status of %d.\n",        \
                     __FILE__, __LINE__, #x, status);                   \
            exit (EXIT_FAILURE);                                        \
        }                                                               \
    }

/* if "test" is true, print an error message and halt execution. */

#define FAIL(test,file_name,message) {                   \
        if (test) {                                      \
            inflateEnd (& strm);                         \
            fprintf (stderr, "%s:%d: " message           \
                     " file '%s' failed: %s\n",          \
                     __FILE__, __LINE__, file_name,      \
                     strerror (errno));                  \
            exit (EXIT_FAILURE);                         \
        }                                                \
    }

#define TFAIL(test,message) {                            \
        if (test) {                                      \
            fprintf (stderr, "%s:%d: " message           \
                     " file '%s' failed: %s\n",          \
                     __FILE__, __LINE__, file_name,      \
                     strerror (errno));                  \
            exit (EXIT_FAILURE);                         \
        }                                                \
    }


/* These are parameters to inflateInit2. See
   http://zlib.net/manual.html for the exact meanings. */
#define INFLATE_WINDOW_BITS 15
#define ENABLE_ZLIB_GZIP 32

#define TMPFILE "/tmp/geos_perf_tmp"

static int
decompress_data_file(const char* file_name, const char* out_file_name)
{
    FILE * file;
    FILE * outfile;
    z_stream strm = {0};
    unsigned char in[CHUNK_SIZE];
    unsigned char out[CHUNK_SIZE];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = in;
    strm.avail_in = 0;
    CALL_ZLIB (inflateInit2(&strm, INFLATE_WINDOW_BITS | ENABLE_ZLIB_GZIP));

    /* Open the file. */
    file = fopen(file_name, "rb");
    FAIL (!file, file_name, "open input");
    outfile = fopen(out_file_name, "w");
    FAIL (!outfile, out_file_name, "open output");
    while (1)
    {
        int bytes_read;
        int zlib_status;

        bytes_read = fread (in, sizeof (char), sizeof (in), file);
        FAIL (ferror (file), file_name, "read");
        strm.avail_in = bytes_read;
        strm.next_in = in;

        do {
            unsigned have;
            strm.avail_out = CHUNK_SIZE;
            strm.next_out = out;
            zlib_status = inflate (& strm, Z_NO_FLUSH);
            switch (zlib_status)
            {
                case Z_OK:
                case Z_STREAM_END:
                case Z_BUF_ERROR:
                    break;

                default:
                    inflateEnd (& strm);
                    fprintf (stderr, "Gzip error %d in '%s'.\n", zlib_status, file_name);
                    return -1;
            }
            have = CHUNK_SIZE - strm.avail_out;
            /* Copy the decompressed output to the landing location */
            fwrite (out, sizeof(unsigned char), have, outfile);
        } while (strm.avail_out == 0);

        if (feof(file)) {
            inflateEnd(&strm);
            break;
        }
    }
    FAIL (fclose (file), file_name, "close input");
    FAIL (fclose (outfile), out_file_name, "close output");
    return 0;
}

int
read_data_file(const char* file_name, GEOSGeometryList* geoms)
{
    char full_file_name[MAXSTRLEN];
    snprintf(full_file_name, MAXSTRLEN, "%s/%s", DATA_DIR, file_name);
    int read_rv = decompress_data_file(full_file_name, TMPFILE);

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    FILE * file = fopen(TMPFILE, "r");
    TFAIL (!file, "open tmp input");

    GEOSWKTReader* reader = GEOSWKTReader_create();
    while ((linelen = getline(&line, &linecap, file)) > 0)
    {
        GEOSGeometry* g = GEOSWKTReader_read(reader, line);
        if (g)
        {
            size_t glsz = geomlist_push(geoms, g);
        }
    }
    GEOSWKTReader_destroy(reader);

    TFAIL (fclose (file), "close input");

    return 0;
}


