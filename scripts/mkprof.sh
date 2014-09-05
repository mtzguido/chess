#!/bin/bash

set -ue

N=${1:-10}

./ice --moves $N --seed 0 --limit 0 --verbose 2 --depth 6

gprof ice gmon.out > profile

gprof2dot.py profile > vprof
