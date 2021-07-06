#include <stdio.h>

#include "geos_perf.h"

/*************************************************************************
* PERFORMANCE TEST
*/

/* GEOS <= 3.5 lacks GEOSSTRtree_nearest() */
#if GEOS_VERSION_CMP > 305

/* Variables where data lives between the setup/run/cleanup stages */
GEOSGeometryList points_random;
GEOSGeometryList points_regular;
GEOSSTRtree* tree;


/* Read any data we need, and create any structures */
static void setup(void)
{
    size_t i;
    geomlist_init(&points_random);
    geomlist_init(&points_regular);
    read_data_file("points_random_10000.wkt.gz", &points_random);
    read_data_file("points_regular_10000.wkt.gz", &points_regular);
    /* tree does not take ownership of inputs */
    tree = GEOSSTRtree_create(10);
    /* populate tree with random points */
    for (i = 0; i < geomlist_size(&points_random); i++)
    {
        const GEOSGeometry* geom = geomlist_get(&points_random, i);
        GEOSSTRtree_insert(tree, geom, (void*)geom);
    }

}

/* For each regular point, find the nearest random point */
static void run(void)
{
    size_t i;
    for (i = 0; i < geomlist_size(&points_regular); i++)
    {
        const GEOSGeometry* geom = geomlist_get(&points_regular, i);
        const GEOSGeometry* nearest = GEOSSTRtree_nearest(tree, geom);
    }
}

/* Clean up any remaining memory */
static void cleanup(void)
{
    GEOSSTRtree_destroy(tree);
    geomlist_free(&points_random);
    geomlist_free(&points_regular);
}

/*************************************************************************
* CONFIGURATION CALLBACK FUNCTION
*/

gp_test config_tree_points_nn(void)
{
    gp_test test;
    test.name = "STRtree nearest-neighbor for points";
    test.description =
        "Load and create an STRtree for a set of points."
        "Query the tree with a fixed grid of points, finding"
        "the nearest point for each input. Exercises the"
        "STRtree nearest query.";
    test.func_setup = setup;
    test.func_run = run;
    test.func_cleanup = cleanup;
    test.count = 20;
    return test;
}

#else

GEOS_PERF_SKIP(config_tree_points_nn);

#endif
