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

rm -f wpipe bpipe full_log
mkfifo wpipe bpipe

if ! [ -d games ]; then
	mkdir games
fi

while [ $n -lt $total ]; do
	n=$((n+1))

	${CHESS_PROG} ${CHESS_ARGS} <wpipe | tee fairylog >bpipe &
	./chess w 2>&1 >wpipe <bpipe | tee chesslog | grep -E '^RES:' | tee -a FINISHLOG

	wait # wait for opponent

	cp gamelog_w games/gamelog_$n
	cp fairylog  games/fairylog_$n
	cp chesslog  games/chesslog_$n

	lose=$(grep Lose FINISHLOG | wc -l)
	draw=$(grep Draw FINISHLOG | wc -l)
	win=$(grep Win FINISHLOG | wc -l)
	
	echo "$n/$total games (results: $lose/$draw/$win)"
done

