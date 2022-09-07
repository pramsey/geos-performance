#include <stdio.h>

#include "geos_perf.h"

/*************************************************************************
* PERFORMANCE TEST
*/

/* Variables where data lives between the setup/run/cleanup stages */
static GEOSGeometryList land_covers;

/* Read any data we need, and create any structures */
static void setup(void)
{
    /* Geometry list always must be initialized before use */
    /* so the internal array is prepared, and counters zero'ed out */
    geomlist_init(&land_covers);
    /* Use utility function to read data file into geometry list */
    read_data_file("invalid_land_cover.wkt.gz", &land_covers);
}

/* For each run, buffer each geometry in the collection */
static void run(void)
{
    size_t i;
    for (i = 0; i < geomlist_size(&land_covers); i++)
    {
        const GEOSGeometry* g = geomlist_get(&land_covers, i);
        char isvalid = GEOSisValid(g);
    }
}

/* Clean up any remaining memory */
static void cleanup(void)
{
    geomlist_free(&land_covers);
}

/*************************************************************************
* CONFIGURATION CALLBACK FUNCTION
*/

gp_test config_isvalid_landcover(void)
{
    gp_test test;
    test.name = "Landcover isValid";
    test.description =
        "Test validity for each landcover polygon.";
    test.func_setup = setup;
    test.func_run = run;
    test.func_cleanup = cleanup;
    test.count = 20;
    return test;
}

