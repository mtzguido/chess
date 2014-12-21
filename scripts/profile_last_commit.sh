#!/bin/bash

set -uex

make clean
make
cp ice ice-new
./scripts/mkprof.sh "$@" && mv profile profile.new

git co HEAD^
make clean
make
cp ice ice-old
./scripts/mkprof.sh "$@" && mv profile profile.old

git co master
