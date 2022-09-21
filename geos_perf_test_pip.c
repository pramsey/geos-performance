#include <stdio.h>
#include <float.h>
#include <dlfcn.h>

#include "geos_perf.h"

/*************************************************************************
* PERFORMANCE TEST
*/

/* Variables where data lives between the setup/run/cleanup stages */
static GEOSGeometryList watersheds;
static GEOSGeometryList prepared_watersheds;
static GEOSSTRtree* tree;

extern char GEOSPreparedContainsXY(const GEOSPreparedGeometry*, double x, double y);

static char old_prep_contains_xy(const GEOSPreparedGeometry* pg, double x, double y)
{
    GEOSGeometry* pt = createPointFromXY(x, y);
    char in = GEOSPreparedContains(pg, pt);
    GEOSGeom_destroy(pt);
    return in;
}

static char (*prep_contains_xy)(const GEOSPreparedGeometry* pg, double x, double y) = NULL;

/* Read any data we need, and create any structures */
static void
setup(void)
{
    size_t i;
    double xmin, xmax, ymin, ymax;
    xmin = ymin = FLT_MAX;
    xmax = ymax = -1 * FLT_MAX;
    geomlist_init(&watersheds);
    read_data_file("watersheds.wkt.gz", &watersheds);

    if (geos_version_major() >= 3 && geos_version_minor() >= 12) {
        prep_contains_xy = dlsym(geos_lib_handle, "GEOSPreparedContainsXY");
        if (!prep_contains_xy) {
            debug_stderr(0, "%s", " Failed to load GEOSPreparedContainsXY ");
        }
        else {
            debug_stderr(0, "%s", " Loaded GEOSPreparedContainsXY ");
        }
    }
    if (!prep_contains_xy) {
        prep_contains_xy = old_prep_contains_xy;
    }
}


static void
getGeometryBounds(const GEOSGeometry* g, double* xmin, double* ymin, double* xmax, double* ymax)
{
    uint32_t i, npoints;
    GEOSGeometry* env = GEOSEnvelope(g);
    const GEOSGeometry* ring = GEOSGetExteriorRing(env);
    const GEOSCoordSequence* cs = GEOSGeom_getCoordSeq(ring);
    GEOSCoordSeq_getSize(cs, &npoints);
    *xmin = *ymin = FLT_MAX;
    *xmax = *ymax = -1 * FLT_MAX;
    for (i = 0; i < npoints; i++)
    {
        double d;
        GEOSCoordSeq_getX(cs, i, &d);
        *xmin = d < *xmin ? d : *xmin;
        *xmax = d > *xmax ? d : *xmax;
        GEOSCoordSeq_getY(cs, i, &d);
        *ymin = d < *ymin ? d : *ymin;
        *ymax = d > *ymax ? d : *ymax;
    }
    GEOSGeom_destroy(env);
}

static int contains_test_kernel_1(const GEOSPreparedGeometry* prepgeom, double x, double y) {
    GEOSGeometry* pt = createPointFromXY(x, y);
    char in = GEOSPreparedContains(prepgeom, pt);
    GEOSGeom_destroy(pt);
    return 1;
}


static int contains_test_kernel_2(const GEOSPreparedGeometry* prepgeom, double x, double y) {


    char in = prep_contains_xy(prepgeom, x, y);
    return 1;
}

/* For each watershed, prepare the geometry, then hit it with
   a bunch of points for in/out test */
static void
run(void)
{

    size_t i;
    for (i = 0; i < geomlist_size(&watersheds); i++)
    {
        double xmin, ymin, xmax, ymax;
        double w, h, s, r, x, y;

        const GEOSGeometry* geom = geomlist_get(&watersheds, i);
        const GEOSPreparedGeometry* prepgeom = GEOSPrepare(geom);

        getGeometryBounds(geom, &xmin, &ymin, &xmax, &ymax);
        // GEOSGeom_getXMin(geom, &xmin);
        // GEOSGeom_getYMin(geom, &ymin);
        // GEOSGeom_getXMax(geom, &xmax);
        // GEOSGeom_getYMax(geom, &ymax);
        w = xmax - xmin;
        h = ymax - ymin;
        s = w < h ? h : w;
        r = s / 25;
        for (x = xmin; x < xmax; x += r)
        {
            for (y = ymin; y < ymax; y += r)
            {
                prep_contains_xy(prepgeom, x, y);
            }
        }
        GEOSPreparedGeom_destroy(prepgeom);
    }
}

/* Clean up any remaining memory */
static void
cleanup(void)
{
    geomlist_free(&watersheds);
}

/*************************************************************************
* CONFIGURATION CALLBACK FUNCTION
*/

gp_test config_point_in_polygon(void)
{
    gp_test test;
    test.name = "Prepared geometry";
    test.description =
        "Load the watersheds, and for each watershed compare"
        "a regular set of points from the envelope to the"
        "watershed using a prepared geometry to find the"
        "inside/outside value.";
    test.func_setup = setup;
    test.func_run = run;
    test.func_cleanup = cleanup;
    test.count = 10;
    return test;
}

