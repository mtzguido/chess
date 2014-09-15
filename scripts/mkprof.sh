#!/bin/bash

set -ue

./ice --seed 0 --limit 0 --verbose=2 --depth 6 --moves "$@"

gprof ice gmon.out > profile

gprof2dot.py profile > vprof
