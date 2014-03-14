CHESS_PROG=hoichess
CHESS_ARGS=-x

rm -f FINISHLOG
n=0
total=${1:-100} # $1 o 100, por defecto

rm -f wpipe bpipe full_log
mkfifo wpipe bpipe

while [ $n -lt $total ]; do
	n=$((n+1))

	${CHESS_PROG} ${CHESS_ARGS} <wpipe | tee fairylog >bpipe &
	./chess w 2>&1 >wpipe <bpipe | tee -a full_log | grep -E '^RES:' | tee -a FINISHLOG

	echo -n "waiting.."
	wait
	echo ok

	cp gamelog_w gamelog_$n
	echo "$n/$total games"
done

