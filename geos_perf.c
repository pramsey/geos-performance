#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "geos_perf.h"
#include <dlfcn.h>

void* geos_lib_handle;

/************************************************************************
* REGISTER NEW TESTS HERE
*
* To register a new test module, add the config function
* signature here.
*/
gp_test config_buffer_watersheds(void);
gp_test config_buffer_australia(void);
gp_test config_intersection(void);
gp_test config_point_in_polygon(void);
gp_test config_tree_points(void);
gp_test config_tree_points_nn(void);
gp_test config_union_watersheds(void);
gp_test config_isvalid_australia(void);
gp_test config_valid_watersheds(void);
gp_test config_isvalid_landcover(void);

/*
* And then add the function name here
*/
static gp_config_func gp_config_funcs[] =
{
    config_buffer_watersheds,
    config_buffer_australia,
    config_intersection,
    config_union_watersheds,
    config_tree_points,
    config_tree_points_nn,
    config_point_in_polygon,
    config_isvalid_australia,
    config_valid_watersheds,
    config_isvalid_landcover,
    NULL
};

/************************************************************************
* Utilities for a geometry list.
*/

void
geomlist_init(GEOSGeometryList* gl)
{
    static size_t capacity = 16;
    gl->capacity = capacity;
    gl->geoms = malloc(sizeof(GEOSGeometry*) * gl->capacity);
    gl->ngeoms = 0;
    return;
}

void
geomlist_free(GEOSGeometryList* gl)
{
    size_t i;
    for (i = 0; i < gl->ngeoms; i++)
    {
        if (gl->geoms[i])
            GEOSGeom_destroy(gl->geoms[i]);
    }
    free(gl->geoms);
    gl->geoms = NULL;
    return;
}

void
geomlist_release(GEOSGeometryList* gl)
{
    free(gl->geoms);
    gl->geoms = NULL;
    return;
}

size_t
geomlist_push(GEOSGeometryList* gl, GEOSGeometry *g)
{
    if (gl->ngeoms >= gl->capacity)
    {
        gl->capacity *= 2;
        gl->geoms = realloc(gl->geoms, sizeof(GEOSGeometry*) * gl->capacity);
    }
    gl->geoms[gl->ngeoms++] = g;
    return gl->ngeoms;
}

GEOSGeometry*
geomlist_pop(GEOSGeometryList* gl)
{
    if (gl->ngeoms == 0)
        return NULL;
    GEOSGeometry* g = gl->geoms[gl->ngeoms-1];
    gl->geoms[gl->ngeoms-1] = NULL;
    gl->ngeoms--;
    return g;
}

size_t
geomlist_size(GEOSGeometryList* gl)
{
    return gl->ngeoms;
}

const GEOSGeometry*
geomlist_get(GEOSGeometryList* gl, size_t i)
{
    if (i > gl->ngeoms-1)
        return NULL;
    return gl->geoms[i];
}

void
geomlist_print(GEOSGeometryList* gl)
{
    GEOSWKTWriter *writer = GEOSWKTWriter_create();
    size_t size = geomlist_size(gl);
    size_t i;
    for (i = 0; i < size; i++)
    {
        const GEOSGeometry *g = geomlist_get(gl, i);
        if (g)
        {
            char *wkt = GEOSWKTWriter_write(writer, g);
            printf("%s\n", wkt);
            GEOSFree(wkt);
        }
    }
    GEOSWKTWriter_destroy(writer);
}

/************************************************************************
* Utility functions to probe GEOS capabilities
*/

int geos_version_major() {
    const char* geos_ver = GEOSversion();

    return atoi(geos_ver);
}

int geos_version_minor() {
    const char* geos_ver = GEOSversion();
    size_t nchar = strlen(geos_ver);

    size_t dot1 = 0;

    for (size_t i = 0; i < nchar; i++) {
        if (geos_ver[i] == '.') {
            dot1 = i;
            break;
        }
    }

    return atoi(geos_ver + dot1 + 1);
}

/************************************************************************
* Utility functions to polyfill old GEOS versions
*/

