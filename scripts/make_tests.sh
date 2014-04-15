#!/bin/bash

set -ue

if false; then
	CHESS_PROG=hoichess
	CHESS_ARGS=-x
else
	CHESS_PROG=fairymax
	CHESS_ARGS=
fi

rm -f FINISHLOG
n=0
total=${1:-100} # $1 o 100, por defecto

rm -f wpipe bpipe full_log gmon.sum
mkfifo wpipe bpipe

if ! [ -d games ]; then
	mkdir games
fi

while [ $n -lt $total ]; do
	n=$((n+1))

	${CHESS_PROG} ${CHESS_ARGS} <wpipe | tee fairylog >bpipe &
	./chess w 2>&1 >wpipe <bpipe | tee chesslog | grep -E '^RES:' | tee -a FINISHLOG

	wait # wait for opponent

	if [ $n -eq 1 ]; then
		mv gmon.out gmon.sum
	else
		gprof -s chess gmon.out gmon.sum
	fi

	cp gamelog games/gamelog_$n
	cp fairylog  games/fairylog_$n
	cp chesslog  games/chesslog_$n

	lose=$(grep Lose FINISHLOG | wc -l)
	draw=$(grep Draw FINISHLOG | wc -l)
	win=$(grep Win FINISHLOG | wc -l)
	score=$(( (2*win + draw)/ (2*n) ))
	
	echo "$n/$total games (results: $lose/$draw/$win. Score=$score)"

	if [ $((lose + draw + win)) -ne $n ]; then
		echo 'wat!'
		break;
	fi
done

gprof chess gmon.sum > tests_profile
