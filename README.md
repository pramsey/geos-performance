# GEOS Performance Tests

This is a very simple framework for a batteries-included set of tests that exercise core features of GEOS, so inter-version performance changes can be evaluated and profiled.

# Usage

## Build and Install GEOS Versions

The point of the suite is to compare different versions. For example, to compare two working branches, or two released versions, let's call them "baseline" and "development".

```
# setup build dir for baseline
cd geos-baseline
mkdir _build
cd _build

# build and install baseline
cmake -DCMAKE_INSTALL_PREFIX=/opt/geos/baseline
make && make install

# setup build dir for development
cd geos-development
mkdir _build
cd _build

# build and install development
cmake -DCMAKE_INSTALL_PREFIX=/opt/geos/development
make && make install
```

## Build the Performance Tests

The performance tests include their data and depend on the GEOS C API and zlib (to uncompress the test data).

The key in building the tests is to **use the oldest version to configure and build the `geos-perf` binary**, so that you don't end up with a binary that has symbols newer than some of the libraries you plan to test.

```
# get the source
git clone git@github.com:pramsey/geos-performance.git
cd geos-performance
mkdir _build
cd _build

# compile against the OLDER version by putting its
# copy of geos-config first in the PATH
export PATH=/opt/geos/baseline/bin:$PATH
cmake ..
make
```

At this point you have a `geos-perf` binary ready to run.

## Run the Performance Tests

To swap GEOS versions, just put the desired GEOS version at the front of the linker priority order using the `LD_LIBRARY_PATH` (Linux) or `DYLD_LIBRARY_PATH` (MacOS) environment variables.

The `geos-perf` program writes out status messages on *stderr*, and a csv output for use in analysis programs on *stdout*, so we can capture *stdout* to a file and watch progress on *stderr*.

```
# Get last release results
export LD_LIBRARY_PATH=/opt/geos/3.9/lib
./geos-perf >> results.csv

# Get baseline results
export LD_LIBRARY_PATH=/opt/geos/baseline/lib
./geos-perf >> results.csv

# Get our development results
export LD_LIBRARY_PATH=/opt/geos/development/lib
./geos-perf >> results.csv
```

Or use the shell to run all the results in one grand loop:

```
for ver in 3.6 3.7 3.8 3.9; do
  export LD_LIBRARY_PATH=/opt/geos/$ver/lib
  ./geos-perf >> geos-perf-results.csv
done
```

# Adding Tests

Each test lives in a single file, and defines 'setup', 'run' and 'cleanup' phases. For simplicity, all the tests are named using `geos_perf_test_*.c` as the file name pattern.

The [geos_perf_tests_buffer.c](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf_test_buffer.c) test is a good example.

* It has one global variable, [watersheds](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf_test_buffer.c#L10) which is a [GeometryList](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf.h#L53-L61), a utility struct for a variable-length collection of `GEOSGeom*`.
* In [setup()](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf_test_buffer.c#L12-L17) it reads data from a gzipped WKT file into a global variable, using a the [read_data_file](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf.h#L72-L77) utility function.
* In [run()](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf_test_buffer.c#L19-L33) it loops through each geometry in the list and runs `GEOSBuffer` on it, then it runs `GEOSGeom_destroy()` on the buffered output.
* In [cleanup()](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf_test_buffer.c#L35-L39) it frees the `GeometryList`.
* The test is exposed to the test runner using a configuration callback, [config_buffer_watersheds](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf_test_buffer.c#L41-L57), that returns a [gp_test](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf.h#L14-L27) struct. The struct includes references to the three key functions, a "count" of how many times to execute the "run" stage, and a name and description field for human-readable summaries of what the test exercises.
* In `geos_perf.c` the test is registered twice (could maybe figure some macro magic to avoid this), once to add the [function signature](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf.c#L17) of the config callback and once to actually [execute the callback](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf.c#L28).

**Note**: Much older baseline versions may **completely lack** functions that exist in newer versions and thus the build will have to omit tests that exercise those functions. See [geos_perf_test_tree_nn.c](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf_test_tree_nn.c) for an example that skips a test when built against an older GEOS release version.
