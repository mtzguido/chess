#!/bin/bash

set -uex

old=$1
new=$2
N=$3

shift 3

mkdir ice-$old ice-$new
git clone . ice-$old && git -C ice-$old checkout $old
git clone . ice-$new && git -C ice-$new checkout $new

make -C ice-$old && cp ice-$old/ice ice-old
make -C ice-$new && cp ice-$new/ice ice-new
rm -rf ./ice-$old ./ice-$new

./scripts/selftest.sh $N ./ice-old ./ice-new "$@"
./scripts/selftest.sh $N ./ice-new fairymax "$@"
./scripts/selftest.sh $N ./ice-old fairymax "$@"
