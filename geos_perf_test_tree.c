#include <stdio.h>

#include "geos_perf.h"

/*************************************************************************
* PERFORMANCE TEST
*/

/* Variables where data lives between the setup/run/cleanup stages */
GEOSGeometryList points_random;
GEOSGeometryList points_regular;
GEOSGeometryList circles_regular;
GEOSSTRtree* tree;


/* Read any data we need, and create any structures */
static void setup(void)
{
    size_t i;
    geomlist_init(&points_random);
    geomlist_init(&points_regular);
    geomlist_init(&circles_regular);
    read_data_file("points_random_10000.wkt.gz", &points_random);
    read_data_file("points_regular_10000.wkt.gz", &points_regular);

    /* Calculate circles for the regular points */
    for (i = 0; i < geomlist_size(&points_regular); i++)
    {
        const GEOSGeometry* geom = geomlist_get(&points_regular, i);
        geomlist_push(&circles_regular, GEOSBuffer(geom, 25.0, 16));
    }

    /* populate tree with the random points */
    tree = GEOSSTRtree_create(10);
    for (i = 0; i < geomlist_size(&points_random); i++)
    {
        const GEOSGeometry* geom = geomlist_get(&points_random, i);
        GEOSSTRtree_insert(tree, geom, (void*)geom);
    }

}

/* no-op callback function */
static void tree_callback(void *item, void *userdata)
{
    return;
}

/* For each regular circle, find all the random points that hit
   its envelope
*/
static void run(void)
{
    size_t i;
    for (i = 0; i < geomlist_size(&circles_regular); i++)
    {
        const GEOSGeometry* geom = geomlist_get(&circles_regular, i);
        GEOSSTRtree_query(
            tree,          /* tree to query */
            geom,          /* geometry envelope to query with */
            tree_callback, /* callback function to run for each find */
            NULL);         /* extra user data to pass to callback */
    }
}

/* Clean up any remaining memory */
static void cleanup(void)
{
    GEOSSTRtree_destroy(tree);
    geomlist_free(&points_random);
    geomlist_free(&points_regular);
    geomlist_free(&circles_regular);
}

/*************************************************************************
* CONFIGURATION CALLBACK FUNCTION
*/

gp_test config_tree_points(void)
{
    gp_test test;
    test.name = "STRtree envelope query";
    test.description =
        "Load and create an STRtree for a set of points."
        "Query the tree with a fixed grid of circles, finding"
        "all indexed points for each circle. Exercises the"
        "STRtree envelope query.";
    test.func_setup = setup;
    test.func_run = run;
    test.func_cleanup = cleanup;
    test.count = 200;
    return test;
}

