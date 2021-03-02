#include <stdint.h>

#include "geos_c.h"
#include "geos_perf_config.h"

#define MAXSTRLEN 1024

/**
* 3.9.2 => 309
*/
#define GEOS_VERSION_CMP ((100*GEOS_VERSION_MAJOR)+GEOS_VERSION_MINOR)

/**
* Generate a "skip" callback function with the appropriate name
*/
#define GEOS_PERF_SKIP(callback_name) \
    gp_test callback_name(void) { \
    gp_test test; \
    test.count = 0; \
    return test; }


/**
* Each setup/run/clean step in a test is
* a void function of this type.
*/
typedef void (*gp_func)(void);

/**
* Each test file has a gp_config_func() that
* returns a test struct, with the name/metadata
* the functions to run, and a count of the number
* of times to run the main test.
*/
typedef struct {
    const char* name;
    const char* description;
    gp_func func_setup;
    gp_func func_run;
    gp_func func_cleanup;
    uint32_t count;
} gp_test;

/**
* The main test runner tracks time for
* each stage and the number of iterations
* and returns summary statistics for
* each test it runs.
*/
typedef struct {
    const char* version;
    const char* name;
    double setup_time;
    double run_time;
    double cleanup_time;
    uint32_t count;
} gp_result;

/**
* Each test must define a config function
* that produces a test structure for the
* runner to execute. The config function is
* referenced at the top of the geos_perf.c
* file.
*/
typedef gp_test (*gp_config_func)(void);

/**
* Geometry list is a simple expandable array
* of GEOSGeometry pointers.
*/
typedef struct {
    GEOSGeometry ** geoms;
    size_t ngeoms;
    size_t capacity;
} GEOSGeometryList;

void geomlist_init(GEOSGeometryList* gl);
void geomlist_free(GEOSGeometryList* gl);
void geomlist_release(GEOSGeometryList* gl);
void geomlist_print(GEOSGeometryList* gl);
size_t geomlist_push(GEOSGeometryList* gl, GEOSGeometry *g);
size_t geomlist_size(GEOSGeometryList* gl);
GEOSGeometry* geomlist_pop(GEOSGeometryList* gl);
const GEOSGeometry* geomlist_get(GEOSGeometryList* gl, size_t i);

/**
* Read a wkt.gz file, with one wkt geometry per line, gzipped.
* File name is relative to the data directory.
* Geometry list must be initialized by caller.
*/
int read_data_file(const char* file_name, GEOSGeometryList* geoms);

/**
* Write a debugging message to stderr if the debug level
* is higher than the threshold.
*/
void debug_stderr(uint32_t level, const char* fmt, ...);

/**
* GEOS < 3.7 does not have GEOSGeom_createPointFromXY
*/
GEOSGeometry* createPointFromXY(double x, double y);

/**
* Global to hold debug level for this run
* currently unused
*/
extern uint32_t debug_level;
