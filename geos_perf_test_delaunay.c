#include <stdio.h>
#include <float.h>

#include "geos_perf.h"

/*************************************************************************
* PERFORMANCE TEST
*/

static GEOSGeometry* mpoint = NULL;

//SELECT ST_AsText(ST_Collect(ST_Point(1000*Random(), 1000*Random())),5)
//FROM Generate_series(1,1000);

/* Read any data we need, and create any structures */
static void
setup(void)
{
    mpoint = read_geometry_file("multipoint_random_1000.wkt.gz");
}


/* Build a triangulation of the input multipoint */
static void
run(void)
{
    GEOSGeometry *delaunay = NULL;

    if (mpoint)
        delaunay = GEOSDelaunayTriangulation(mpoint, 0.0, 1);

    if (delaunay)
        GEOSGeom_destroy(delaunay);
}

/* Clean up any remaining memory */
static void
cleanup(void)
{
    if (mpoint) GEOSGeom_destroy(mpoint);
}

/*************************************************************************
* CONFIGURATION CALLBACK FUNCTION
*/

gp_test config_delaunay(void)
{
    gp_test test;
    test.name = "Delaunay";
    test.description =
        "Load a multipoint, and build a delaunay triangulation"
        "of the points.";
    test.func_setup = setup;
    test.func_run = run;
    test.func_cleanup = cleanup;
    test.count = 2000;
    return test;
}

