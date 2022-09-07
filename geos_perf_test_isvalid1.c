#include <stdio.h>

#include "geos_perf.h"

/*************************************************************************
* PERFORMANCE TEST
*/

/* Variables where data lives between the setup/run/cleanup stages */
static GEOSGeometryList australiaFile;
static GEOSGeometry *australia;

/* Read any data we need, and create any structures */
static void setup(void)
{
    geomlist_init(&australiaFile);
    read_data_file("australia.wkt.gz", &australiaFile);
    /* takes ownership of just the first geometry */
    australia = australiaFile.geoms[0];
    /* gently free just the containing geomlist */
    geomlist_release(&australiaFile);
}

/* For each run, union all the geometries in the collection */
static void run(void)
{
    char iv = GEOSisValid(australia);
}

/* Clean up any remaining memory */
static void cleanup(void)
{
    GEOSGeom_destroy(australia);
}

/*************************************************************************
* CONFIGURATION CALLBACK FUNCTION
*/

gp_test config_isvalid_australia(void)
{
    gp_test test;
    test.name = "Australia isValid";
    test.description =
        "Load a complex boundary of Australia with many"
        "offshore islands and calculate valididy of"
        "the multi-polygon.";
    test.func_setup = setup;
    test.func_run = run;
    test.func_cleanup = cleanup;
    test.count = 4;
    return test;
}

