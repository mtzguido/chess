#!/bin/bash

set -ue

secs_to_time () {
	mins=$(($1 / 60))
	secs=$(($1 % 60))

	echo "${mins}m${secs}s"
}

percent () {
	if [ "$2" -eq "0" ]; then
		echo "inf"
	else
		bc -l <<< "scale=2; (100 * $1 / $2)"
	fi
}

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

if ! [ -x ./chess ]; then
	echo "Can't find chess program. Run 'make' first" >&2
	exit 1
fi

if ! [ -d games ]; then
	mkdir games
	echo 0 > games/.seq
	seq=0000
else
	seq=$(($(cat games/.seq)+1))
	echo $seq > games/.seq
	seq=$(printf "%04d" $seq)
fi

DIR=games/$seq
mkdir $DIR

echo "PROG=$CHESS_PROG"		>> $DIR/log
echo "ARGS=$CHESS_ARGS"		>> $DIR/log
echo "OPTS=$@"			>> $DIR/log
echo "Running $total games"	>> $DIR/log

tottime=0
totowntime=0

echo "		b/d/w		score (min - max)"
(while [ $n -lt $total ]; do
	n=$((n+1))

	clock_1=$(date +%s)
	${CHESS_PROG} ${CHESS_ARGS} <wpipe | tee fairylog >bpipe &
	./chess $@ 2>&1 >wpipe <bpipe | tee chesslog | grep -E '^RES:' >> FINISHLOG
	clock_2=$(date +%s)
	owntime=$(($(grep '>> Total time:' chesslog | grep -Eo '[0-9]*') / 1000))
	gametime=$((clock_2 - clock_1))
	tottime=$((tottime+gametime))
	totowntime=$((totowntime+owntime))

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

	echo "$(secs_to_time gametime)	$n/$total	$black/$draw/$white		$score ($min_score - $max_score) (owntime: $(secs_to_time owntime))"

	if [ $((black + draw + white)) -ne $n ]; then
		echo 'wat!'
		break;
	fi
done; echo ; echo "Time: $(secs_to_time $totowntime)/$(secs_to_time $tottime) ($(percent $totowntime $tottime)%)") | tee -a $DIR/log

[ -f gmon.sum ] && gprof chess gmon.sum > $DIR/tests_profile

rm -f FINISHLOG

exit 0
