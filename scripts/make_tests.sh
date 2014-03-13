rm -f FINISHLOG
n=0
total=${1:-100} # $1 o 100, por defecto

rm -f wpipe bpipe full_log
mkfifo wpipe bpipe

while [ $n -lt $total ]; do
	n=$((n+1))
	( fairymax <wpipe >bpipe ) & disown
	pid=$!

	./chess w 2>&1 >wpipe <bpipe | tee -a full_log | grep -E '^RES:' | tee -a FINISHLOG

	kill -9 $pid

	cp gamelog_w gamelog_$n
	echo "$n/$total games"
done

