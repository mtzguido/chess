#!/bin/bash

OPEN_S=0
OPEN_Q=0
N=0
SEEDS=0
BOARDS=$(wc -l scripts/bench_heur.boards | cut -d' ' -f1)

if [ "$1" == '--seeds' ]; then
	SEEDS=$2
	shift
	shift
fi

TOT=0
for i in $SEEDS; do
	TOT=$((TOT + BOARDS))
done

while read board; do
	for s in $SEEDS; do
		RES=$(./ice --bench-search --seed $s --init "$board" "$1" | tail -n1)
		S=$(echo "$RES" | cut -d' ' -f1)
		Q=$(echo "$RES" | cut -d' ' -f2)

		OPEN_S=$((OPEN_S + S))
		OPEN_Q=$((OPEN_Q + Q))

		N=$((N+1))
		echo "($N/$TOT)	$S/$Q	$OPEN_S/$OPEN_Q"
	done
done < scripts/bench_heur.boards

echo "Average S/Q:	$((OPEN_S / TOT))/$((OPEN_Q / TOT))"
echo "Average total:	$(((OPEN_S + OPEN_Q) / TOT))"
