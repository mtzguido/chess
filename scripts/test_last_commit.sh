#!/bin/bash

N=$1
shift

make clean
make
mv ice ice-new
git co HEAD^
make clean
make
mv ice ice-old
git co master

./scripts/selftest.sh $N ./ice-old ./ice-new "$@"
