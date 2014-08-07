#!/bin/bash

set -ue

CHESS_PROG=./chess
OPPONENT_PROG=fairymax
OPPONENT_ARGS=

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

msg () {
	echo "$@" | tee -a $DIR/log
}

if ! [ -x $CHESS_PROG ]; then
	echo "Can't find chess program. Maybe you should run 'make' first?" >&2
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

tottime=0
totowntime=0

# Make pipes
WPIPE=$(mktemp -u --suffix=.gchess)
BPIPE=$(mktemp -u --suffix=.gchess)

cleanup () {
	rm -f $WPIPE $BPIPE
}

trap cleanup EXIT

mkfifo $WPIPE
mkfifo $BPIPE

# Reset profiling data
rm -f gmon.sum

# Get number of tests and options
total=$1
shift
CHESS_ARGS=$@

msg "OPPONENT_PROG=$OPPONENT_PROG"
msg "OPPONENT_ARGS=$OPPONENT_ARGS"
msg "CHESS_PROG=$CHESS_PROG"
msg "CHESS_ARGS=$CHESS_ARGS"
msg
msg "Running $total games"

msg "	b/d/w		score	time"
ii=0
black=0
draw=0
white=0
while [ $ii -lt $total ]; do
	ii=$((ii+1))

	clock_1=$(date +%s)

	${OPPONENT_PROG} ${OPPONENT_ARGS} <$WPIPE | tee opponent_log >$BPIPE &
	./chess $@ 2>chess_log >$WPIPE <$BPIPE

	clock_2=$(date +%s)
	owntime=$(($(grep '>> Total time:' chess_log | grep -Eo '[0-9]*') / 1000))
	gametime=$((clock_2 - clock_1))
	tottime=$((tottime+gametime))
	totowntime=$((totowntime+owntime))

	wait # wait for opponent

	if [ -f gmon.out ]; then
		gprof chess gmon.out > $DIR/prof_$ii
		if [ $ii -eq 1 ]; then
			mv gmon.out gmon.sum
		else
			gprof -s chess gmon.out gmon.sum
		fi
	fi

	if grep -E '^RES: Lose' chess_log &>/dev/null; then
		black=$((black+1))
	elif grep -E '^RES: Win' chess_log &>/dev/null; then
		white=$((white+1))
	elif grep -E '^RES: Draw' chess_log &>/dev/null; then
		draw=$((draw+1))
	else
		echo '?????' >&2
	fi

	mv gamelog	$DIR/gamelog_$ii
	mv opponent_log	$DIR/opponent_log_$ii
	mv chess_log	$DIR/chess_log_$ii

	score=$(bc -l <<< "scale=2; (2.00*$white + $draw)/ (2.00*$ii)")

	msg "$ii/$total	$black/$draw/$white		$score	$(secs_to_time owntime)/$(secs_to_time gametime)"

done

msg
msg "Time: $(secs_to_time $totowntime)/$(secs_to_time $tottime) ($(percent $totowntime $tottime)%)"

[ -f gmon.sum ] && gprof chess gmon.sum > $DIR/tests_profile

cleanup

exit 0
