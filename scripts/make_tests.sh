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

rm -f wpipe bpipe full_log gmon.sum
mkfifo wpipe bpipe

if ! [ -d games ]; then
	mkdir games
	echo 0 > games/.seq
	seq=0
else
	seq=$(($(cat games/.seq)+1))
	echo $seq > games/.seq
fi

DIR=games/$seq
mkdir $DIR

echo "PROG=$CHESS_PROG"		>> $DIR/log
echo "ARGS=$CHESS_ARGS"		>> $DIR/log
echo "OPTS=$@"			>> $DIR/log
echo "Running $total games"	>> $DIR/log

echo "		b/d/w		score (min - max)"
(while [ $n -lt $total ]; do
	n=$((n+1))

	clock_1=$(date +%s)
	${CHESS_PROG} ${CHESS_ARGS} <wpipe | tee fairylog >bpipe &
	./chess $@ 2>&1 >wpipe <bpipe | tee chesslog | grep -E '^RES:' >> FINISHLOG
	clock_2=$(date +%s)
	owntime=$(($(grep '>> Total time:' chesslog | grep -Eo '[0-9]*') / 1000))
	ownmins=$((owntime / 60))
	ownsecs=$((owntime % 60))
	gametime=$((clock_2 - clock_1))
	gamemins=$((gametime / 60))
	gamesecs=$((gametime % 60))

	wait # wait for opponent

	if [ -f gmon.out ]; then
		gprof chess gmon.out > $DIR/prof_$n
		if [ $n -eq 1 ]; then
			mv gmon.out gmon.sum
		else
			gprof -s chess gmon.out gmon.sum
		fi
	fi

	mv gamelog	$DIR/gamelog_$n
	mv fairylog	$DIR/fairylog_$n
	mv chesslog	$DIR/chesslog_$n

	black=$(grep Lose FINISHLOG | wc -l)
	draw=$(grep Draw FINISHLOG | wc -l)
	white=$(grep Win FINISHLOG | wc -l)
	score=$(bc -l <<< "scale=2; (2*$white + $draw)/ (2*$n)")
	min_score=$(bc -l <<< "scale=2; (2*$white + $draw)/ (2*$total)")
	max_score=$(bc -l <<< "scale=2; (2*$white + $draw + 2*($total - $n))/ (2*$total)")
	
	echo "${gamemins}m${gamesecs}s	$n/$total	$black/$draw/$white		$score ($min_score - $max_score) (owntime: ${ownmins}m${ownsecs}s)"

	if [ $((black + draw + white)) -ne $n ]; then
		echo 'wat!'
		break;
	fi
done) | tee -a $DIR/log

[ -f gmon.out ] && gprof chess gmon.sum > $DIR/tests_profile

rm -f FINISHLOG

exit 0
