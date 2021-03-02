#include <stdio.h>

#include "geos_perf.h"

/*************************************************************************
* PERFORMANCE TEST
*/

/* Variables where data lives between the setup/run/cleanup stages */
GEOSGeometryList watersheds;

/* Read any data we need, and create any structures */
static void setup(void)
{
    geomlist_init(&watersheds);
    read_data_file("watersheds.wkt.gz", &watersheds);
}

/* For each run, buffer each geometries in the collection */
static void run(void)
{
    size_t i;
    for (i = 0; i < geomlist_size(&watersheds); i++)
    {
        const GEOSGeometry* g = geomlist_get(&watersheds, i);
        GEOSGeometry* buffer = GEOSBuffer(
            g,     /* input geometry */
            100.0, /* buffer size */
            24     /* quadsegs */
            );
        GEOSGeom_destroy(buffer);
    }
}

/* Clean up any remaining memory */
static void cleanup(void)
{
    geomlist_free(&watersheds);
}

/*************************************************************************
* CONFIGURATION CALLBACK FUNCTION
*/

gp_test config_buffer_watersheds(void)
{
    gp_test test;
    test.name = "Watershed buffer";
    test.description =
        "Generate buffers for complex watershed polygons."
        "Exercises the buffer function.";
    test.func_setup = setup;
    test.func_run = run;
    test.func_cleanup = cleanup;
    test.count = 5;
    return test;
}

