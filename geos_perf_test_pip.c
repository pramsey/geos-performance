#include <stdio.h>
#include <float.h>

#include "geos_perf.h"

/*************************************************************************
* PERFORMANCE TEST
*/

/* Variables where data lives between the setup/run/cleanup stages */
GEOSGeometryList watersheds;
GEOSGeometryList prepared_watersheds;
GEOSSTRtree* tree;


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

/* For each run, prepare a geometry, then hit it with
   a bunch of points */
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
                GEOSGeometry* pt = createPointFromXY(x, y);
                char in = GEOSPreparedContains(prepgeom, pt);
                GEOSGeom_destroy(pt);
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

