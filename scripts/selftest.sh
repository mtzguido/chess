#!/bin/bash

N=$1
ARG1=$2
ARG2=$3
shift 3

if ! [ -d logs ]; then
	mkdir logs
	echo 0 > logs/.seq
	seq=0000
else
	seq=$(($(cat logs/.seq) + 1))
	echo $seq > logs/.seq
	seq=$(printf "%04d" $seq)
fi

FILE=logs/LOG_$seq

i=0
th=0
maxth=$(cat /proc/cpuinfo | grep 'cpu cores' | head -n 1 | grep -Eo '[0-9]*')
while [ $i -lt $N ]; do
	if [ $th -ge $maxth ]; then
		wait -n
		th=$((th-1))
	fi

	th=$((th+1))
	i=$((i+1))
	if [ "$((i%2))" == 0 ]; then
		PLAYER1="$ARG1"
		PLAYER2="$ARG2"
	else
		PLAYER1="$ARG2"
		PLAYER2="$ARG1"
	fi

	xvfb-run -a xboard -noGUI -xexit -mg 1 -tc 4 -mps 40 \
		-fcp "$PLAYER1" -scp "$PLAYER2" -sgf "$FILE" \
		"$@" &
done
wait