/* GEOS < 3.7 does not have GEOSGeom_createPointFromXY */
GEOSGeometry *
createPointFromXY(double x, double y)
{
    GEOSCoordSequence* cs = GEOSCoordSeq_create(
        1,  /* size */
        2); /* dims */

    GEOSCoordSeq_setX(cs, 0, x);
    GEOSCoordSeq_setY(cs, 0, y);

    return GEOSGeom_createPoint(cs);
}


/************************************************************************
* Utility functions for the testing process.
*/

static struct timeval
time_now()
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t;
}

static double
time_difference(struct timeval start, struct timeval end)
{
    int64_t diff_us = (int64_t)(end.tv_usec - start.tv_usec);
    if (start.tv_sec == end.tv_sec)
    {
        return (double)diff_us / 1000000.0;
    }
    else
    {
        int64_t diff_s = (int64_t)(end.tv_sec - start.tv_sec);
        return (double)(diff_s) + (double)(diff_us) / 1000000.0;;
    }
}

static void
geos_log_stderr(const char* fmt, ...)
{
    va_list ap;
    if (debug_level < 1)
        return;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

static void
log_stderr(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

void
debug_stderr(uint32_t level, const char* fmt, ...)
{
    va_list ap;
    if (debug_level < level)
        return;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

static void
result_to_csv(const gp_result* result)
{
    fprintf(stdout, "%s,%s,%u,%0.5g,%0.5g,%0.5g\n",
        result->version,
        result->name,
        result->count,
        result->setup_time,
        result->run_time,
        result->cleanup_time);
}

static gp_result
run_test(const gp_test* test)
{
    size_t i;
    gp_result result;
    double run_time = 0.0,
           setup_time = 0.0,
           cleanup_time = 0.0;
    struct timeval start, end;

    /* Prepare to run tests */
    log_stderr("SETUP [%s] ...", test->name);
    start = time_now();
    if (test->func_setup)
        test->func_setup();
    end = time_now();
    setup_time = time_difference(start, end);
    log_stderr(" %0.3gs\n", setup_time);

    /* Run the tests and time them */
    log_stderr("  RUN [%s] ...", test->name);
    for (i = 0; i < test->count; i++)
    {
        start = time_now();
        if (test->func_run)
            test->func_run();
        end = time_now();
        run_time += time_difference(start, end);
    }
    log_stderr(" %0.3gs\n", run_time);

    /* Clean up after the tests */
    log_stderr("CLEAN [%s] ...", test->name);
    start = time_now();
    if (test->func_cleanup)
        test->func_cleanup();
    end = time_now();
    cleanup_time = time_difference(start, end);
    log_stderr(" %0.3gs\n", cleanup_time);

    /* Sumarize the results */
    result.version = GEOSversion();
    result.count = test->count;
    result.setup_time = setup_time;
    result.run_time = run_time;
    result.cleanup_time = cleanup_time;
    result.name = test->name;
    return result;
}



/************************************************************************
* Main testing loop
*/

uint32_t debug_level = 0;

int
main(int argc, char *argv[])
{
    initGEOS(geos_log_stderr, geos_log_stderr);
    log_stderr("VERSION [GEOS %s]\n", GEOSversion());
#ifdef APPLE
    const char *libname = "libgeos_c.dylib";
#else
    const char *libname = "libgeos_c.so";
#endif

    geos_lib_handle = dlopen(libname, RTLD_LAZY);
    if (!geos_lib_handle) {
        log_stderr("SETUP [Global] Failed to dynamically load %s\n",libname);
    }
    else {
        log_stderr("SETUP [Global] Loaded %s\n",libname);
    }


    gp_config_func* config_func;
    for (config_func = gp_config_funcs; *config_func != NULL; config_func++)
    {
        gp_test test = (*config_func)();

        // If command-line arguments are provided, interpret them to be the
        // set of tests we should run and skip tests not included in the list.
        if (argc > 1) {
            int found = 0;
            for (int i = 1; i < argc; i++) {
                if (strcmp(test.name, argv[i]) == 0) {
                    found = 1;
                }
            }
            if (!found) {
                continue;
            }
        }

        if (test.count < 1)
            continue;

        gp_result result = run_test(&test);
        result_to_csv(&result);
    }

    finishGEOS();
    dlclose(geos_lib_handle);

    return 0;
}

