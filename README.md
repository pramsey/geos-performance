# GEOS Performance Tests

This is a very simple framework for a batteries-included set of tests that exercise core features of GEOS, so inter-version performance changes can be evaluated and profileed.

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

The key in building the tests is to **use the oldest version as the build baseline**, so that the performance suite doesn't fail due to missing symbols.

(**Note**: This means that tests that require newer functions cannot be built or used against older GEOS versions. With some compile-time defines, which don't yet exist, it should be possible to support both old and new versions, within reason.)

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

To swap GEOS versions, just change put the desired GEOS version at the front of the linker priority order using `LD_LIBRARY_PATH` (Linux) or `DYLD_LIBRARY_PATH` (MacOS).

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

# Adding Tests

Each test lives in a single file, and defines a 'setup', 'run' and 'cleanup' phase. For simplicity, all the tests are named `geos_perf_test_*.c`.

The [geos_perf_tests_buffer.c](geos_perf_tests_buffer.c) is a good example.

* It has one global variable, [watersheds](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf_test_buffer.c#L10) which is a [GeometryList](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf.h#L53-L61), a utility struct for a variable-length collection of `GEOSGeom*`.
* In [setup()](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf_test_buffer.c#L12-L17) it data from a gzipped WKT file into a global variable, using a the [read_data_file](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf.h#L72-L77) utility function.
* In [run()](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf_test_buffer.c#L19-L33) it loops through each geometry in the list and buffers it.
* In [cleanup()](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf_test_buffer.c#L35-L39) it frees the `GeometryList`.
* The test is exposed to the test runner using a configuration callback, [config_buffer_watersheds](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf_test_buffer.c#L41-L57), that returns a [gp_test](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf.h#L14-L27) struct. The struct includes references to the three key functions, a "count" of how many times to execute the "run" stage, and a name and description field for human-readable summaries of what the test exercises.
* In `geos_perf.c` the test is registered twice (could maybe figure some macro magic to avoid this), once to add the [function signature](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf.c#L17) of the config callback and once to actually [execute the callback](https://github.com/pramsey/geos-performance/blob/fdeba6d471a5ef6f1b45e03956e53f3606ca9368/geos_perf.c#L28).
