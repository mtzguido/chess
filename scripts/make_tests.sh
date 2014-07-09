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
total=$1

shift

echo 'Starting tests. Options = '$@

rm -f wpipe bpipe full_log gmon.sum
mkfifo wpipe bpipe

if ! [ -d games ]; then
	mkdir games
fi

echo "		b/d/w		score (min - max)"
while [ $n -lt $total ]; do
	n=$((n+1))

	clock_1=$(date +%s)
	${CHESS_PROG} ${CHESS_ARGS} <wpipe | tee fairylog >bpipe &
	./chess $@ 2>&1 >wpipe <bpipe | tee chesslog | grep -E '^RES:' >> FINISHLOG
	clock_2=$(date +%s)
	gametime=$((clock_2 - clock_1))
	gamemins=$((gametime / 60))
	gamesecs=$((gametime % 60))

	wait # wait for opponent

	if [ -f gmon.out ]; then
		gprof chess gmon.out > games/prof_$n
		if [ $n -eq 1 ]; then
			mv gmon.out gmon.sum
		else
			gprof -s chess gmon.out gmon.sum
		fi
	fi

	cp gamelog games/gamelog_$n
	cp fairylog  games/fairylog_$n
	cp chesslog  games/chesslog_$n

	black=$(grep Lose FINISHLOG | wc -l)
	draw=$(grep Draw FINISHLOG | wc -l)
	white=$(grep Win FINISHLOG | wc -l)
	score=$(bc -l <<< "scale=2; (2*$white + $draw)/ (2*$n)")
	min_score=$(bc -l <<< "scale=2; (2*$white + $draw)/ (2*$total)")
	max_score=$(bc -l <<< "scale=2; (2*$white + $draw + 2*($total - $n))/ (2*$total)")
	
	echo "${gamemins}m${gamesecs}s	$n/$total	$black/$draw/$white		$score ($min_score - $max_score) "

	if [ $((black + draw + white)) -ne $n ]; then
		echo 'wat!'
		break;
	fi
done

[ -f gmon.out ] && gprof chess gmon.sum > games/tests_profile

exit 0
