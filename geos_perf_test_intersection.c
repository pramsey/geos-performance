#include <stdio.h>

#include "geos_perf.h"

/*************************************************************************
* PERFORMANCE TEST
*/

/* Variables where data lives between the setup/run/cleanup stages */
GEOSGeometryList watersheds;
GEOSGeometryList watersheds_buffered;
GEOSSTRtree* tree;


/* Read any data we need, and create any structures */
static void setup(void)
{
    size_t i;
    geomlist_init(&watersheds);
    geomlist_init(&watersheds_buffered);
    read_data_file("watersheds.wkt.gz", &watersheds);

    /* Calculate circles for the regular points */
    for (i = 0; i < geomlist_size(&watersheds); i++)
    {
        const GEOSGeometry* geom = geomlist_get(&watersheds, i);
        geomlist_push(&watersheds_buffered, GEOSBuffer(geom, 100.0, 16));
    }

    /* populate tree watersheds */
    tree = GEOSSTRtree_create(10);
    for (i = 0; i < geomlist_size(&watersheds); i++)
    {
        const GEOSGeometry* geom = geomlist_get(&watersheds, i);
        GEOSSTRtree_insert(tree, geom, (void*)geom);
    }

}

/* no-op callback function */
static void tree_callback(void *item, void *userdata)
{
    GEOSGeometryList* query_result = (GEOSGeometryList*)userdata;
    GEOSGeometry* geom = (GEOSGeometry*)item;
    geomlist_push(query_result, geom);
    return;
}

/* For each run, union all the geometries in the collection */
static void run(void)
{
    size_t i, j;
    GEOSGeometryList query_result;
    for (i = 0; i < geomlist_size(&watersheds_buffered); i++)
    {
        const GEOSGeometry* bufshed = geomlist_get(&watersheds_buffered, i);
        geomlist_init(&query_result);
        GEOSSTRtree_query(
            tree,           /* tree to query */
            bufshed,        /* geometry envelope to query with */
            tree_callback,  /* callback function to run for each find */
            &query_result); /* extra user data to pass to callback */

        for (j = 0; j < geomlist_size(&query_result); j++)
        {
            const GEOSGeometry* shed = geomlist_get(&query_result, j);
            GEOSGeometry* inter = GEOSIntersection(shed, bufshed);
            GEOSGeom_destroy(inter);
        }
        geomlist_release(&query_result);
    }
}

/* Clean up any remaining memory */
static void cleanup(void)
{
    GEOSSTRtree_destroy(tree);
    geomlist_free(&watersheds_buffered);
    geomlist_free(&watersheds);
}

/*************************************************************************
* CONFIGURATION CALLBACK FUNCTION
*/

gp_test config_intersection(void)
{
    gp_test test;
    test.name = "Waterhsed intersections";
    test.description =
        "Load and create an STRtree for the watersheds."
        "Set up a list of buffered watersheds."
        "For each buffered watershed, calculate intersection with"
        "all watersheds found by querying the tree.";
    test.func_setup = setup;
    test.func_run = run;
    test.func_cleanup = cleanup;
    test.count = 1;
    return test;
}

