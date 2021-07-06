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
    /* Geometry list always must be initialized before use */
    /* so the internal array is prepared, and counters zero'ed out */
    geomlist_init(&watersheds);
    /* Use utility function to read data file into geometry list */
    read_data_file("watersheds.wkt.gz", &watersheds);
}

/* For each run, valid test each geometry in the collection */
static void run(void)
{
    size_t i;
    for (i = 0; i < geomlist_size(&watersheds); i++)
    {
        const GEOSGeometry* g = geomlist_get(&watersheds, i);
        char valid = GEOSisValid(g);
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

gp_test config_valid_watersheds(void)
{
    gp_test test;
    test.name = "Watershed isValid";
    test.description =
        "Test validity for each watershed polygon.";
    test.func_setup = setup;
    test.func_run = run;
    test.func_cleanup = cleanup;
    test.count = 15;
    return test;
}

