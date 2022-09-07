#include <stdio.h>

#include "geos_perf.h"

/*************************************************************************
* PERFORMANCE TEST
*/

/* Variables where data lives between the setup/run/cleanup stages */
static GEOSGeometryList watersheds;
static GEOSGeometry *collection;

/* Read any data we need, and create any structures */
static void setup(void)
{
    geomlist_init(&watersheds);
    read_data_file("watersheds.wkt.gz", &watersheds);
    /* collection takes ownership of all geometries */
    collection = GEOSGeom_createCollection(
        GEOS_GEOMETRYCOLLECTION,
        watersheds.geoms,
        watersheds.ngeoms);
    /* gently free just the containing geomlist */
    geomlist_release(&watersheds);
}

/* For each run, union all the geometries in the collection */
static void run(void)
{
    GEOSGeometry* geom = GEOSUnaryUnion(collection);
    GEOSGeom_destroy(geom);
}

/* Clean up any remaining memory */
static void cleanup(void)
{
    /* geomlist is already cleaned, just need to get rid of collection */
    GEOSGeom_destroy(collection);
}

/*************************************************************************
* CONFIGURATION CALLBACK FUNCTION
*/

gp_test config_union_watersheds(void)
{
    gp_test test;
    test.name = "Watershed unary union";
    test.description =
        "Load a collection of watershed boundaries"
        "and use unary union to merge them into a"
        "single final multi-polygon. Exercises"
        "cascaded union and overlay.";
    test.func_setup = setup;
    test.func_run = run;
    test.func_cleanup = cleanup;
    test.count = 2;
    return test;
}

