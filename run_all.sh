#!/bin/bash

cd build
for ver in 3.6 3.7 3.8 3.9 master; do
	export LD_LIBRARY_PATH=/opt/geos/${ver}/lib
	./geos_perf
done
